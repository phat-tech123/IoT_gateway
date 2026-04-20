#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"


void setup() {
  Serial.begin(115200);

  // xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  // xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate( temp_humi_monitor, "Task TEMP HUMI Monitor" ,2048  ,NULL  ,2 , NULL);
  // xTaskCreate(lcd_display, "Task display TEMP HUMI on LCD", 2048, NULL, 2, NULL);
  xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
}

void loop() {
  
} 



// #include "LiquidCrystal_I2C.h"
// #include "DHT20.h"
// DHT20 dht20;
// LiquidCrystal_I2C lcd(33,16,2);


// void setup() {
//     Wire.begin(11, 12);
//     Serial.begin(115200);
//     dht20.begin();
// }

// void loop() {

//     dht20.read();

//     float temperature = dht20.getTemperature();
//     float humidity = dht20.getHumidity();

//     if (!isnan(temperature) && !isnan(humidity)) {

//         int label = 0;

//         if (temperature > 35 || temperature < 15 ||
//             humidity > 85 || humidity < 30) {
//             label = 2;
//         }
//         else if (temperature > 30 || temperature < 20 ||
//                  humidity > 75 || humidity < 50) {
//             label = 1;
//         }

//         Serial.print(temperature, 2);
//         Serial.print(",");
//         Serial.print(humidity, 2);
//         Serial.print(",");
//         Serial.println(label);
//     }
//     else {
//         Serial.println("Sensor error");
//     }

//     delay(5000);   // dùng delay thay vì vTaskDelay trong loop()
// }