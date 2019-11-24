#include <SPI.h>
#include <RF24Network.h>
#include <RF24.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
 
RF24 radio(D2, D8); // CE/CSN pins
RF24Network network(radio);

const uint16_t channel = 90;
const uint16_t thisNode = 0; // Address of this node (the gateway node)

// wifi credentials
const char* ssid = "halorozwoj";
const char* password = "12345678";
 
//Server address to write to
const char* host = "http://192.168.100.6:8080/temperature-broker/";

struct dataStruct{
  uint16_t measurementTypeId;
  uint16_t stationId;
  float value;
}sensorData;
 
 
void setup() {
  Serial.begin(115200);
  connectToWifi();
  yield();
  Serial.println("starting......");
  restartRadio();                    // turn on and configure radio
  bool tmp = radio.isChipConnected();
  Serial.print("Is radio properly connected = ");
  Serial.println(tmp);
  Serial.println("Listening for sensor values..."); 
}
 
void loop() {
  network.update();
  while (network.available()) {
    Serial.println("Received radio connection ");
    yield();
    RF24NetworkHeader header;        
    network.read(header, &sensorData, sizeof(sensorData));

    delay(3000);

    HTTPClient http;
    String postData = "{\"value\":" + String(sensorData.value) + ",\"measurementTypeId\":" + String(sensorData.measurementTypeId) + ",\"stationId\":" + String(sensorData.stationId) + "}";    
    http.begin(host); //Specify request destination
    http.addHeader("Content-Type", "application/json");
    int responseCode = http.POST(postData);
    String response = http.getString();

    Serial.print("Data sent: ");
    Serial.println(postData);
    Serial.print("Response code: ");
    Serial.println(responseCode);
    Serial.print("Response: ");
    Serial.println(response);
   
    http.end();
  }
}

void connectToWifi() {
  WiFi.begin(ssid, password);
  Serial.println(" ");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
}

void restartRadio(){
  yield(); // give wifi functionality some time to do its operations
  radio.begin(); // Start up the radio
  network.begin(channel, thisNode);
  network.update(); // always be pumping the network
}
