#include <SPI.h>
#include <RF24Network.h>
#include <RF24.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

 //Radio constances
RF24 radio(D2, D8); // CE/CSN pins
RF24Network network(radio);
const uint16_t channel = 90;
const uint16_t thisNode = 0; // Address of this node (gateway node)

//Wifi constances
const char* ssid = "Wifi nowa budowa";
const char* password = "123456789a";
const char* host = "http://temperature-broker.herokuapp.com/temperature-broker/";
#define maxBuffor 20
uint8_t counter = 0;
unsigned long lastTimeSend;
const unsigned long sendDelay = 1000 * 90;

struct dataStruct{
  uint16_t measurementTypeId;
  uint16_t stationId;
  char secretId[36];
  float value;
};

dataStruct sensorsData[maxBuffor];
 
void setup() {
  Serial.begin(115200);
  Serial.println(" ");
  Serial.println("Starting");
  
  reconnectToWifi();
  
  restartRadio();

  lastTimeSend = millis();
  
  Serial.println("Listening for sensor values"); 
}
 
void loop() {
  network.update();
  while (network.available()) {
    Serial.println("Received radio connection");
    readRadioData();
    if(shouldSendData()){
      sendAllData();
    }
  }
}

void readRadioData(){
    RF24NetworkHeader header;
    dataStruct sensorData;
    if(network.read(header, &sensorData, sizeof(sensorData))){
      sensorsData[counter] = sensorData;
      counter++;
    }
}

bool shouldSendData(){
  if((counter == maxBuffor - 1 || lastTimeSend + sendDelay < millis()) && counter > 0){
    return true;
  }
  return false;
}

void sendAllData(){
    yield();
    if(WiFi.status() != WL_CONNECTED){
        reconnectToWifi();
    }
    
    String postData = createPostData();

    HTTPClient http;
     
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
    
    lastTimeSend = millis();
    counter = 0;
}

String createPostData(){
  String postData = "[";

    for(uint8_t i = 0; i < counter; i++) {
      dataStruct sensorData = sensorsData[i];
      postData += "{\"value\":" + String(sensorData.value) + ",\"measurementTypeId\":" + String(sensorData.measurementTypeId) + ",\"stationId\":" + String(sensorData.stationId) + ",\"secretId\":\"" + String(sensorData.secretId) + "\"}";
      if(i != counter - 1) {
        postData += ",";
      }
    }

    postData += "]";
    return postData;
}

void reconnectToWifi() {
  Serial.println("Reconnecting to wifi");
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
  yield();
}

void restartRadio(){
  Serial.println("Restarting radio");
  radio.begin();
  network.begin(channel, thisNode);
  network.update();
  Serial.print("Is radio properly connected = ");
  Serial.println(radio.isChipConnected());
}
