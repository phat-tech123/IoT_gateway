#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "global.h"
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
boolean led_enable = true;
boolean alarm_warning = false;

void setup_wifi() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 MQTT + DHT20 + TinyML demo...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("ESP32_001")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void publishTelemetry() {
  if (!client.connected()) {
    reconnect();
  }

  String payload = "{";
  payload += "\"temperature\":" + String(glob_temperature, 2) + ",";
  payload += "\"humidity\":" + String(glob_humidity, 2) + ",";
  payload += "\"anomaly_score\":" + String(ai_result, 2) + ",";
  payload += "\"alarm\":" + String(alarm_warning ? 1 : 0);
  payload += "}";

  if (client.publish("esp32/ESP32_001/telemetry", payload.c_str())) {
    Serial.println("Published telemetry: " + payload);
  } else {
    Serial.println("Failed to publish telemetry");
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  xTaskCreate(temp_humi_monitor, "TempHumiMonitor", 4096, NULL, 2, NULL);
  xTaskCreate(tiny_ml_task, "TinyMLTask", 8192, NULL, 2, NULL);
  xTaskCreate(led_blinky, "LedBlinky", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "NeoBlinky", 4096, NULL, 2, NULL);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  publishTelemetry();
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}
