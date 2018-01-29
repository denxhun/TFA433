#include "tfa433.h"

#define FILTER_MIN 1500
#define FILTER_MAX 9000
#define MEDIUM_LEN 3000
#define STOP_MIN 7500

TFA433::TFA433(int pin) {
  this->pin = pin;
  pinMode(pin, INPUT);
}

void TFA433::start() {
  for (int i = 0; i < BUFF_SIZE; i++) {
    buff[i] = 8;
    lastBuff[i] = 8;
  }
  avail = false;
  buffEnd=0;
  attachInterrupt(digitalPinToInterrupt(pin), handler, CHANGE);
}

void TFA433::stop(){
  detachInterrupt(digitalPinToInterrupt(pin));
}

bool TFA433::isDataAvailable(){
  return avail;
}

void TFA433::handler() {
  static unsigned long lastMs = 0, currMs,diffMs;
  currMs = micros();
  diffMs = currMs - lastMs;
  lastMs = currMs;

  if (diffMs > FILTER_MIN && diffMs < FILTER_MAX) { //Filter out the too long and too short pulses
    if (!avail) { //avail means data available for processing
      if (diffMs > STOP_MIN) { // INIT/STOP pulse
        // dbg("dS");
        if (buffEnd == 42) { //There is the right amount of data in buff
          if (isRepeat()){ //if this is the repeat of the previous package then don't respond a false positive availability.
            avail = true;
          }
        } else {
          buffEnd = 0;
        }
      } else {
        if (buffEnd < BUFF_SIZE) {  //buffer is not full yet
          if (diffMs < MEDIUM_LEN) { //0
            buff[buffEnd++] = 0;
            // dbg("d0");
          } else { //1
            buff[buffEnd++] = 1;
            // dbg("d1");
          }
        }
      }
    }
  }
}

bool TFA433::isRepeat() {
  for(int i=0;i<BUFF_SIZE;i++){
    if (buff[i]!=lastBuff[i]){
      for (int j=0;j<buffEnd;j++){
        lastBuff[j] = buff[j];
      }
      return false;
    }
  }
  return true;
}

void TFA433::getData(byte &id, byte &channel, byte &humidity, int &temperature, bool &battery){
  int crc = 0;
  int temp1 = 0, temp2 = 0, temp3 = 0;
  int humi1 = 0, humi2 = 0;

  for(int i=0;i<34;i++) {
    if(buff[i] != (crc&1)) {
      crc = (crc>>1) ^ 12;
    } else {
      crc = (crc>>1);
    }
  }
  crc ^= binToDec(buff, 34, 37);
  if (crc != binToDec(buff, 38, 41)) {
    //Serial.println("CRC error!");
    return;
  }
  //dbg("CRC OK.");
  id = binToDecRev(buff, 2, 9);
  channel = binToDecRev(buff, 12, 13) + 1;
  temp1 = binToDecRev(buff, 14, 17);
  temp2 = binToDecRev(buff, 18, 21);
  temp3 = binToDecRev(buff, 22, 25);

  // Convert from °F to °C,  a zero value is equivalent to -90.00 °F with an exp of 10, we enlarge that to 2 digit
  temperature = (((((temp1 + temp2*16 + temp3*256) * 10) - 9000 - 3200) * 5) / 9);

  humi1 = binToDecRev(buff, 26, 29);
  humi2 = binToDecRev(buff, 30, 33);
  humidity = humi1 + humi2*16;

  if(binToDecRev(buff, 35, 35) == 1) {
    battery = false;
  } else {
    battery = true;
  }
//  Serial.print("\nid:" + String(id) + " channel:" + String(channel));
//  Serial.print("  Temperature:");
//  Serial.print(temperature/100, 2);
//  Serial.print("°C Humidity:" + String(humidity) + "% battery:" );
//  if (battery==1)
//    Serial.println("OK");
//  else
//    Serial.println("NOK");

}

tfaResult TFA433::getData(){
  tfaResult result;
  getData(result.id, result.channel, result.humidity, result.temperature, result.battery);
  return result;
}

int TFA433::binToDecRev(volatile byte *binary, int s, int e) {
  int result = 0;
  unsigned int mask = 1;
  for(; e > 0 && s<=e; mask <<= 1)
    if(binary[e--] != 0)
      result |= mask;
  return result;
}

int TFA433::binToDec(volatile byte *binary, int s, int e) {
  unsigned int mask = 1;
  int result = 0;
  for(; s<=e; mask <<= 1)
    if(binary[s++] != 0)
      result |= mask;
  return result;
}
