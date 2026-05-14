# Custom MIDI Controller — Linux Character Device Driver (LKM)

Dự án này được chia thành nhiều module nhỏ lẻ và cấp thư mục khác nhau để dễ dàng phân công công việc cho một nhóm 5 người.

## Phân công nhóm (5 Người)

Dự án được cấu trúc theo thư mục:

1. **Thành viên 1 - Khởi tạo & Cấu trúc (Core/Init)**
   - **Nhiệm vụ**: Khởi tạo Module, cấp phát Major/Minor, khởi tạo Character Device, xử lý quá trình nạp/gỡ module an toàn.
   - **File phụ trách**: `driver/main.c`

2. **Thành viên 2 - File Operations (Giao tiếp Kernel - User)**
   - **Nhiệm vụ**: Triển khai các hàm thao tác file `open`, `read`, `write`, `release`. Đảm bảo copy dữ liệu an toàn giữa Kernel Space và User Space (tránh lỗi Stack/Memory).
   - **File phụ trách**: `driver/fops.c`, `driver/midi_core.h`

3. **Thành viên 3 - Mô phỏng phần cứng & Đồng bộ (HW/Concurrency)**
   - **Nhiệm vụ**: Quản lý Kernel FIFO (bộ đệm vòng), Wait Queues, Mutex, Spinlock và Timer (Mô phỏng ngắt ISR truyền dữ liệu Yamaha MIDI).
   - **File phụ trách**: `driver/hw_sim.c`

4. **Thành viên 4 - Ứng dụng User Space (Test App)**
   - **Nhiệm vụ**: Viết chương trình C phía người dùng để tương tác qua `/dev/custom_midi`. Đọc ghi luồng byte, in ra dạng Hex Dump và dịch mã Status của MIDI.
   - **File phụ trách**: `tests/test_app.c`

5. **Thành viên 5 - Build System & Integration**
   - **Nhiệm vụ**: Viết Kbuild Makefiles cho thư mục `driver`, Makefile cho `tests`, file Root `Makefile` để build cả 2 cùng lúc và viết tài liệu `README.md`.
   - **File phụ trách**: `Makefile`, `driver/Makefile`, `tests/Makefile`, `docs/README.md` (hoặc root `README.md`).

---

## Hướng dẫn chạy nhanh

### 1. Build Toàn bộ dự án
```bash
make all
```
*Lệnh này sẽ tự động gọi Makefile bên trong `driver/` và `tests/`.*

### 2. Nạp (Load) Driver vào Kernel
```bash
make load
```
*(Yêu cầu quyền sudo - sẽ tự động chạy `sudo insmod driver/custom_midi.ko`)*

### 3. Kiểm tra cài đặt
```bash
dmesg | tail -n 5
ls -l /dev/custom_midi
```

### 4. Chạy Test App
```bash
cd tests
sudo ./test_app
```
*(Nhấn Ctrl-C để thoát)*

### 5. Gỡ (Unload) Driver
```bash
# Trở về thư mục gốc và chạy
make unload
```

### 6. Dọn dẹp mã nguồn
```bash
make clean
```
