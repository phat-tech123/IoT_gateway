#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define LED_PIN 48
#define NEO_PIN 45
#define LED_COUNT 1 
#define BOOT_PIN 0

extern Adafruit_NeoPixel strip;

//task 1 
// extern QueueHandle_t tempQueue;
extern SemaphoreHandle_t tempCold;
extern SemaphoreHandle_t tempWarm;
extern SemaphoreHandle_t tempHot;

//task 2 
// extern QueueHandle_t humiQueue;
extern SemaphoreHandle_t humiCold;
extern SemaphoreHandle_t humiWarm;
extern SemaphoreHandle_t humiHot;

//task 3
typedef enum {
    NORMAL,
    WARNING,
    CRITICAL
} display_state_t;
extern QueueHandle_t stateQueue;


//task 4
extern float glob_temperature;
extern float glob_humidity;
extern SemaphoreHandle_t sensorMutex;

extern float ai_prob;
extern float ai_class;
extern SemaphoreHandle_t aiMutex;

extern float temp_warn;
extern float temp_crit;
extern float humi_warn;
extern float humi_crit;
extern SemaphoreHandle_t thresholdSemaphore;

//Task 6
extern SemaphoreHandle_t xBinarySemaphoreInternet;

#endif