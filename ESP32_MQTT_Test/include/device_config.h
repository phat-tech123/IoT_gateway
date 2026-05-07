#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

// Chọn thiết bị khi biên dịch: 1 = ESP1, 2 = ESP2
#define ESP_DEVICE_ID 2

#if ESP_DEVICE_ID == 1
#define ESP_IS_ESP1 1
#define ESP_IS_ESP2 0
#define ESP_CLIENT_ID "ESP32_001"
#define ESP_DEVICE_NAME "ESP1"
#define ESP_PUBLISH_TELEMETRY_TOPIC "esp32/esp1/telemetry"
#define ESP_SUBSCRIBE_LED_TOPIC "esp32/esp1/cmd/led"
#define ESP_STATUS_TOPIC "esp32/esp1/status"
#define ESP_SUBSCRIBE_TELEMETRY_TOPIC ""
#define ESP_PUBLISH_ANOMALY_TOPIC ""
#elif ESP_DEVICE_ID == 2
#define ESP_IS_ESP1 0
#define ESP_IS_ESP2 1
#define ESP_CLIENT_ID "ESP32_002"
#define ESP_DEVICE_NAME "ESP2"
#define ESP_SUBSCRIBE_TELEMETRY_TOPIC "esp32/esp1/telemetry"
#define ESP_PUBLISH_ANOMALY_TOPIC "esp32/esp2/anomaly"
#define ESP_SUBSCRIBE_LED_TOPIC "esp32/esp2/cmd/led"
#define ESP_STATUS_TOPIC "esp32/esp2/status"
#define ESP_PUBLISH_TELEMETRY_TOPIC ""
#else
#error "ESP_DEVICE_ID phải là 1 hoặc 2"
#endif

#endif // DEVICE_CONFIG_H
