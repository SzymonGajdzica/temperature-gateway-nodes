#include <SPI.h>
#include <RF24Network.h>
#include <RF24.h>
#include <RF24_config.h>
#include <printf.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(6); // dallas pin
DallasTemperature sensors(&oneWire);

RF24 radio(9, 10); // CE/CSN pins
RF24Network network(radio); 

const uint16_t channel = 90;
const uint16_t thisNode = 1;
const uint16_t gatewayNode = 0;

struct dataStruct{
  uint16_t measurementTypeId;
  uint16_t stationId;
  float value;
}data;

void setup() {
  Serial.begin(115200);
  Serial.println("starting...");

  sensors.begin();
  restartRadio();
  
  data.value = 0.0;
  data.stationId = 46;
  data.measurementTypeId = 32;
  
  char sensorInfo[100];
  sprintf(sensorInfo, "stationId = %i & measurementTypeId = %i", data.stationId, data.measurementTypeId);
  Serial.print("Sensor config: ");
  Serial.println(sensorInfo);
  Serial.println("Starting measuring");

}

void loop() {

  delay(10000);

  sensors.requestTemperatures();
  data.value = sensors.getTempCByIndex(0);
  
  Serial.print("Sending temperature: ");
  Serial.print(data.value); 
  Serial.println("C"); 

  RF24NetworkHeader header(gatewayNode);
  bool ok = network.write(header, &data, sizeof(data));
  if (ok) {
    Serial.println("Data deliviered succesfully");
  } else {
    Serial.println("Failed to send data");
  }
  
}

void restartRadio() {
  radio.begin(); // Start up the radio
  network.begin(channel, thisNode);
  network.update(); // always be pumping the network
  bool tmp = radio.isChipConnected();
  Serial.print("Is radio properly connected = ");
  Serial.println(tmp);
}
