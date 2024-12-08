#define blynkTemplateId "TMPL6Br9XbE2_"
#define blynkTemplateName "sau"

#include <ESP8266WiFi.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WebServer.h>  // For Web Server

const char blynkAuth[] = "khn2mK6HNos9yQKCkyDT1pBPesFCYJd1";  // Blynk Auth Token

WiFiManager wifiManager;
ESP8266WebServer webServer(80);  // Web server on port 80

String incomingDataStr = "";
String serialLog = "";  // To store serial log
bool debugServer = false;
bool controlMode = 0;
char incomingData[64];  // Array to hold incoming data
float tempRange = 0;
float humiRange = 0;
float luxRange = 0;
uint8_t mosfetSpeed = 100;

int relayState1 = 0;
int relayState2 = 0;
int relayState3 = 0;
int relayState4 = 0;
int relayState5 = 0;

void readSerial();
void handleRoot();  // Web server root handler
void sendToArduino();
byte calculateChecksum(byte* data, size_t length);

void setup() {
  Serial.begin(9600);
  Serial.println("ESP8266 Blynk Connection");

  // Set up WiFiManager for automatic connection
  wifiManager.autoConnect("ESP8266_Blynk");
  Serial.println("WiFi connected.");

  // Initialize Blynk
  Blynk.begin(blynkAuth, WiFi.SSID().c_str(), WiFi.psk().c_str());

  // Start the Web Server
  if (debugServer) {
    webServer.on("/", handleRoot);  // Define root endpoint
    webServer.begin();
  }
}

void loop() {
  Blynk.run();               // Keep Blynk active
  webServer.handleClient();  // Handle web server requests
  readSerial();
}

void readSerial() {
  if (Serial.available() > 0) {
    int len = Serial.readBytesUntil('\n', incomingData, sizeof(incomingData));
    incomingData[len] = '\0';  // Null terminate the string

    // Check if the incoming data starts with "s,"
    if (strstr(incomingData, "s,") != NULL) {
      float temperature = 0;
      float humidity = 0;
      float lux = 0;

      // Parse the string data: "s,25.5,60.2,123.4,relay1State,relay2State,...,mosfetSpeed"
      int result = sscanf(incomingData, "s,%f,%f,%f",
                          &temperature, &humidity, &lux);
      if (result == 3) {  // Successfully parsed all expected values
        // Send the data to Blynk (Virtual Pins)
        Blynk.virtualWrite(V0, temperature);  // Send temperature to V0
        Blynk.virtualWrite(V1, humidity);     // Send humidity to V1
        Blynk.virtualWrite(V2, lux);          // Send lux value to V2
      }
    } else if (strstr(incomingData, "d,") != NULL) {
      int relay1 = 0, relay2 = 0, relay3 = 0, relay4 = 0;

      int result = sscanf(incomingData, "d,%d,%d,%d,%d,%d",
                          &relay1, &relay2, &relay3, &relay4, &mosfetSpeed);

      if (result == 5) {                      // Successfully parsed all expected values
        Blynk.virtualWrite(V6, relay1);       // Send Relay 1 state to V6
        Blynk.virtualWrite(V7, relay2);       // Send Relay 2 state to V7
        Blynk.virtualWrite(V8, relay3);       // Send Relay 3 state to V8
        Blynk.virtualWrite(V9, relay4);       // Send Relay 4 state to V9
        Blynk.virtualWrite(V5, mosfetSpeed);  // Send Mosfet speed to V5
      }
    }
  }
}

byte calculateChecksum(byte* data, size_t length) {
  byte checksum = 0;
  for (size_t i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// Web server root handler
void handleRoot() {
  String html = "<html><head><title>ESP8266 Serial Log</title></head><body>";
  html += "<h1>ESP8266 Serial Log</h1>";
  html += "<div style='white-space: pre-wrap; font-family: monospace;'>" + serialLog + "</div>";
  html += "</body></html>";
  serialLog = "";
  webServer.send(200, "text/html", html);
}

void sendToArduino() {
  String message = "g," + String(tempRange) + "," + String(humiRange) + "," + String(luxRange) + String(controlMode) + "," + String(mosfetSpeed);
  message += "," + String(relayState1) + "," + String(relayState2) + "," + String(relayState3) + "," + String(relayState4);

  Serial.println(message);  // Send the formatted message to Arduino over serial
}

BLYNK_WRITE(V5) {
  mosfetSpeed = param.asInt();
  sendToArduino();
}

BLYNK_WRITE(V4) {
  controlMode = !!param.asInt();
  sendToArduino();
}

BLYNK_WRITE(V3) {
  tempRange = param.asFloat();
  sendToArduino();
}

BLYNK_WRITE(V10) {
  humiRange = param.asFloat();
  sendToArduino();
}

BLYNK_WRITE(V11) {
  luxRange = param.asFloat();
  sendToArduino();
}

BLYNK_WRITE(V6) {
  relayState1 = param.asInt();
  sendToArduino();
}

BLYNK_WRITE(V7) {
  relayState2 = param.asInt();
  sendToArduino();
}

BLYNK_WRITE(V8) {
  relayState3 = param.asInt();
  sendToArduino();
}

BLYNK_WRITE(V9) {
  relayState4 = param.asInt();
  sendToArduino();
}
