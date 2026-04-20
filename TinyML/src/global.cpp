#include "global.h"


Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Task 1
// QueueHandle_t tempQueue =  xQueueCreate(1, sizeof(float));
SemaphoreHandle_t tempCold = xSemaphoreCreateBinary();
SemaphoreHandle_t tempWarm = xSemaphoreCreateBinary();
SemaphoreHandle_t tempHot = xSemaphoreCreateBinary();



// Task 2
// QueueHandle_t humiQueue =  xQueueCreate(1, sizeof(float));
SemaphoreHandle_t humiCold = xSemaphoreCreateBinary();
SemaphoreHandle_t humiWarm = xSemaphoreCreateBinary();
SemaphoreHandle_t humiHot = xSemaphoreCreateBinary();

//Task 3
QueueHandle_t stateQueue = xQueueCreate(1, sizeof(display_state_t));

//Task 4
SemaphoreHandle_t aiMutex = xSemaphoreCreateMutex();
float ai_prob = 0;
float ai_class = 0;

SemaphoreHandle_t sensorMutex = xSemaphoreCreateMutex();
float glob_temperature = 0;
float glob_humidity = 0;


SemaphoreHandle_t thresholdSemaphore = xSemaphoreCreateBinary();
float temp_warn = 30.0;
float temp_crit = 32.0;
float humi_warn = 70.0;
float humi_crit = 80.0;

//Task 6
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

