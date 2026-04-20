#include "neo_blinky.h"


void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    while(1) {           
        if(xSemaphoreTake(humiCold, 0) == pdTRUE) {
            strip.setPixelColor(0, strip.Color(0, 0, 255)); 
        } 
        else if(xSemaphoreTake(humiWarm, 0) == pdTRUE){
            strip.setPixelColor(0, strip.Color(0, 255, 0)); 
        }
        else if(xSemaphoreTake(humiHot, 0) == pdTRUE){
            strip.setPixelColor(0, strip.Color(255, 0, 0)); 
        }
        // float humi;                      
        // if(xQueueReceive(humiQueue, &humi, 0) == pdTRUE){
        //     if(humi < 50.0) {
        //         strip.setPixelColor(0, strip.Color(0, 0, 255)); 
        //     } else if(humi < 70.0){
        //         strip.setPixelColor(0, strip.Color(0, 255, 0)); 
        //     } else{
        //         strip.setPixelColor(0, strip.Color(255, 0, 0)); 
        //     }
        // }
        strip.show();
        vTaskDelay(500);
    }
}