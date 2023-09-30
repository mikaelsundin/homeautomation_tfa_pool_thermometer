#include "tfa433.h"

#define BIT_COUNT  36
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
				if (_buffEnd == BIT_COUNT) { //There is the right amount of data in buff
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

void TFA433::getData(uint16_t &id, uint8_t &channel, int16_t &temperature){
	int16_t temp;

    //Extract data
    id = (uint16_t)_binToDecRev(_buff, 0, 7); //Fixed ID or Preamble
    
	channel = _binToDecRev(_buff, 14, 15) + 1;
    temp = _binToDecRev(_buff, 16, 27);
    
    //Sign extend to 16bit if required.
    if(temp & 0x0800){
        temp -= 0x1000;
    }

    //Print packet.
    //for(int i=0;i<36;i++){
    //    Serial.print(_buff[i]);
    //}
    //Serial.println();

    //Set temperature
    temperature = temp;
    
    //Mark as not avaiable
	_avail = false;
}

tfaResult TFA433::getData(){
	tfaResult result;
	getData(result.id, result.channel, result.temperature);
 
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
