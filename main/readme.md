# QMS- CALLING NUMBER KEYPAD

Thiết bị gọi số thứ tự và đánh giá chất lượng dịch vụ,
ứng dụng cho ngân hàng, bệnh viện, cơ quan hành chính, quầy giao dịch.

Hệ thống cho phép:

- Nhân viên gọi số thứ tự khách hàng đến quầy dịch vụ
- Số được gọi sẽ được gửi đến màn hình gọi số để hiển thị và màn hình đánh giá chất lượng để lưu kết quả đánh giá
- Hiển thị thông tin rõ ràng, dễ sử dụng

---

## ✨ Features

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

## 🧩 System Overview

| Thiết bị | Mô tả                   | Chức năng                                                                                                 |
| -------- | ----------------------- | --------------------------------------------------------------------------------------------------------- |
| MCU      | ESP32                   | xử lý trung tâm, gửi và nhận dữ liệu đến server, màn hình hiển thị số và màn hình đánh giá thông qua MQTT |
| Keypad   | Bàn phím ma trận số 4x4 | Nhập số, ký tự và tùy chỉnh các chức năng của thiết bị                                                    |
| Display  | LCD 16x2                | Hiển thị thông tin như số đang gọi, dữ liệu nhập từ bàn phím, các chức năng của thiết bị, ...             |

---

## 📍 Pin Configuration

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

## 📁 Project Structure

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

## 🔄 WORKFLOW (State Machine)

Hệ thống hoạt động theo mô hình **State Machine**, mỗi trạng thái tương ứng với
một màn hình hiển thị và tập chức năng cụ thể.

---

### 🔹 STATE_INIT – Khởi động hệ thống

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

### 🔹 STATE_WIFI_CONNECT – Kết nối WiFi

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

### 🔹 STATE_MAIN – Màn hình chính / Gọi số

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

### 🔹 STATE_MENU – Menu chức năng

- Hiển thị danh sách chức năng
- Điều khiển:
  - **1** → Chuyển sang chức năng tiếp theo
  - **2** → Quay lại chức năng trước
  - **D** → Chọn chức năng
  - **A** → Thoát menu → `STATE_MAIN`

---

### 🔹 STATE_WIFI_CONFIG – Cấu hình WiFi

#### ▪ STATE_WIFI_SSID_INPUT – Nhập tên WiFi

- Nhập tên WiFi từ bàn phím
- **^** → Chuyển chữ hoa / chữ thường
- **D** → Chuyển sang nhập mật khẩu
- **DEL** → Xóa ký tự vừa nhập

#### ▪ STATE_WIFI_PASS_INPUT – Nhập mật khẩu

- Nhập mật khẩu WiFi
- Mặc định hiển thị `*`
- **C** → Ẩn / hiện mật khẩu
- **^** → Chuyển chữ hoa / chữ thường
- **DEL** → Xóa ký tự vừa nhập
- **D** → Bắt đầu kết nối WiFi → `STATE_WIFI_CONNECT`

---

### 🔹 STATE_CALL_SKIPPED – Gọi số đã bỏ qua

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

### 🔹 STATE_TRANSFER_SERVICE – Chuyển dịch vụ

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

### 🔹 STATE_REGISTER_DEVICE – Đăng ký thiết bị

- Gửi MAC address của thiết bị lên server
- Chờ cấu hình từ hệ thống web (tên thiết bị, dịch vụ)
- Hoàn tất → `STATE_MAIN`

---

### 🔹 STATE_RESET – Reset thiết bị

- Xóa số đang xử lý hiện tại
- Gửi lệnh reset đến:
  - Màn hình đánh giá
  - Màn hình hiển thị số
- Chuyển về `STATE_MAIN`

---

### 🔹 STATE_CLOSE_COUNTER – Đóng quầy

- Gửi lệnh hiển thị:
  - `"QUẦY TẠM THỜI ĐÓNG"`
- Khóa chức năng gọi số
- Chuyển sang `STATE_LOCKED`

---

### 🔹 STATE_LOCKED – Quầy bị khóa

- Vô hiệu hóa các chức năng gọi số
- **A** → Thoát chế độ khóa → `STATE_MAIN`

## ⚠ Error Handling

- WiFi retry: 5 lần
- MQTT reconnect: tự động
- Mất kết nối đến server hoặc mất kết nối wifi: hiển thị "STATUS:NO" trên màn hình LCD

## 🚀 Build & Flash

### Requirements

- ESP-IDF v5.x
- Python 3.10+

### Build

```bash
idf.py build
```

### Build

```bash
idf.py flash monitor
```
