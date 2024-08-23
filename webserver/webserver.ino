#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "index.h"
#include <DHT20.h>
#include <Adafruit_NeoPixel.h>
#include "RelayStatus.h"

#define TXD 8
#define RXD 9
#define BAUD_RATE 9600

const char* ssid = "YOUR_WIFI_NAME_HERE";
const char* password = "YOUR_WIFI_PASSWORD_HERE";

AsyncWebServer server(80);

DHT20 dht20;
Adafruit_NeoPixel rgb(4, D5, NEO_GRB + NEO_KHZ800);

void sendModbusCommand(const uint8_t command[], size_t length)
{
  for (size_t i = 0; i < length; i++)
  {
    Serial2.write(command[i]);
  }
}

void handleMessage(String message)
{
  // String message = data->value();

  if (message.startsWith("!RELAY") && message.endsWith("#"))
  {
    int indexStart = message.indexOf('!') + 6;
    int indexEnd = message.indexOf(':');
    String indexStr = message.substring(indexStart, indexEnd);
    int index = indexStr.toInt();

    int statusStart = indexEnd + 1;
    int statusEnd = message.indexOf('#');
    String statusStr = message.substring(statusStart, statusEnd);

    // Debug prints
    Serial.print("Raw message: ");
    Serial.println(message);
    Serial.print("Index string: ");
    Serial.println(indexStr);
    Serial.print("Index: ");
    Serial.println(index);
    Serial.print("Status string: ");
    Serial.println(statusStr);

    // Send the Modbus command for the specific relay
    if (statusStr == "ON" && index < (sizeof(relay_ON) / sizeof(relay_ON[0])))
    {
      sendModbusCommand(relay_ON[index], sizeof(relay_ON[0]));
      Serial.println("Relay " + String(index) + " turned ON");
    }
    else if (statusStr == "OFF" && index < (sizeof(relay_OFF) / sizeof(relay_OFF[0])))
    {
      sendModbusCommand(relay_OFF[index], sizeof(relay_OFF[0]));
      Serial.println("Relay " + String(index) + " turned OFF");
    }
    else
    {
      Serial.println("Invalid command");
    }

    String sendData = String(index) + '-' + statusStr;
    Serial.println("Data sent to MODBUS RTU485: " + sendData);
  }
}

float getTemperature() {
  dht20.read();
  float temp = dht20.getTemperature();
  return temp;
}

float getHumidity() {
  dht20.read();
  float humi = dht20.getHumidity();
  return humi;
}

float getSoilMoisture() {
  int soilMoisture = analogRead(A0);
  return (float)soilMoisture;
}

float getLight() {
  int light = analogRead(A1);
  return (float)light;
}

void onLight() {
  rgb.setPixelColor(0, rgb.Color(255,255,255));
  rgb.setPixelColor(1, rgb.Color(255,255,255)); 
  rgb.setPixelColor(2, rgb.Color(255,255,255)); 
  rgb.setPixelColor(3, rgb.Color(255,255,255));       
  rgb.show();
}

void offLight() {
  rgb.setPixelColor(0, rgb.Color(0,0,0));
  rgb.setPixelColor(1, rgb.Color(0,0,0)); 
  rgb.setPixelColor(2, rgb.Color(0,0,0)); 
  rgb.setPixelColor(3, rgb.Color(0,0,0));       
  rgb.show();
}

void onFan() {
  analogWrite(D7, 100);
}

void offFan() {
  analogWrite(D7, 0);
}

void onRelay() {
  digitalWrite(D3, HIGH);
}

void offRelay() {
  digitalWrite(D3, LOW);
}


void setup() {
  Serial.begin(9600);
  Serial2.begin(BAUD_RATE, SERIAL_8N1, TXD, RXD);
  sendModbusCommand(relay_OFF[0], sizeof(relay_OFF[0]));


  dht20.begin();

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D3, OUTPUT); // relay

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Print the ESP32's IP address
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());

  // Serve the HTML page from the file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");
    Serial.println("GET /");              

    if (request->hasArg("device") && request->hasArg("state")) {
        String device = request->arg("device");
        String state = request->arg("state");

        if (device == "fan") {
            if (state == "on") 
              onFan();
            else if (state == "off")
              offFan();
            Serial.println("Fan state: " + state);
        } else if (device == "light") {
            if (state == "on") {
              onLight();
              digitalWrite(D13, HIGH);
            } else if (state == "off") {
              offLight();
              digitalWrite(D13, LOW);
            }
            Serial.println("Light state: " + state);
        } else if (device == "pump") {
            if (state == "on") 
              onRelay();
            else if (state == "off")
              offRelay();
            Serial.println("Pump state: " + state);
        }
    }

    if (request->hasArg("port") && request->hasArg("state")) {
        String port = request->arg("port");
        String state = request->arg("state");
        state.toUpperCase();
        int portNumber = port.toInt();

        // Control Modbus port logic (replace with actual implementation)
        Serial.print("Modbus Port ");
        Serial.print(portNumber);
        Serial.print(" state: ");
        Serial.println(state);

        String message = "!RELAY";   // Khởi tạo chuỗi message
        message.concat(port);        // Nối port vào message
        message.concat(":");         // Nối ":" vào message
        message.concat(state);       // Nối state vào message
        message.concat("#");

        handleMessage(message);
    }             

    request->send(200, "text/html", webpage);
  });

  // Define a route to get the temperature data
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");
    Serial.println("GET /temperature");                       
    float temperature = getTemperature();
    // Format the temperature with two decimal places
    String temperatureStr = String(temperature, 2);
    request->send(200, "text/plain", temperatureStr);
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");
    Serial.println("GET /humidity");                       
    float humi = getHumidity();
    String humiStr = String(humi, 2);
    request->send(200, "text/plain", humiStr);
  });

  server.on("/soilMoisture", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");
    Serial.println("GET /soilMoisture");                       
    float soilMoisture = getSoilMoisture();
    String soilMoistureStr = String(soilMoisture);
    request->send(200, "text/plain", soilMoistureStr);
  });

  server.on("/light", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");
    Serial.println("GET /light");                       
    float light = getLight();
    String lightStr = String(light, 2);
    request->send(200, "text/plain", lightStr);
  });

  // Start the server
  server.begin();
}

void loop() {
  // Your code here
}
