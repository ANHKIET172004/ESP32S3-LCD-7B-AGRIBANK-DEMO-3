# QMS- CALLING NUMBER KEYPAD

Thiết bị gọi số thứ tự và đánh giá chất lượng dịch vụ,
ứng dụng cho ngân hàng, bệnh viện, cơ quan hành chính, quầy giao dịch.

Hệ thống cho phép:

- Nhân viên gọi số thứ tự khách hàng đến quầy dịch vụ
- Số được gọi sẽ được gửi đến màn hình gọi số để hiển thị và màn hình đánh giá chất lượng để lưu kết quả đánh giá
- Hiển thị thông tin rõ ràng, dễ sử dụng

---

## Features

- Gọi số tiếp theo
- Gọi lại số đang phục vụ
- Gọi số ưu tiên
- Bỏ qua số hiện tại
- Gọi số bị bỏ qua
- Hiển thị số đang phục vụ trên màn hình
- Hiển thị trạng thái kết nối server trên màn hình
- Hiển thị id của thiết bị trên màn hình
- Nhập wifi để kết nối và lưu thông tin wifi vào bộ nhớ NVS khi kết nối thành công
- Tự kết nối đến wifi đã lưu khi khởi động thiết bị
- Tự động kết nối lại wifi khi bị mất kết nối
- Reset số đang lưu và gửi lệnh reset số đến màn hình hiển thị và màn hình đánh giá chất lượng
- Gửi lệnh đến màn hình hiển thị đóng quầy

---

## System Overview

| Thiết bị | Mô tả                   | Chức năng                                                                                                 |
| -------- | ----------------------- | --------------------------------------------------------------------------------------------------------- |
| MCU      | ESP32                   | xử lý trung tâm, gửi và nhận dữ liệu đến server, màn hình hiển thị số và màn hình đánh giá thông qua MQTT |
| Keypad   | Bàn phím ma trận số 4x4 | Nhập số, ký tự và tùy chỉnh các chức năng của thiết bị                                                    |
| Display  | LCD 16x2                | Hiển thị thông tin như số đang gọi, dữ liệu nhập từ bàn phím, các chức năng của thiết bị, ...             |

---

## Pin Configuration

- Keypad Rows / Columns → GPIO
- LCD16x2 Display → I2C (PCF8574)

| PIN CỦA THIẾT BỊ NGOẠI VI | PIN ESP32   |
| ------------------------- | ----------- |
| COL1                      | GPIO_NUM_13 |
| COL2                      | GPIO_NUM_12 |
| COL3                      | GPIO_NUM_14 |
| COL4                      | GPIO_NUM_27 |
| ROW1                      | GPIO_NUM_32 |
| ROW2                      | GPIO_NUM_33 |
| ROW3                      | GPIO_NUM_25 |
| ROW4                      | GPIO_NUM_26 |
| SDA                       | GPIO_NUM_21 |
| SCL                       | GPIO_NUM_22 |

- Ma trận phím 4x4:
  | | C1 | C2 | C3 | C4 |
  |-----|----|----|----|----|
  | R1 | 1 | 2 | 3 | A |
  | R2 | 4 | 5 | 6 | B |
  | R3 | 7 | 8 | 9 | C |
  | R4 | ^ | 0 | DEL | D |

## Project Structure

```

project/
├── main/
│ ├── esp_mqtt_client/
│ ├── keypad/
│ ├── lcd_i2c/
│ ├── led/
│ ├── mac_utils/
│ ├── mutex/
| ├── nvs_utils/
│ ├── state_machine/
│ └── wifi/
└── README.md
```

## WORKFLOW (State Machine)

Hệ thống hoạt động theo mô hình **State Machine**, mỗi trạng thái tương ứng với
một màn hình hiển thị và tập chức năng cụ thể.

---

### STATE_INIT – Khởi động hệ thống

- Thiết bị khởi động
- Khởi tạo các module:
  - LCD
  - Keypad
  - WiFi
  - MQTT
  - NVS
- Đọc thông tin WiFi đã lưu trong NVS
- Chuyển sang `STATE_WIFI_CONNECT`

---

### STATE_WIFI_CONNECT – Kết nối WiFi

- Nếu **có WiFi đã lưu trong NVS**:
  - Thử kết nối WiFi đã lưu
  - Hiển thị trạng thái kết nối trên LCD
  - Retry tối đa **5 lần**
- Nếu **kết nối thành công**:
  - `STATUS = OK`
  - Chuyển sang `STATE_MAIN`
- Nếu **kết nối thất bại sau 5 lần**:
  - `STATUS = NO`
  - Chuyển sang `STATE_MAIN`

---

### STATE_MAIN – Màn hình chính / Gọi số

- Hiển thị:
  - Số đang gọi
  - ID thiết bị
  - Trạng thái kết nối (`OK / NO`)
- Xử lý phím:
  - **B** → Gọi số tiếp theo
    - Nếu thất bại hiển thị `"GOI THAT BAI"`
  - **C** → Bỏ qua số hiện tại
  - **Nhập số (4 chữ số và là số được lấy từ kiosk) + D** → Gọi ưu tiên số đã nhập
  - **D (không nhập số)** → Gọi lại số hiện tại
  - **A** → Chuyển sang `STATE_MENU`

---

### STATE_MENU – Menu chức năng

- Hiển thị danh sách chức năng
- Điều khiển:
  - **1** → Chuyển sang chức năng tiếp theo
  - **2** → Quay lại chức năng trước
  - **D** → Chọn chức năng
  - **A** → Thoát menu → `STATE_MAIN`

---

### STATE_WIFI_CONFIG – Cấu hình WiFi

#### ▪ STATE_WIFI_SSID_INPUT – Nhập tên WiFi

- Nhập tên WiFi từ bàn phím
- **^** → Chuyển chữ hoa / chữ thường
- **D** → Chuyển sang nhập mật khẩu
- **DEL** → Xóa ký tự vừa nhập

#### STATE_WIFI_PASS_INPUT – Nhập mật khẩu

- Nhập mật khẩu WiFi
- Mặc định hiển thị `*`
- **C** → Ẩn / hiện mật khẩu
- **^** → Chuyển chữ hoa / chữ thường
- **DEL** → Xóa ký tự vừa nhập
- **D** → Bắt đầu kết nối WiFi → `STATE_WIFI_CONNECT`

---

### STATE_CALL_SKIPPED – Gọi số đã bỏ qua

- Gửi yêu cầu đến server
- Server trả về số đã bị bỏ qua trước đó
- Hiển thị số nhận được
- Chuyển về `STATE_MAIN`

---

### 🔹 STATE_TRANSFER_COUNTER – Chuyển quầy

- Hiển thị danh sách ID các thiết bị trong hệ thống
- Điều khiển:
  - **1** → Chuyển tới thiết bị tiếp theo
  - **2** → Quay lại thiết bị trước
  - **D** → Chọn quầy
- Sau khi chọn:
  - Chuyển số đang xử lý sang quầy được chọn
  - Gửi lệnh xóa dữ liệu đến màn hình đánh giá:
    - Nếu tồn tại `next_number` → xóa `next_number`
    - Nếu không → xóa `current_number`
- Chuyển về `STATE_MAIN`

---

### STATE_TRANSFER_SERVICE – Chuyển dịch vụ

- Hiển thị danh sách dịch vụ
- Điều khiển:
  - **1 / 2** → Duyệt danh sách dịch vụ
  - **D** → Chọn dịch vụ
- Sau khi chọn dịch vụ:
  - Chọn vị trí chuyển số:
    - Đầu dịch vụ
    - Cuối dịch vụ
- Hoàn tất → `STATE_MAIN`

---

### STATE_REGISTER_DEVICE – Đăng ký thiết bị

- Gửi MAC address của thiết bị lên server
- Chờ cấu hình từ hệ thống web (tên thiết bị, dịch vụ)
- Hoàn tất → `STATE_MAIN`

---

### STATE_RESET – Reset thiết bị

- Xóa số đang xử lý hiện tại
- Gửi lệnh reset đến:
  - Màn hình đánh giá
  - Màn hình hiển thị số
- Chuyển về `STATE_MAIN`

---

### STATE_CLOSED – Đóng quầy

- Gửi lệnh hiển thị:
  - `"QUẦY TẠM THỜI ĐÓNG"`
- Khóa chức năng gọi số
- Chuyển sang `STATE_LOCKED`

---

### STATE_LOCKED – Quầy bị khóa

- Vô hiệu hóa các chức năng gọi số
- **A** → Thoát chế độ khóa → `STATE_MAIN`

## ⚠ Error Handling

- WiFi retry: 5 lần
- MQTT reconnect: tự động
- Mất kết nối đến server hoặc mất kết nối wifi: hiển thị "STATUS:NO" trên màn hình LCD

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# Hello World Example

Starts a FreeRTOS task to print "Hello World".

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## How to use example

Follow detailed instructions provided specifically for this example.

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)


## Example folder contents

The project **hello_world** contains one source file in C language [hello_world_main.c](main/hello_world_main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_hello_world.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.
