#include "temp_humi_monitor.h"
DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

void temp_humi_monitor(void *pvParameters){
    
    Wire.begin(11, 12);
    // Serial.begin(115200);

    float tw = temp_warn;
    float tc = temp_crit; 
    float hw = humi_warn;
    float hc = humi_crit;
    dht20.begin();
    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        
        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            // return;
        }


        // Task 1
        if(temperature >= tc){
            xSemaphoreGive(tempHot); 
        } else if(temperature >= tw){
            xSemaphoreGive(tempCold);   
        } else {
            xSemaphoreGive(tempWarm);  
        }
        
        // Task 2
        if(humidity >= hc){
            xSemaphoreGive(humiHot); 
        } else if(humidity >= hw){
            xSemaphoreGive(humiCold);
        } else {
            xSemaphoreGive(humiWarm); 
        }

        // Task 3
        display_state_t state;
        if (temperature >= tc || humidity >= hc){
            state = CRITICAL;
        } 
        else if (temperature >= tw || humidity >= hw){
            state = WARNING;
        } 
        else{
            state = NORMAL;
        }
        xQueueOverwrite(stateQueue, &state);

        // Task 4
        xSemaphoreTake(sensorMutex, portMAX_DELAY);
        glob_temperature = temperature;
        glob_humidity = humidity;
        xSemaphoreGive(sensorMutex);

        if (xSemaphoreTake(thresholdSemaphore, 0) == pdTRUE) {
            tw = temp_warn;
            tc = temp_crit;
            hw = humi_warn;
            hc = humi_crit;
        }
        
        // Serial.print("Humidity: ");
        // Serial.print(humidity);
        // Serial.print("%  Temperature: ");
        // Serial.print(temperature);
        // Serial.println("°C");

        vTaskDelay(5000);
    }
    
}

void lcd_display(void *pvParameters) {
    lcd.begin();
    while (1) {
        display_state_t state;
        if(xQueueReceive(stateQueue, &state, 0) == pdTRUE){
            
            lcd.clear();
            if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
                float t = glob_temperature;
                float h = glob_humidity;
                
                lcd.setCursor(0, 0);
                lcd.print(h);
                lcd.setCursor(0, 1);
                lcd.print(t);
                xSemaphoreGive(sensorMutex);
            }
            switch(state){
                case CRITICAL:
                    Serial.print("CRITICAL\n");
                    lcd.setCursor(7, 0);
                    lcd.print("CRITICAL");
                    break;

                case WARNING:
                    Serial.print("WARNING\n");
                    
                    lcd.setCursor(7, 0);
                    lcd.print("WARNING");
                    break;

                case NORMAL:
                    Serial.print("NORMAL\n");
                    
                    lcd.setCursor(7, 0);
                    lcd.print("NORMAL");
                    break;

                default:
                    break;
            }
        }
        vTaskDelay(5000);
    }
}

