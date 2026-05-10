# IoT Gateway ESP32: MQTT và TinyML phát hiện bất thường môi trường

> BÀI TẬP LỚN MÔN PHÁT TRIỂN ỨNG DỤNG IOT

## Đề tài
**IoT Gateway ESP32: MQTT và TinyML phát hiện bất thường môi trường**

## Thông tin nhóm
- Giảng viên hướng dẫn: Lê Trọng Nhân
- Sinh viên thực hiện:
  - Lê Công Tú – 2570367
  - Cao Vĩnh Phát – 2212497
- Github repository: https://github.com/phat-tech123/IoT_gateway
- Youtube demo: https://youtu.be/bGHuYFx_vc0

> Thành phố Hồ Chí Minh, Tháng 5 Năm 2026

---

## Tổng quan dự án
Dự án xây dựng một hệ thống IoT Gateway sử dụng ESP32, MQTT và TinyML để thu thập dữ liệu môi trường, phát hiện bất thường và chuyển tiếp dữ liệu tới nền tảng CoreIoT. Hệ thống gồm:

- `ESP32_MQTT_Test`: mã nguồn cho hai ESP32 (ESP1 liên tục đo nhiệt độ/độ ẩm, ESP2 đánh giá anomaly bằng mô hình TinyML)
- `gateway/mqtt_gateway.py`: Python gateway chuyển tiếp dữ liệu giữa MQTT broker cục bộ và CoreIoT
- `description_system.txt`: ghi chú yêu cầu và topic MQTT

## Kiến trúc hệ thống

### Local MQTT Broker
- Broker local chạy trên `192.168.137.1:1883`
- ESP1, ESP2 và gateway đều kết nối tới broker này

### CoreIoT
- Host: `app.coreiot.io`
- Port: `1883`
- Access token hiện tại (gateway): `g5fqNKTTwyhgEkO9NVKC`

### Luồng dữ liệu
- ESP1 gửi telemetry môi trường lên broker cục bộ
- ESP2 nhận telemetry ESP1, chạy TinyML phát hiện anomaly và publish kết quả
- Gateway lắng nghe telemetry/status/anomaly cục bộ và gửi dữ liệu này lên CoreIoT
- Gateway nhận lệnh điều khiển LED từ CoreIoT và chuyển tiếp tới ESP1/ESP2

---

## Cấu trúc phần mềm ESP32

### `ESP32_MQTT_Test/include/device_config.h`
- `ESP_DEVICE_ID = 1` -> ESP1
- `ESP_DEVICE_ID = 2` -> ESP2

Mỗi ESP chỉ hoạt động đúng khi compile với `ESP_DEVICE_ID` tương ứng.

### ESP1
- Client ID: `ESP32_001`
- Device name: `ESP1`
- Publish telemetry:
  - Topic: `esp32/esp1/telemetry`
  - Payload: `{ "temperature": <value>, "humidity": <value>, "timestamp": <seconds> }`
- Subscribe lệnh LED:
  - Topic: `esp32/esp1/cmd/led`
  - Payload: `{ "state": "on" }` hoặc `{ "state": "off" }`
- Publish status:
  - Topic: `esp32/esp1/status`
  - Payload: `{ "status": "online", "rssi": -60 }`
- Last will: `esp32/esp1/status` = `offline` nếu mất kết nối MQTT

### ESP2
- Client ID: `ESP32_002`
- Device name: `ESP2`
- Subscribe telemetry ESP1:
  - Topic: `esp32/esp1/telemetry`
- Publish anomaly result:
  - Topic: `esp32/esp2/anomaly`
  - Payload: `{ "anomaly": true|false, "score": <value> }`
- Subscribe lệnh LED:
  - Topic: `esp32/esp2/cmd/led`
- Publish status:
  - Topic: `esp32/esp2/status`
  - Payload: `{ "status": "online" }`

### Hành vi chung
- ESP1 gửi telemetry mỗi ~2 giây
- ESP1 và ESP2 gửi status mỗi ~10 giây
- ESP2 gửi anomaly mỗi 5 giây nếu đã nhận telemetry từ ESP1
- ESP1 và ESP2 xử lý command LED từ topic `esp32/espX/cmd/led`

---

## Gateway Python `gateway/mqtt_gateway.py`

### Kết nối
- Local MQTT broker: `192.168.137.1:1883`
- CoreIoT server: `app.coreiot.io:1883`
- CoreIoT token: `g5fqNKTTwyhgEkO9NVKC`

### Local subscriptions
Gateway subscribe:
- `esp32/+/telemetry`
- `esp32/+/anomaly`
- `esp32/+/status`

### Chuyển dữ liệu lên CoreIoT
Khi nhận message local ESP:
- parse JSON từ payload
- lấy `device` từ topic (`esp1` hoặc `esp2`)
- publish lên CoreIoT topic: `v1/gateway/telemetry`
- payload gửi lên dạng:
```json
{
  "esp1": [{ "ts": <ms>, "values": { ... } }]
}
```
hoặc
```json
{
  "esp2": [{ "ts": <ms>, "values": { ... } }]
```

### Offline detection
Gateway giữ timestamp lần cuối nhận status cho ESP1 và ESP2.
- Nếu không nhận `esp32/esp1/status` hoặc `esp32/esp2/status` trong 20 giây, gateway sẽ publish offline status lên CoreIoT
- Payload offline gửi theo cùng cấu trúc telemetry với `values: { "status": "offline" }`

### Control từ CoreIoT
Gateway subscribe CoreIoT topic: `v1/gateway/rpc`

Khi gateway nhận message có chứa chuỗi `rpc` trong topic:
- parse JSON payload
- nếu payload có cấu trúc nested:
  - `{ "device":"esp1", "data": { "id": .., "method": ..., "params": ... } }`
- thì thực hiện phương thức:
  - `setStateLed1` -> publish tới `esp32/esp1/cmd/led`
  - `setStateLed2` -> publish tới `esp32/esp2/cmd/led`
- state được chuyển đổi:
  - `true` -> `{ "state":"on" }`
  - `false` -> `{ "state":"off" }`

### RPC response
Gateway gửi response trở lại CoreIoT trên cùng topic `v1/gateway/rpc` với payload dạng:
```json
{
  "device":"esp1",
  "id": <request_id>,
  "data": {
    "result":"ok",
    "state":"on"
  }
}
```

### Debug logging
Gateway in ra:
- topic và payload nhận từ CoreIoT
- khi publish lệnh LED tới ESP
- khi publish telemetry/status/anomaly lên CoreIoT

---

## Dashboard CoreIoT

Dashboard hiện có:
- trạng thái ESP1 và ESP2 (`online`)
- nút `SetLed1` và `SetLed2`
- biểu đồ nhiệt độ và độ ẩm
- biểu đồ anomaly score

### Tương thích với code hiện tại
- Dashboard cần publish control tới topic `v1/gateway/rpc` hoặc topic chứa `rpc`
- payload phải chứa `method` và `params` trong object `data`
- gateway hỗ trợ nested JSON như:
  - `{ "device":"esp1", "data": { "method":"setStateLed1", "params": true } }`
  - `{ "device":"esp2", "data": { "method":"setStateLed2", "params": false } }`

---

## Chạy thử

1. Chạy broker Mosquitto tại `192.168.137.1:1883` (.\mosquitto.exe -c mosquitto.conf -v)
2. Chạy `python gateway/mqtt_gateway.py`
3. Compile ESP1/ESP2 trong `ESP32_MQTT_Test/include/device_config.h` bằng cách đổi `ESP_DEVICE_ID`
4. Nạp ESP1/ESP2 lên board tương ứng
5. Quan sát console gateway để chắc rằng nó:
   - kết nối tới CoreIoT
   - nhận được local message từ ESP
   - nhận được message CoreIoT chứa `rpc`

---

## Ghi chú

- Gateway đang dùng access token `g5fqNKTTwyhgEkO9NVKC` và topic CoreIoT là `v1/gateway/telemetry` / `v1/gateway/rpc`
- Nếu CoreIoT thực tế yêu cầu topic khác, cần điều chỉnh `CORE_IOT_TOPIC` và `CORE_IOT_RPC_TOPIC`
- ESP2 hiện chỉ gửi anomaly khi đã nhận telemetry từ ESP1
- `device_config.h` cần chỉnh đúng thiết bị trước khi build từng ESP
