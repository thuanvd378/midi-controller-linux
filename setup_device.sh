#!/bin/bash

# Thư mục device trên sysfs (tuỳ thuộc vào cổng cắm USB, ở đây là 1-4)
# Script này tìm tự động các thiết bị Yamaha và gán vào custom_usb_midi

VENDOR_ID="0499"
CUSTOM_DRIVER="custom_usb_midi"
SND_DRIVER="snd-usb-audio"

echo "Đang tìm kiếm thiết bị đàn Yamaha (Vendor ID: $VENDOR_ID)..."

DEVICE_PATHS=$(for d in /sys/bus/usb/devices/*; do
    if [ -f "$d/idVendor" ] && [ "$(cat "$d/idVendor")" = "$VENDOR_ID" ]; then
        echo "$d"
    fi
done)

if [ -z "$DEVICE_PATHS" ]; then
    echo "❌ Không tìm thấy đàn Yamaha nào được cắm vào. Hãy kiểm tra lại cáp kết nối."
    exit 1
fi

for dev_path in $DEVICE_PATHS; do
    DEVICE_NAME=$(basename $dev_path)
    echo "✅ Đã tìm thấy thiết bị ở cổng: $DEVICE_NAME"
    
    # Duyệt qua các interface của thiết bị
    for interface_path in $dev_path/$DEVICE_NAME:*; do
        if [ -d "$interface_path" ]; then
            IFACE_NAME=$(basename $interface_path)
            
            # Kiểm tra xem snd-usb-audio có đang chiếm giữ không
            if [ -L "$interface_path/driver" ]; then
                CURRENT_DRIVER=$(basename $(readlink $interface_path/driver))
                if [ "$CURRENT_DRIVER" = "$SND_DRIVER" ]; then
                    echo "⚠️ Phát hiện $SND_DRIVER đang chiếm giữ $IFACE_NAME, tiến hành gỡ bỏ..."
                    echo -n "$IFACE_NAME" | sudo tee /sys/bus/usb/drivers/$SND_DRIVER/unbind > /dev/null
                fi
            fi
            
            # Thử gán cho custom_usb_midi
            echo "➡️ Đang kết nối $IFACE_NAME với $CUSTOM_DRIVER..."
            echo -n "$IFACE_NAME" | sudo tee /sys/bus/usb/drivers/$CUSTOM_DRIVER/bind > /dev/null 2>&1
            
            # Kiểm tra xem gán thành công không
            if [ -L "$interface_path/driver" ]; then
                NEW_DRIVER=$(basename $(readlink $interface_path/driver))
                if [ "$NEW_DRIVER" = "$CUSTOM_DRIVER" ]; then
                    echo "✅ Kết nối thành công luồng dữ liệu ($IFACE_NAME)."
                else
                    echo "ℹ️ Bỏ qua luồng điều khiển chung ($IFACE_NAME)."
                fi
            else
                echo "ℹ️ Bỏ qua luồng điều khiển chung ($IFACE_NAME)."
            fi
        fi
    done
done

echo ""
echo "🎉 Hoàn tất! Hãy mở ứng dụng đàn ảo để trải nghiệm."
