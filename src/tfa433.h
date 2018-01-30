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

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

typedef struct{
	byte id;
	byte channel;
	byte humidity;
	int temperature;
	bool battery;
} tfaResult;

class TFA433{
	public:
		TFA433();
		void start(int pin);
		void stop();
		bool isDataAvailable();
		void getData(byte &id, byte &channel, byte &humidity, int &temperature, bool &battery);
		tfaResult getData();
		
	private:
		const static int _BUFF_SIZE;

		volatile static bool _avail;
		volatile static byte _buff[];
		volatile static byte _buffEnd;

		static unsigned long _lastPackageArrived;
		static byte _lastBuff[];
		static byte _pin;

		static void _handler();
		static bool _isRepeat();
		int _binToDecRev(volatile byte *binary, int s, int e);
		int _binToDec(volatile byte *binary, int s, int e);
};

#endif
