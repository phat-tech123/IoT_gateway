#include "temp_humi_monitor.h"

DHT20 dht20;

void temp_humi_monitor(void *pvParameters) {
  Wire.begin(11, 12);
  dht20.begin();

  while (1) {
    if (dht20.read()) {
      float temperature = dht20.getTemperature();
      float humidity = dht20.getHumidity();

      if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT20 sensor!");
        temperature = -1;
        humidity = -1;
      }

      glob_temperature = temperature;
      glob_humidity = humidity;

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
    } else {
      Serial.println("DHT20 read failed");
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
