#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "app.coreiot.io";  
const char* coreIOT_Token = "oeuwvs100yiomk81udoa";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);

// Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    if (client.connect("ESP32Client", coreIOT_Token, NULL)) {
      Serial.println("connected to ThingsBoard!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Copy payload an toàn
  char message[256];
  if (length >= sizeof(message)) {
    Serial.println("Payload too large!");
    return;
  }

  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Payload: ");
  Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Kiểm tra tồn tại method
  if (!doc.containsKey("method")) {
    Serial.println("No method field!");
    return;
  }

  const char* method = doc["method"];
  if (method == NULL) return;

  // ===== LED1 =====
  if (strcmp(method, "setValueLED1") == 0) {

    bool state = doc["params"].as<bool>();

    Serial.print("LED1 state: ");
    Serial.println(state);

    digitalWrite(48, state ? HIGH : LOW);
  }

  // ===== LED2 =====
  else if (strcmp(method, "setValueLED2") == 0) {

    bool state = doc["params"].as<bool>();

    Serial.print("LED2 state: ");
    Serial.println(state);
    if(state){
      strip.setPixelColor(0, strip.Color(0, 255, 0));
      strip.show();
    }
    else{
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
    }
  }

  else {
    Serial.print("Unknown method: ");
    Serial.println(method);
  }
}

// void callback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.println("] ");

//   // Allocate a temporary buffer for the message
//   char message[length + 1];
//   memcpy(message, payload, length);
//   message[length] = '\0';
//   Serial.print("Payload: ");
//   Serial.println(message);

//   // Parse JSON
//   StaticJsonDocument<256> doc;
//   DeserializationError error = deserializeJson(doc, message);

//   if (error) {
//     Serial.print("deserializeJson() failed: ");
//     Serial.println(error.c_str());
//     return;
//   }

//   const char* method = doc["method"];
//   if (strcmp(method, "setValueLED1") == 0) {
//     // Check params type (could be boolean, int, or string according to your RPC)
//     // Example: {"method": "setValueLED", "params": "ON"}
//     const char* params = doc["params"];

//     if (strcmp(params, "ON") == 0) {
//       Serial.println("Device turned ON.");
//       //TODO

//     } else {   
//       Serial.println("Device turned OFF.");
//       //TODO

//     }
//   } else {
//     Serial.print("Unknown method: ");
//     Serial.println(method);
//   }

  
//   if (strcmp(method, "setValueLED2") == 0) {
//     // Check params type (could be boolean, int, or string according to your RPC)
//     // Example: {"method": "setValueLED", "params": "ON"}
//     const char* params = doc["params"];

//     if (strcmp(params, "ON") == 0) {
//       Serial.println("Device turned ON.");
//       //TODO

//     } else {   
//       Serial.println("Device turned OFF.");
//       //TODO

//     }
//   } else {
//     Serial.print("Unknown method: ");
//     Serial.println(method);
//   }
// }


void setup_coreiot(){
  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(" Connected!");

  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(callback);

}

void  coreiot_task(void *pvParameters){

    setup_coreiot();
    
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();
    while(1){

        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        // Sample payload, publish to 'v1/devices/me/telemetry'
        String payload = "{\"temperature\":" + String(glob_temperature) +  ",\"humidity\":" + String(glob_humidity) + "}";
        
        client.publish("v1/devices/me/telemetry", payload.c_str());

        Serial.println("Published payload: " + payload);
        vTaskDelay(1000);  // Publish every 1 seconds
    }
}
