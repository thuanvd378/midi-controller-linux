#!/bin/bash

# Script này gỡ bỏ module custom_midi khỏi hệ thống
# và trả lại quyền điều khiển thiết bị cho driver gốc của Linux (snd-usb-audio)

VENDOR_ID="0499"
CUSTOM_DRIVER="custom_usb_midi"
SND_DRIVER="snd-usb-audio"

echo "Đang dọn dẹp hệ thống và khôi phục cài đặt gốc..."

# 1. Trả lại quyền điều khiển thiết bị nếu đàn đang được cắm
DEVICE_PATHS=$(for d in /sys/bus/usb/devices/*; do
    if [ -f "$d/idVendor" ] && [ "$(cat "$d/idVendor")" = "$VENDOR_ID" ]; then
        echo "$d"
    fi
done)

if [ -n "$DEVICE_PATHS" ]; then
    for dev_path in $DEVICE_PATHS; do
        DEVICE_NAME=$(basename $dev_path)
        
        for interface_path in $dev_path/$DEVICE_NAME:*; do
            if [ -d "$interface_path" ]; then
                IFACE_NAME=$(basename $interface_path)
                
                # Nếu custom_midi đang chiếm giữ
                if [ -L "$interface_path/driver" ]; then
                    CURRENT_DRIVER=$(basename $(readlink $interface_path/driver))
                    if [ "$CURRENT_DRIVER" = "$CUSTOM_DRIVER" ]; then
                        echo "⚠️ Đang ngắt kết nối $IFACE_NAME khỏi $CUSTOM_DRIVER..."
                        echo -n "$IFACE_NAME" | sudo tee /sys/bus/usb/drivers/$CUSTOM_DRIVER/unbind > /dev/null
                        
                        echo "➡️ Trả $IFACE_NAME về cho driver hệ thống ($SND_DRIVER)..."
                        echo -n "$IFACE_NAME" | sudo tee /sys/bus/usb/drivers/$SND_DRIVER/bind > /dev/null 2>&1
                    fi
                fi
            fi
        done
    done
    echo "✅ Đã trả lại thiết bị cho hệ điều hành."
else
    echo "ℹ️ Không tìm thấy đàn nào đang cắm vào máy, bỏ qua bước khôi phục kết nối USB."
fi

# 2. Xoá module ra khỏi kernel
if lsmod | grep -q "^custom_midi "; then
    echo "⚠️ Đang gỡ driver custom_midi khỏi kernel..."
    sudo rmmod custom_midi
    echo "✅ Đã gỡ module thành công."
else
    echo "ℹ️ Module custom_midi hiện chưa được nạp, không cần gỡ."
fi

# 3. Dọn dẹp file biên dịch (tuỳ chọn)
echo "🧹 Đang dọn dẹp các file rác sinh ra trong quá trình build..."
make clean > /dev/null 2>&1

echo ""
echo "🎉 Hoàn tất! Hệ thống đã trở về trạng thái như chưa từng cài đặt driver."
