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

const uint16_t mChannel = 90;
const uint16_t thisNode = 1;
const uint16_t gatewayNode = 0;

struct dataStruct{
  uint16_t measurementTypeId = 4;
  uint16_t stationId = 3;
  char secretId[36] = "a0e1ef6f-affd-40f8-906c-604b1afa3529";
  float value;
}data;

void setup() {
  Serial.begin(115200);

  Serial.println("Starting...");

  sensors.begin();
  restartRadio();

  Serial.println("Starting measuring");
}

void loop() {

  delay(60000);

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
  Serial.println("Restarting radio...");
  radio.begin(); // Start up the radio
  network.begin(mChannel, thisNode);
  network.update(); // always be pumping the network
  Serial.print("Is radio properly connected = ");
  Serial.println(radio.isChipConnected());
}
