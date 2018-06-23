/*
	SerialRAM.h
	Very simple class to interface with Microchip's 4K/16K I2C Serial EERAM (47L04, 47C04, 47L16 and 47C16) chips

	This example code is licensed under CC BY 4.0.
	Please see https://creativecommons.org/licenses/by/4.0/

	modified 6th August 2017
	by Tony Pottier
	https://idyl.io

*/

#ifndef _SerialRAM_h
#define _SerialRAM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


typedef union {
	uint16_t a16;
	uint8_t a8[2];
}address16b;

class SerialRAM {
private:
	int8_t SRAM_REGISTER;
	int8_t CONTROL_REGISTER;
	int8_t STORAGE_ARRAY_SIZE;
	uint8_t readControlRegister();

public:
	
	uint8_t begin(const uint8_t A0 = 0, const uint8_t A1 = 0, const uint8_t SIZE = 16);
	uint8_t write(const uint16_t address, const uint8_t value);
	uint8_t read(const uint16_t address);
	void setAutoStore(const bool value);
	bool getAutoStore();
	
	void store();
	void recall();
	
	uint8_t setWriteProtect(const uint8_t prot);
	uint8_t getWriteProtect();
	
	void setEventBit(const bool value);
	bool getEventBit();
	
	bool getMatchStatus();
	
	uint8_t write(const uint16_t address, const uint8_t* values, const uint16_t size);
	uint8_t read(const uint16_t address, uint8_t* values, const uint16_t size);


};




#endif

