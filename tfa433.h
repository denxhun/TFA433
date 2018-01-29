/*
433 MHz wether station receiver library for so called TFA

Pulse should look like this:
     __   0   __     1     __    START/STOP    __
   _|  |_____|  |_________|  |________________|  |_

Based on my measurement pulse lengths are the following:
0: 1500 - 2500 us
1: 3500 - 4500 us
S: 7500 - 9000 us

Pulses shorter than 1500 and longer than 9000 are skipped.
Before the first data bit arrived usually 8-16 "S" pulsey are coming.
Senders are usually repeats the message 3-6 times. Between repeats there are 2 "S" pulses.

*/
#ifndef tfa433_h
#define tfa433_h

#include <Arduino.h>

typedef struct{
  byte id;
  byte channel;
  byte humidity;
  int temperature;
  bool battery;
} tfaResult;

class TFA433{
  public:
    TFA433(int pin);
    void start();
    void stop();
    bool isDataAvailable();
    void getData(byte &id, byte &channel, byte &humidity, int &temperature, bool &battery);
    tfaResult getData();
  private:
    const static byte BUFF_SIZE = 50;

    volatile static bool avail;
    volatile static byte buff[BUFF_SIZE];
    volatile static byte buffEnd;

    static byte lastBuff[BUFF_SIZE];
    byte pin;

    static void handler();
    static bool isRepeat();
    int binToDecRev(volatile byte *binary, int s, int e);
    int binToDec(volatile byte *binary, int s, int e);
};

#endif
