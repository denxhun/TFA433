#include "tfa433.h"

#define FILTER_MIN 1500
#define FILTER_MAX 9000
#define MEDIUM_LEN 3000
#define STOP_MIN 7500

//#define dbg(s) Serial.println(s)
#define dbg(s)

const int TFA433::_BUFF_SIZE = 50;

volatile bool TFA433::_avail = false;
volatile byte TFA433::_buff[TFA433::_BUFF_SIZE];
volatile byte TFA433::_buffEnd = 0;

unsigned long TFA433::_lastPackageArrived;
byte TFA433::_lastBuff[TFA433::_BUFF_SIZE];
byte TFA433::_pin = 0;

TFA433::TFA433() {
}

void TFA433::start(int pin) {
	_pin = pin;
	pinMode(_pin, INPUT);
	for (int i = 0; i < _BUFF_SIZE; i++) {
		_buff[i] = 8;
		_lastBuff[i] = 8;
	}
	_lastPackageArrived = 0;
	_avail = false;
	_buffEnd=0;
	attachInterrupt(digitalPinToInterrupt(_pin), _handler, CHANGE);
	dbg("tfa started");
}

void TFA433::stop(){
	detachInterrupt(digitalPinToInterrupt(_pin));
}

bool TFA433::isDataAvailable(){
	return _avail;
}

void TFA433::_handler() {
	static unsigned long lastMs = 0, currMs,diffMs;
	currMs = micros();
	diffMs = currMs - lastMs;
	lastMs = currMs;

	if (diffMs > FILTER_MIN && diffMs < FILTER_MAX) { //Filter out the too long and too short pulses
		if (!_avail) { //avail means data available for processing
			if (diffMs > STOP_MIN) { // INIT/STOP pulse
				dbg("S");
				if (_buffEnd == 42) { //There is the right amount of data in buff
					if (!_isRepeat()){ //if this is the repeat of the previous package ( in 3 sec) then don't respond a false positive availability.
						_avail = true;
					} else {
						_buffEnd = 0;
					}
				} else {
					dbg("S buffEnd:" + String(_buffEnd));
					_buffEnd = 0;
				}
			} else {
				if (_buffEnd < _BUFF_SIZE) {  //buffer is not full yet
					if (diffMs < MEDIUM_LEN) { //0
						_buff[_buffEnd++] = 0;
						dbg("0");
					} else { //1
						_buff[_buffEnd++] = 1;
						dbg("1");
					}
				}
			}
		}
	}
}

bool TFA433::_isRepeat() {
	bool result = false;
	for(int i=0;i<_buffEnd;i++){
		if (_buff[i]!=_lastBuff[i]){
			for (int j=0;j<_buffEnd;j++){
				_lastBuff[j] = _buff[j];
			}
			_lastPackageArrived = millis();
			return false;
		}
	}
	result = (millis()-_lastPackageArrived < 3000);
	_lastPackageArrived = millis();
	return result;
}

void TFA433::getData(byte &id, byte &channel, byte &humidity, int &temperature, bool &battery){
	int crc = 0;
	int temp1 = 0, temp2 = 0, temp3 = 0;
	int humi1 = 0, humi2 = 0;

	for(int i=0;i<34;i++) {
		if(_buff[i] != (crc&1)) {
			crc = (crc>>1) ^ 12;
		} else {
			crc = (crc>>1);
		}
	}
	crc ^= _binToDec(_buff, 34, 37);
	if (crc != _binToDec(_buff, 38, 41)) {
		dbg("CRC error!");
		_buffEnd = 0;
		_avail = false;
		return;
	}
	dbg("CRC OK.");
	id = _binToDecRev(_buff, 2, 9);
	channel = _binToDecRev(_buff, 12, 13) + 1;
	temp1 = _binToDecRev(_buff, 14, 17);
	temp2 = _binToDecRev(_buff, 18, 21);
	temp3 = _binToDecRev(_buff, 22, 25);

	// Convert from F to C,  a zero value is equivalent to -90.00 F with an exp of 10, we enlarge that to 2 digit
	temperature = (((((temp1 + temp2*16 + temp3*256) * 10) - 9000 - 3200) * 5) / 9);

	humi1 = _binToDecRev(_buff, 26, 29);
	humi2 = _binToDecRev(_buff, 30, 33);
	humidity = humi1 + humi2*16;

	battery = _binToDecRev(_buff, 35, 35) != 1;
	_avail = false;
}

tfaResult TFA433::getData(){
	tfaResult result;
	getData(result.id, result.channel, result.humidity, result.temperature, result.battery);
	return result;
}

int TFA433::_binToDecRev(volatile byte *binary, int s, int e) {
	int result = 0;
	unsigned int mask = 1;
	for(; e > 0 && s<=e; mask <<= 1)
	if(binary[e--] != 0)
		result |= mask;
	return result;
}

int TFA433::_binToDec(volatile byte *binary, int s, int e) {
	unsigned int mask = 1;
	int result = 0;
	for(; s<=e; mask <<= 1)
	if(binary[s++] != 0)
		result |= mask;
	return result;
}
