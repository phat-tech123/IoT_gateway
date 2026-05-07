#ifndef LED_BLINKY_H
#define LED_BLINKY_H

#include <Arduino.h>
#include "global.h"

#define LED_GPIO 48

void led_blinky(void *pvParameters);

#endif // LED_BLINKY_H
