#include <Arduino.h>
#include <tfa433.h>

TFA433 tfa(D2); //Input pin where 433 receiver is connected.
byte id = 0, channel = 0, humidity = 0;
int temperature = 0;
bool battery = false;

void setup(){
    tfa.start();
}

void loop(){
  if (tfa.isDataAvailable()){
    //Pointer results:
    tfa.getData(*id, *channel, *humidity, *temperature, *battery);
    Serial.printf(F("id: %d, channel: %d, humidity: %d, temperature: %d.%d, battery: %s\n"), id, channel, humidity, temperature/100, temperature%100, (battery?"OK":"NOK"));
    //Struct results:
    tfaResult result = tfa.getData();
    Serial.printf(F("id: %d, channel: %d, humidity: %d, temperature: %d.%d, battery: %s\n"), result.id, result.channel, result.humidity, result.temperature/100, result.temperature%100, (result.battery?"OK":"NOK"));
  }
}
