#include <SPI.h>
#include <RF24Network.h>
#include <RF24.h>
#include <RF24_config.h>
#include <printf.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LowPower.h>

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
  Serial.println("Starting");

  requestTemperature();

  restartRadio();

  Serial.println("Starting measurements in 5 sec");
  delay(5000);
}

void loop() {

  requestTemperature();

  Serial.print("Sending temperature: ");
  Serial.print(data.value); 
  Serial.println("C"); 

  radio.powerUp();

  RF24NetworkHeader header(gatewayNode);
  bool ok = network.write(header, &data, sizeof(data));
  if (ok) {
    Serial.println("Data deliviered succesfully");
  } else {
    Serial.println("Failed to send data");
  }

  radio.powerDown();

  Serial.println("Sleeping for 64 sec");
  Serial.flush();
  for (int i = 0; i < 8; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  
}

void requestTemperature(){
  sensors.begin();
  sensors.requestTemperatures();
  data.value = sensors.getTempCByIndex(0);
}

void restartRadio() {
  Serial.println("Restarting radio");
  radio.begin();
  network.begin(mChannel, thisNode);
  network.update();
  Serial.print("Is radio properly connected = ");
  Serial.println(radio.isChipConnected());
}
