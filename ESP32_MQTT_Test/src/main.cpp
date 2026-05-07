#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "global.h"
#include "device_config.h"
#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "tinyml.h"

const char* ssid = "Tu";
const char* password = "123456789";
const char* mqtt_server = "192.168.137.1";

WiFiClient espClient;
PubSubClient client(espClient);

float glob_temperature = 0.0;
float glob_humidity = 0.0;
float ai_result = 0.0;
boolean led_enable = false;
boolean alarm_warning = false;
boolean telemetry_available = false;

unsigned long lastTelemetryPublish = 0;
unsigned long lastStatusPublish = 0;
unsigned long lastAnomalyPublish = 0;

float parseJsonFloat(const String& message, const String& key) {
  int idx = message.indexOf(key);
  if (idx < 0) return NAN;
  int colon = message.indexOf(':', idx);
  if (colon < 0) return NAN;
  int endChar = message.indexOf(',', colon);
  if (endChar < 0) endChar = message.indexOf('}', colon);
  if (endChar < 0) endChar = message.length();
  String token = message.substring(colon + 1, endChar);
  token.trim();
  token.replace("\"", "");
  return token.toFloat();
}

String parseJsonString(const String& message, const String& key) {
  int idx = message.indexOf(key);
  if (idx < 0) return String();
  int colon = message.indexOf(':', idx);
  if (colon < 0) return String();
  int quoteStart = message.indexOf('"', colon);
  if (quoteStart < 0) return String();
  int quoteEnd = message.indexOf('"', quoteStart + 1);
  if (quoteEnd < 0) return String();
  return message.substring(quoteStart + 1, quoteEnd);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr(topic);
  String message = String((char*)payload, length);
  Serial.print("MQTT callback topic=");
  Serial.print(topicStr);
  Serial.print(" message=");
  Serial.println(message);

  if (topicStr == ESP_SUBSCRIBE_LED_TOPIC) {
    String state = parseJsonString(message, "\"state\"");
    if (state.equalsIgnoreCase("on")) {
      led_enable = true;
      Serial.println("LED command ON");
    } else if (state.equalsIgnoreCase("off")) {
      led_enable = false;
      Serial.println("LED command OFF");
    }
  }

#if ESP_IS_ESP2
  if (topicStr == ESP_SUBSCRIBE_TELEMETRY_TOPIC) {
    float temperature = parseJsonFloat(message, "\"temperature\"");
    float humidity = parseJsonFloat(message, "\"humidity\"");
    if (!isnan(temperature) && !isnan(humidity)) {
      glob_temperature = temperature;
      glob_humidity = humidity;
      telemetry_available = true;
      Serial.print("Received ESP1 telemetry: ");
      Serial.print(temperature);
      Serial.print(" C, ");
      Serial.print(humidity);
      Serial.println(" %");
    }
  }
#endif
}

bool connectMqtt() {
  if (client.connected()) {
    return true;
  }

  Serial.print("Connecting MQTT as ");
  Serial.println(ESP_CLIENT_ID);

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);

  if (client.connect(ESP_CLIENT_ID, NULL, NULL, ESP_STATUS_TOPIC, 1, true, "{\"status\":\"offline\"}")) {
    Serial.println("MQTT connected");
#if ESP_IS_ESP1
    client.subscribe(ESP_SUBSCRIBE_LED_TOPIC);
#endif
#if ESP_IS_ESP2
    client.subscribe(ESP_SUBSCRIBE_TELEMETRY_TOPIC);
    client.subscribe(ESP_SUBSCRIBE_LED_TOPIC);
#endif
    return true;
  }

  Serial.print("MQTT connect failed, rc=");
  Serial.println(client.state());
  return false;
}

void publishStatus() {
  if (!client.connected()) {
    return;
  }

  String payload = "{";
#if ESP_IS_ESP1
  payload += "\"status\":\"online\",";
  payload += "\"rssi\":" + String(WiFi.RSSI());
#else
  payload += "\"ai_running\":true";
#endif
  payload += "}";
  client.publish(ESP_STATUS_TOPIC, payload.c_str(), true);
}

void publishTelemetry() {
  if (!client.connected() || !ESP_IS_ESP1) {
    return;
  }

  String payload = "{";
  payload += "\"temperature\":" + String(glob_temperature, 2) + ",";
  payload += "\"humidity\":" + String(glob_humidity, 2) + ",";
  payload += "\"timestamp\":" + String(millis() / 1000);
  payload += "}";
  client.publish(ESP_PUBLISH_TELEMETRY_TOPIC, payload.c_str());
}

void publishAnomaly() {
  if (!client.connected() || !ESP_IS_ESP2 || !telemetry_available) {
    return;
  }

  String payload = "{";
  payload += "\"anomaly\":" + String(alarm_warning ? "true" : "false") + ",";
  payload += "\"score\":" + String(ai_result, 2);
  payload += "}";
  client.publish(ESP_PUBLISH_ANOMALY_TOPIC, payload.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("Starting ");
  Serial.println(ESP_DEVICE_NAME);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  while (!connectMqtt()) {
    delay(2000);
  }

#if ESP_IS_ESP1
  xTaskCreate(temp_humi_monitor, "TempHumiMonitor", 4096, NULL, 2, NULL);
#endif
#if ESP_IS_ESP2
  xTaskCreate(tiny_ml_task, "TinyMLTask", 8192, NULL, 2, NULL);
#endif
  xTaskCreate(led_blinky, "LedBlinky", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "NeoBlinky", 4096, NULL, 2, NULL);
}

void loop() {
  if (!client.connected()) {
    if (!connectMqtt()) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      return;
    }
  }

  client.loop();

  unsigned long nowMillis = millis();

#if ESP_IS_ESP1
  if (nowMillis - lastTelemetryPublish >= 2000) {
    publishTelemetry();
    lastTelemetryPublish = nowMillis;
  }
#endif

  if (nowMillis - lastStatusPublish >= 10000) {
    publishStatus();
    lastStatusPublish = nowMillis;
  }

#if ESP_IS_ESP2
  if (nowMillis - lastAnomalyPublish >= 5000) {
    publishAnomaly();
    lastAnomalyPublish = nowMillis;
  }
#endif

  vTaskDelay(100 / portTICK_PERIOD_MS);
}
