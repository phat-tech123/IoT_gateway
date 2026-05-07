#ifndef NEO_BLINKY_H
#define NEO_BLINKY_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "global.h"

#define NEO_PIN 45
#define LED_COUNT 1

void neo_blinky(void *pvParameters);

#endif // NEO_BLINKY_H
