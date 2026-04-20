#include "led_blinky.h"

void led_blinky(void *pvParameters){
  pinMode(LED_GPIO, OUTPUT);
  while(1) {
    TickType_t delay;
    if (xSemaphoreTake(tempCold, 0) == pdTRUE) {
        delay = pdMS_TO_TICKS(5000);
    }
    else if (xSemaphoreTake(tempWarm, 0) == pdTRUE) {
        delay = pdMS_TO_TICKS(1000);
    }
    else if (xSemaphoreTake(tempHot, 0) == pdTRUE) {
        delay = pdMS_TO_TICKS(100);
    }
    
    
    digitalWrite(LED_GPIO, HIGH);
    vTaskDelay(delay);
    digitalWrite(LED_GPIO, LOW);
    vTaskDelay(delay);
  }
}