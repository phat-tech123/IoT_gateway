#include "led_blinky.h"

void led_blinky(void *pvParameters) {
  pinMode(LED_GPIO, OUTPUT);

  while (1) {
    if (led_enable) {
      digitalWrite(LED_GPIO, HIGH);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      digitalWrite(LED_GPIO, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else {
      digitalWrite(LED_GPIO, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}
