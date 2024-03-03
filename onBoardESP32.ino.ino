#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <driver/dac.h>

#define DAC_PIN_1 25  // for LEFT Motor
#define DAC_PIN_2 26  // for RIGHT Motor

const char *ssid = "MyWiFiCar";
const char *password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

#define UP 1
#define RIGHT 2
#define LEFT 3
#define STOP 0

void moveCar(int direction, int voltageValue) {
  switch (direction) {
    case UP:
      dacWrite(DAC_PIN_1, voltageValue);
      dacWrite(DAC_PIN_2, voltageValue);
      Serial.println("Moving UP Voltage:");
      Serial.println(voltageValue);
      break;
    case RIGHT:
      dacWrite(DAC_PIN_1, voltageValue);
      Serial.println("Moving RIGHT Voltage:");
      Serial.println(voltageValue);
      break;
    case LEFT:
      dacWrite(DAC_PIN_2, voltageValue);
      Serial.println("Moving LEFT Voltage:");
      Serial.println(voltageValue);
      break;
    case STOP:
     delay(1000);
      dacWrite(DAC_PIN_1, 0);
      dacWrite(DAC_PIN_2, 0);
      Serial.println("STOPPED");
       delay(1000);
      break;
    default:
      dacWrite(DAC_PIN_1, 0);
      dacWrite(DAC_PIN_2, 0);
      Serial.println("Unknown command, STOPPED");
      break;
  }
}


void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html",
                  "<!DOCTYPE html>\
    <html>\
      <head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
        <style>\
          .slidecontainer {\
            width: 100%;\
          }\
          .slider {\
            -webkit-appearance: none;\
            width: 100%;\
            height: 25px;\
            border-radius: 5px;\
            background: #d3d3d3;\
            outline: none;\
            opacity: 0.7;\
            -webkit-transition: .2s;\
            transition: opacity .2s;\
          }\
          .slider:hover {\
            opacity: 1;\
          }\
          .slider::-webkit-slider-thumb {\
            -webkit-appearance: none;\
            appearance: none;\
            width: 25px;\
            height: 25px;\
            border-radius: 50%;\
            background: #4CAF50;\
            cursor: pointer;\
          }\
          .slider::-moz-range-thumb {\
            width: 25px;\
            height: 25px;\
            border-radius: 50%;\
            background: #4CAF50;\
            cursor: pointer;\
          }\
        </style>\
      </head>\
      <body>\
        <h1>ESP32 Analog Voltage Control</h1>\
        <div class=\"slidecontainer\">\
          <input type=\"range\" min=\"0\" max=\"255\" value=\"0\" class=\"slider\" id=\"analogSlider\" oninput=\"sendAnalogValue(this.value)\">\
          <p>Value: <span id=\"analogValue\">0</span></p>\
          <button onmousedown=\"sendDirection(1)\" onmouseup=\"sendStop()\">UP</button>\
          <button onmousedown=\"sendDirection(2)\" onmouseup=\"sendStop()\">RIGHT</button>\
          <button onmousedown=\"sendDirection(3)\" onmouseup=\"sendStop()\">LEFT</button>\
          <button onmousedown=\"sendDirection(0)\">STOP</button>\
        </div>\
        <script>\
          var webSocket = new WebSocket('ws://' + window.location.hostname + '/CarInput');\
          var slider = document.getElementById('analogSlider');\
          var output = document.getElementById('analogValue');\
          slider.oninput = function() {\
            output.innerHTML = this.value;\
          };\
          function sendDirection(direction) {\
            var voltage = slider.value;\
            webSocket.send('Direction,' + direction + ',' + voltage);\
          }\
          function sendStop() {\
            webSocket.send('Direction,0,0');\
          }\
        </script>\
      </body>\
    </html>");
}



void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                              AwsEventType type,
                              void *arg,
                              uint8_t *data,
                              size_t len) {
  if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode == WS_TEXT) {
      String message = "";
      for (size_t i = 0; i < len; i++) {
        message += (char)data[i];
      }

      if (message.startsWith("Direction,")) {
        int commaIndex1 = message.indexOf(',');
        int commaIndex2 = message.indexOf(',', commaIndex1 + 1);
        int direction = message.substring(commaIndex1 + 1, commaIndex2).toInt();
        int voltageValue = message.substring(commaIndex2 + 1).toInt();
        moveCar(direction, voltageValue);  // Pass both direction and voltageValue to moveCar function
      }
    }
  }
}



void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
  Serial.println("HTTP server started");
  dacWrite(DAC_PIN_1, 0);
    dacWrite(DAC_PIN_2, 0);
}

void loop() {
  // No looping tasks here since the server operates asynchronously
}