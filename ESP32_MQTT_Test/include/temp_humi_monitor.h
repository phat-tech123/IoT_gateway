#ifndef TEMP_HUMI_MONITOR_H
#define TEMP_HUMI_MONITOR_H

#include <Arduino.h>
#include <Wire.h>
#include <DHT20.h>
#include "global.h"

void temp_humi_monitor(void *pvParameters);

#endif // TEMP_HUMI_MONITOR_H
