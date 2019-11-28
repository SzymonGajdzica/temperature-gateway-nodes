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

const char* ssid = "Wifi nowa budowa";
const char* password = "123456789a";
 
const char* host = "http://temperature-broker.herokuapp.com/temperature-broker/";

struct dataStruct{
  uint16_t measurementTypeId;
  uint16_t stationId;
  char secretId[36];
  float value;
}sensorData;
 
 
void setup() {
  Serial.begin(115200);
  Serial.println("starting......");
  
  connectToWifi();
  
  restartRadio();
  
  Serial.println("Listening for sensor values..."); 
}
 
void loop() {
  network.update();
  while (network.available()) {
    Serial.println("Received radio connection ");
    yield();
    RF24NetworkHeader header;        
    network.read(header, &sensorData, sizeof(sensorData));

    yield();

    HTTPClient http;
    String postData = "{\"value\":" + String(sensorData.value) + ",\"measurementTypeId\":" + String(sensorData.measurementTypeId) + ",\"stationId\":" + String(sensorData.stationId) + ",\"secretId\":\"" + String(sensorData.secretId) + "\"}";    
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
  Serial.print(WiFi.localIP());  
  yield();
}

void restartRadio(){
  Serial.println("Restarting radio...");
  yield(); // give wifi functionality some time to do its operations
  radio.begin(); // Start up the radio
  network.begin(channel, thisNode);
  network.update(); // always be pumping the network
  Serial.print("Is radio properly connected = ");
  Serial.println(radio.isChipConnected());
}
