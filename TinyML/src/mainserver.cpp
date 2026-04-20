#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>

bool led1_state = false;
bool led2_state = false;
bool isAPMode = true;

WebServer server(80);
// Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

String ssid = "ESP32-IOTWebServer";
String password = "12345678";
String wifi_ssid = "";
String wifi_password = "";

unsigned long connect_start_ms = 0;
bool connecting = false;

String mainPage() {
  // float temperature = glob_temperature;
  // float humidity = glob_humidity;
  String led1 = led1_state ? "ON" : "OFF";
  String led2 = led2_state ? "ON" : "OFF";

  return R"rawliteral(
<!DOCTYPE html><html><head>
  <meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>ESP32 Premium Dashboard</title>
  <style>
    :root { 
      --bg: #0b0f1a; --card: #161b2d; --text: #f8fafc; --subtext: #94a3b8;
      --temp: #ff4d6d; --humi: #00d2ff; --accent: #6366f1;
      --border: rgba(255,255,255,0.08);
    }

    body { 
      font-family: 'Segoe UI', system-ui, sans-serif; background: var(--bg); color: var(--text); 
      margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; 
    }
    
    .wrapper { 
      display: grid; grid-template-columns: repeat(auto-fit, minmax(340px, 1fr)); 
      gap: 25px; width: 100%; max-width: 1100px; 
    }

    /* Card Styling */
    .card { 
      background: var(--card); border-radius: 28px; padding: 25px; 
      box-shadow: 0 20px 40px rgba(0,0,0,0.4); border: 1px solid var(--border);
      transition: transform 0.3s ease;
    }
    .card:hover { transform: translateY(-5px); }

    h3 { 
      margin: 0 0 20px 0; font-size: 12px; color: var(--subtext); 
      text-transform: uppercase; letter-spacing: 2px; font-weight: 800; 
    }

    /* Uniform Row Styling */
    .row-item {
      display: flex; align-items: center; justify-content: space-between;
      margin-bottom: 15px; padding: 15px; border-radius: 18px;
      background: rgba(255,255,255,0.03); border: 1px solid transparent;
      transition: 0.3s;
    }
    .row-item:hover { background: rgba(255,255,255,0.05); border-color: rgba(255,255,255,0.1); }
    
    .row-item span { font-size: 15px; font-weight: 500; color: var(--subtext); }

    /* Right Side Content */
    .row-content { display: flex; gap: 10px; align-items: center; }

    /* Values and Inputs */
    .stat-val { font-size: 26px; font-weight: 700; font-variant-numeric: tabular-nums; }
    
    input { 
      width: 75px; text-align: center; padding: 10px 5px; border-radius: 12px; 
      border: 1.5px solid #2d3748; background: #0b0f1a; color: white; 
      font-weight: 700; outline: none; transition: 0.3s;
    }
    input:focus { border-color: var(--accent); box-shadow: 0 0 10px rgba(99, 102, 241, 0.3); }
    input::placeholder { font-size: 10px; opacity: 0.4; text-transform: uppercase; }

    /* Buttons */
    button { 
      width: 100%; padding: 15px; border: none; border-radius: 16px; 
      cursor: pointer; font-weight: 700; transition: 0.3s; 
      background: #2d3748; color: white; margin-top: 10px; font-size: 14px;
    }
    button:hover { background: #3d495d; box-shadow: 0 5px 15px rgba(0,0,0,0.3); }

    .btn-apply { 
      background: linear-gradient(135deg, #6366f1, #4f46e5); color: white; 
      margin-top: 20px; box-shadow: 0 10px 20px rgba(99, 102, 241, 0.2);
    }
    .btn-apply:hover { filter: brightness(1.1); }

    .btn-wifi { 
      background: transparent; color: var(--subtext); 
      border: 1.5px dashed #2d3748; margin-top: 20px; font-size: 11px; letter-spacing: 1px;
    }

    @media (max-width: 600px) { .wrapper { grid-template-columns: 1fr; } body { padding: 10px; } }
  </style>
</head>
<body>

<div class="wrapper">
  <div class="card">
    <h3>📍 Live Metrics</h3>
    <div class="row-item" style="border-right: 4px solid var(--temp)">
      <span>Temperature</span>
      <div class="row-content">
        <b id="txt-t" class="stat-val" style="color:var(--temp)">--°</b>
      </div>
    </div>
    <div class="row-item" style="border-right: 4px solid var(--humi)">
      <span>Humidity</span>
      <div class="row-content">
        <b id="txt-h" class="stat-val" style="color:var(--humi)">--%</b>
      </div>
    </div>
  </div>

  <div class="card">
    <h3>⚙ Thresholds</h3>
    <div class="row-item">
      <span>Temp</span>
      <div class="row-content">
        <input id="tw" type="number" placeholder="Warn">
        <input id="tc" type="number" placeholder="Crit">
      </div>
    </div>
    <div class="row-item">
      <span>Humi</span>
      <div class="row-content">
        <input id="hw" type="number" placeholder="Warn">
        <input id="hc" type="number" placeholder="Crit">
      </div>
    </div>
    <button class="btn-apply" onclick="save()">APPLY CONFIGURATION</button>
  </div>

  <div class="card">
  <h3>🤖 AI Prediction</h3>

  <div class="row-item" style="border-right: 4px solid var(--ai)">
    <span>Class</span>
    <div class="row-content">
      <b id="txt-ai-class" class="stat-val" style="color:var(--ai)">--</b>
    </div>
  </div>

  <div class="row-item" style="border-right: 4px solid var(--ai)">
    <span>Confidence</span>
    <div class="row-content">
      <b id="txt-ai-prob" class="stat-val" style="color:var(--ai)">--%</b>
    </div>
  </div>
</div>

  <div class="card">
    <h3>💡 Remote Control</h3>
    <button onclick="toggle(1)">LED 1: <b id="l1">)rawliteral" + led1 + R"rawliteral(</b></button>
    <button onclick="toggle(2)">LED 2: <b id="l2">)rawliteral" + led2 + R"rawliteral(</b></button>
    <button class="btn-wifi" onclick="window.location='/settings'">SYSTEM WIFI SETTINGS</button>
  </div>
</div>

<script>
  function toggle(id) {
    fetch('/toggle?led='+id).then(r=>r.json()).then(j=>{
      document.getElementById('l1').innerText=j.led1;
      document.getElementById('l2').innerText=j.led2;
    });
  }

  function save() {
    const params = `tw=${tw.value}&tc=${tc.value}&hw=${hw.value}&hc=${hc.value}`;
    fetch(`/threshold?${params}`).then(()=>alert("✅ Settings Synced Successfully!"));
  }

  setInterval(() => {
    fetch('/sensors').then(r=>r.json()).then(d => {
      document.getElementById('txt-t').innerText = d.temp.toFixed(1) + "°";
      document.getElementById('txt-h').innerText = d.hum.toFixed(1) + "%";
      // THÊM 2 DÒNG NÀY:
      document.getElementById('txt-ai-class').innerText = d.ai_class; 
      document.getElementById('txt-ai-prob').innerText = d.ai_prob.toFixed(1) + "%";
    });
  }, 3000);
</script>
</body></html>
)rawliteral";
}

String settingsPage() {
  return R"rawliteral(
    <!DOCTYPE html><html><head>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>Settings</title>
      <style>
        body { font-family: Arial; text-align:center; margin:0;}
        .container { margin:20px auto; max-width:350px;background:#f9f9f9;border-radius:10px;box-shadow:0 2px 10px #ccc;padding:20px;}
        input[type=text], input[type=password]{width:90%;padding:10px;}
        button { padding:10px 15px; margin:10px; font-size:18px;}
      </style>
    </head>
    <body>
      <div class='container'>
        <h2>Wi-Fi Settings</h2>
        <form id="wifiForm">
          <input name="ssid" id="ssid" placeholder="SSID" required><br>
          <input name="password" id="pass" type="password" placeholder="Password" required><br><br>
          <button type="submit">Connect</button>
          <button type="button" onclick="window.location='/'">Back</button>
        </form>
        <div id="msg"></div>
      </div>
      <script>
        document.getElementById('wifiForm').onsubmit = function(e){
          e.preventDefault();
          let ssid = document.getElementById('ssid').value;
          let pass = document.getElementById('pass').value;
          fetch('/connect?ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass))
            .then(r=>r.text())
            .then(msg=>{
              document.getElementById('msg').innerText=msg;
            });
        };
      </script>
    </body></html>
  )rawliteral";
}

// ========== Handlers ==========
void handleRoot() { server.send(200, "text/html", mainPage()); }

void handleToggle() {
  int led = server.arg("led").toInt();
  if (led == 1) {
    led1_state = !led1_state;
    digitalWrite(LED_PIN, led1_state ? HIGH : LOW);
  }
  else if (led == 2){
    led2_state = !led2_state;
    if(led2_state){
      strip.setPixelColor(0, strip.Color(255, 0, 0));
      strip.show();
    }
    else{
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
    }
  }

  server.send(200, "application/json",
    "{\"led1\":\"" + String(led1_state ? "ON":"OFF") +
    "\",\"led2\":\"" + String(led2_state ? "ON":"OFF") + "\"}");
}

// void handleSensors() {
//   float t;
//   float h;
//   float ai_p;
//   float ai_c;
//   if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
//     t = glob_temperature;
//     h = glob_humidity;
//     xSemaphoreGive(sensorMutex);
//   }
//   if (xSemaphoreTake(aiMutex, portMAX_DELAY) == pdTRUE) {
//     ai_p = ai_prob;
//     ai_c = ai_class;
//     xSemaphoreGive(aiMutex);
//   }
  
//   // String json = "{\"temp\":"+String(t)+",\"hum\":"+String(h)+"}";
//   String json = "{";
//   json += "\"temp\":" + String(t) + ",";
//   json += "\"hum\":" + String(h) + ",";
//   json += "\"ai_prob\":" + String(ai_p*100) + ",";
//   json += "\"ai_class\":" + (ai_c == 0) ? "NOMRMAL" : (ai_c == 1) ? "WARINING" : "CRITICAL";
//   json += "}";
//   server.send(200, "application/json", json);
// }
void handleSensors() {
  float t = 0, h = 0, ai_p = 0, ai_c = 0;

  if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
    t = glob_temperature;
    h = glob_humidity;
    xSemaphoreGive(sensorMutex);
  }
  if (xSemaphoreTake(aiMutex, portMAX_DELAY) == pdTRUE) {
    ai_p = ai_prob;
    ai_c = ai_class;
    xSemaphoreGive(aiMutex);
  }

  // Xác định nhãn text cho AI Class
  String label;
  if (ai_c == 0) label = "NORMAL";
  else if (ai_c == 1) label = "WARNING";
  else label = "CRITICAL";

  // Tạo chuỗi JSON thủ công
  String json = "{";
  json += "\"temp\":" + String(t, 1) + ",";
  json += "\"hum\":" + String(h, 1) + ",";
  json += "\"ai_prob\":" + String(ai_p * 100.0, 1) + ","; // Nhân 100 để ra %
  json += "\"ai_class\":\"" + label + "\""; // Cần có dấu \" bao quanh label
  json += "}";

  server.send(200, "application/json", json);
}
void handleThreshold() {
  // xSemaphoreTake(thresholdMutex, portMAX_DELAY);
  temp_warn = server.arg("tw").toFloat();
  temp_crit = server.arg("tc").toFloat();
  humi_warn = server.arg("hw").toFloat();
  humi_crit = server.arg("hc").toFloat();
  xSemaphoreGive(thresholdSemaphore);

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}


void handleSettings() { server.send(200, "text/html", settingsPage()); }

void handleConnect() {
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");
  server.send(200, "text/plain", "Connecting....");
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  connectToWiFi();
}

// ========== WiFi ==========
void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/threshold", HTTP_GET, handleThreshold);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/connect", HTTP_GET, handleConnect);
  server.begin();
}

void startAP() {
  WiFi.disconnect(true);   // Clear STA

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi() {
  WiFi.disconnect(true);   // Clear previous connection

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid.c_str());
  Serial.print(wifi_password.c_str());
  
  Serial.println(wifi_ssid);
}

// ========== Main task ==========
void main_server_task(void *pvParameters){
  pinMode(BOOT_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  
  strip.begin();
  // Set all pixels to off to start
  strip.clear();
  strip.show();


  startAP();
  setupServer();

  while(1){
    server.handleClient();

    // BOOT Button to switch to AP Mode
    if (digitalRead(BOOT_PIN) == LOW) {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW) {
        if (!isAPMode) {
          startAP();
          setupServer();
        }
      }
    }
    
    // STA Mode
    if (connecting) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
        connecting = false;
      } else if (millis() - connect_start_ms > 10000) { // timeout 10s
        Serial.println("WiFi connect failed! Back to AP.");
        startAP();
        setupServer();
        connecting = false;
      }
    }
    vTaskDelay(200); // avoid watchdog reset
  }
}



// String mainPage() {
//   float temperature = glob_temperature;
//   float humidity = glob_humidity;
//   String led1 = led1_state ? "ON" : "OFF";
//   String led2 = led2_state ? "ON" : "OFF";

//   return R"rawliteral(
//     <!DOCTYPE html><html><head>
//       <meta name='viewport' content='width=device-width, initial-scale=1.0'>
//       <title>ESP32 Dashboard</title>
//       <style>
//         body { font-family: Arial;text-align:center; margin:0;}
//         .container { margin:20px auto; max-width:350px; background:#f9f9f9; border-radius:10px; box-shadow:0 2px 10px #ccc;padding:20px;}
//         button { padding:10px 15px; margin:10px; font-size:18px;}
//         #settings { float:right; background:#007bff;color:white; border-radius:4px;}
//       </style>
//     </head>
//     <body>
//       <div class='container'>
//         <h2>ESP32 Dashboard</h2>
//         <div>
//           <b>Temperature:</b> <span id='temp'>)rawliteral" + String(temperature) + R"rawliteral(</span> &deg;C<br>
//           <b>Humidity:</b> <span id='hum'>)rawliteral" + String(humidity) + R"rawliteral(</span> %<br>
//         </div>
//         <div>
//             <button onclick='toggleLED(1)'>LED1: <span id="l1">)rawliteral" + led1 + R"rawliteral(</span></button>
//             <button onclick='toggleLED(2)'>LED2: <span id="l2">)rawliteral" + led2 + R"rawliteral(</span></button>
//         </div>
//         <button id="settings" onclick="window.location='/settings'">&#9881; Settings</button>
//       </div>
//       <script>
//         function toggleLED(id) {
//           fetch('/toggle?led='+id)
//           .then(response=>response.json())
//           .then(json=>{
//             document.getElementById('l1').innerText=json.led1;
//             document.getElementById('l2').innerText=json.led2;
//           });
//         }
//         setInterval(()=>{
//           fetch('/sensors')
//            .then(res=>res.json())
//            .then(d=>{
//              document.getElementById('temp').innerText=d.temp;
//              document.getElementById('hum').innerText=d.hum;
//            });
//         },3000);
//       </script>
//     </body></html>
//   )rawliteral";
// }