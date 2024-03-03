#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <driver/dac.h>

#define DAC_PIN_1 25 // for LEFT Motor
#define DAC_PIN_2 26 // for RIGHT Motor



const char* ssid = "MyWiFiCar";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

//The rotateMotor function should be removed and reworked as it's no longer needed


void rotateMotor(int voltageValue) {
  Serial.print("Setting voltage to: ");
  Serial.println(voltageValue);
  dacWrite(DAC_PIN_1, voltageValue);
}

//TODO: Buttons shaped as UP, LEFT and RIGHT arrows 
/*Direction buttons should have event listeners for the mousedown 
and mouseup events on the button element. W
hen the button is pressed (mousedown event), the performAction() 
function is called. When the button is released (mouseup event), the 
isButtonPressed flag is set to false. 
As long as the button is held down, the action will continue to be performed.
*/

/*The directional buttons should call upon their respective move function i.e UP 
calls the up function and parses the voltage/speed value */

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
        </div>\
        <script>\
          var webSocket = new WebSocket('ws://' + window.location.hostname + '/CarInput');\
          var slider = document.getElementById('analogSlider');\
          var output = document.getElementById('analogValue');\
          slider.oninput = function() {\
            output.innerHTML = this.value;\
            webSocket.send('Voltage,' + this.value);\
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
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->opcode == WS_TEXT) {
      String message = "";
      for (size_t i = 0; i < len; i++) {
        message += (char)data[i];
      }
      

      if (message.startsWith("Voltage,")) {
        int voltageValue = message.substring(8).toInt();
        
       
        rotateMotor(voltageValue);
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
}

void loop() {
  // No looping tasks here since the server operates asynchronously
}