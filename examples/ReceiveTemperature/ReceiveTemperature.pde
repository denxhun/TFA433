#include <tfa433.h>

TFA433 tfa = TFA433(); //Input pin where 433 receiver is connected.

void setup() {
  Serial.begin(115200);
  tfa.start(2);
}

void loop() {
  if (tfa.isDataAvailable()) {
	char txt[100];
//Pointer results:
//    byte id = 0, channel = 0, humidity = 0;
//    int temperature = 0;
//    bool battery = false;
//    tfa.getData(id, channel, humidity, temperature, battery);
//    sprintf(txt, "id: %d, channel: %d, humidity: %d%%, temperature: %d.%d C, battery: %s\n", id, channel, humidity, temperature / 100, temperature % 100, (battery ? "OK" : "NOK"));
//    Serial.print(txt);
//Struct results:
	tfaResult result = tfa.getData();
	sprintf(txt, "id: %d, channel: %d, humidity: %d%%, temperature: %d.%d C, battery: %s\n", result.id, result.channel, result.humidity, result.temperature / 100, result.temperature % 100, (result.battery ? "OK" : "NOK"));
	Serial.print(txt);
  }
}