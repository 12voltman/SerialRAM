/*
	SerialRAM.cpp
	Very simple class to interface with Microchip's 4K/16K I2C Serial EERAM (47L04, 47C04, 47L16 and 47C16) chips

	This example code is licensed under CC BY 4.0.
	Please see https://creativecommons.org/licenses/by/4.0/

	modified 6th August 2017
	by Tony Pottier
	https://idyl.io

*/

#include <stdint.h>
#include <Wire.h>
#include "SerialRAM.h"


///<summary>
///	Initialize the RAM chip with the given A0 and A1 values.
///	<param name="A0">A0 value (logic 0 or 1) of the RAM chip you want to address. Default value 0.</param>
///	<param name="A1">A1 value (logic 0 or 1) of the RAM chip you want to address. Default value 0.</param>
///	<param name="SIZE">Size of the RAM chip you want to address in kbits (only 4 or 16 valid). Default value 16.</param>
///</summary>
uint8_t SerialRAM::begin(const uint8_t A0, const uint8_t A1, const uint8_t SIZE) {
	//build mask
	uint8_t mask = (A0 << 1) | (A1);
	mask <<= 1;

	//save registers addresses
	this->SRAM_REGISTER = 0x50 | mask;
	this->CONTROL_REGISTER = 0x18 | mask;

	//Arduino I2C lib
	Wire.begin();
	
	//check chip size variable
	if(SIZE == 16){
		this->STORAGE_ARRAY_SIZE = 0xf8;
		return 0;
	}
	else if(SIZE == 4){
		this->STORAGE_ARRAY_SIZE = 0xfe;
		return 0;
	}
	else {
		this->STORAGE_ARRAY_SIZE = 0xf8;
		return 1;
	}
}


///<summary>
///	Write the given byte "value" at the 16 bit address "address".
///		47x16 chips valid addresses range from 0x0000 to 0x07FF
///		47x04 chips valid addresses range from 0x0000 to 0x01FF
///		<param name="address">16 bit address</param>
///		<param name="value">value (byte) to be written</param>
///		<returns>0:success, 1:data too long to fit in transmit buffer, 2 : received NACK on transmit of address, 3 : received NACK on transmit of data, 4 : other error , 5 : address out of bounds</returns>
///</summary>
uint8_t SerialRAM::write(const uint16_t address, const uint8_t value) {
	address16b a;
	a.a16 = address;
	uint8_t arrSize = this->STORAGE_ARRAY_SIZE;
	if(a.a8[1] & arrSize){
		return 5;
	}
	Wire.beginTransmission(this->SRAM_REGISTER);
	Wire.write(a.a8[1]);
	Wire.write(a.a8[0]);
	Wire.write(value);
	return Wire.endTransmission();
}

///<summary>
///	Read the byte "value" located at the 16 bit address "address".
///		47x16 chips valid addresses range from 0x0000 to 0x07FF
///		47x04 chips valid addresses range from 0x0000 to 0x01FF
///		<param name="address">16 bit address</param>
///		<returns>value (byte) read at the address, or 0 if address out of bounds</returns>
///</summary>
uint8_t SerialRAM::read(const uint16_t address) {
	uint8_t buffer;
	address16b a;
	a.a16 = address;
	uint8_t arrSize = this->STORAGE_ARRAY_SIZE;
	if(a.a8[1] & arrSize){
		return 0;
	}
	Wire.beginTransmission(this->SRAM_REGISTER);
	Wire.write(a.a8[1]);
	Wire.write(a.a8[0]);
	Wire.endTransmission();

	Wire.requestFrom(this->SRAM_REGISTER, 1);
	buffer = Wire.read();
	Wire.endTransmission();

	return buffer;
}

uint8_t SerialRAM::readControlRegister() {
	uint8_t buffer = 0x80;

	Wire.beginTransmission(this->CONTROL_REGISTER);
	Wire.write(0x00); //status register
	Wire.endTransmission();

	Wire.requestFrom(this->CONTROL_REGISTER, 1);
	buffer = Wire.read();
	Wire.endTransmission();

	return buffer;
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<param name="value">Set to true to activate, false otherwise</param>
///</summary>
void SerialRAM::setAutoStore(const bool value)
{
	uint8_t buffer = this->readControlRegister();
	buffer = value ? buffer|0x02 : buffer&0xfd;
	Wire.beginTransmission(this->CONTROL_REGISTER);
	Wire.write(0x00); //status register
	Wire.write(buffer);
	Wire.endTransmission();
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<returns>true if auto store is active</return>
///</summary>
bool SerialRAM::getAutoStore()
{
	uint8_t buffer = this->readControlRegister();
	return buffer & 0x02;
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<param name="value">0-7 are valid levels of protection</param>
///</summary>
uint8_t SerialRAM::setWriteProtect(const uint8_t prot)
{
	uint8_t protectArea = prot;
	if(protectArea & 0xf8) {
		return 1;
	}
	uint8_t buffer = this->readControlRegister();
	buffer = (buffer & 0xe3) | (protectArea << 2);
	Wire.beginTransmission(this->CONTROL_REGISTER);
	Wire.write(0x00); //status register
	Wire.write(buffer);
	Wire.endTransmission();
	return 0;
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<returns>level of write protection</return>
///</summary>
uint8_t SerialRAM::getWriteProtect()
{
	uint8_t buffer = this->readControlRegister();
	return (buffer & 0x1c) >> 2;
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<param name="value">Set the state of the Hardware Event bit</param>
///</summary>
void SerialRAM::setEventBit(const bool value)
{
	uint8_t buffer = this->readControlRegister();
	buffer = value ? buffer|0x01 : buffer&0xfe;
	Wire.beginTransmission(this->CONTROL_REGISTER);
	Wire.write(0x00); //status register
	Wire.write(buffer);
	Wire.endTransmission();
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<returns>state of hardware store bit</return>
///</summary>
bool SerialRAM::getEventBit()
{
	uint8_t buffer = this->readControlRegister();
	return buffer & 0x01;
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<returns>true if the content of both arrays matches</return>
///</summary>
bool SerialRAM::getMatch()
{
	uint8_t buffer = this->readControlRegister();
	return !(buffer & 0x80);
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<returns>true if the content of both arrays matches</return>
///</summary>
void SerialRAM::store()
{
	Wire.beginTransmission(this->CONTROL_REGISTER);
	Wire.write(0x55); //control register
	Wire.write(0x33);
	Wire.endTransmission();
}

///<summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when power is lost.
///		<returns>true if the content of both arrays matches</return>
///</summary>
void SerialRAM::recall()
{
	Wire.beginTransmission(this->CONTROL_REGISTER);
	Wire.write(0x55); //control register
	Wire.write(0xdd);
	Wire.endTransmission();
}

///<summary>
///	Write the array of bytes "values" at the 16 bit address "address".
///		47x16 chips valid addresses range from 0x0000 to 0x07FF
///		47x04 chips valid addresses range from 0x0000 to 0x01FF
///		<param name="address">16 bit address</param>
///		<param name="values">values (bytes) to be written</param>
///		<returns>0:success, 1:data too long to fit in transmit buffer, 2 : received NACK on transmit of address, 3 : received NACK on transmit of data, 4 : other error </returns>
///</summary>
uint8_t SerialRAM::write(const uint16_t address, const uint8_t* values, const uint16_t size)
{
	address16b a;
	a.a16 = address;
	uint16_t size1 = size;
	uint16_t arrSize = this->STORAGE_ARRAY_SIZE;
	if(a.a8[1] & this->STORAGE_ARRAY_SIZE){
		return 5;
	}
	//else if((a.a16 + size1) | (arrSize << 8)){
	//	return 5;
	//}
	Wire.beginTransmission(this->SRAM_REGISTER);
	Wire.write(a.a8[1]);
	Wire.write(a.a8[0]);
	Wire.write(values, size);
	return Wire.endTransmission();
}

///<summary>
///	Read "size" number of bytes into "values" array located at the 16 bit address "address".
///		Make sure values is big enough to contain all data or a segfault will occur.
///		<param name="address">16 bit startign address of the data</param>
///		<param name="values">array to be used to store the data</param>
///		<param name="size">number of bytes to retrieve</param>
///</summary>
uint8_t SerialRAM::read(const uint16_t address, uint8_t * values, const uint16_t size)
{
	address16b a;
	a.a16 = address;
	uint16_t size1 = size;
	uint16_t arrSize = this->STORAGE_ARRAY_SIZE;
	if(a.a8[1] & arrSize){
		return 5;
	}
	//else if((a.a16 + size1) | (arrSize << 8)){
	//	return 5;
	//}
	Wire.beginTransmission(this->SRAM_REGISTER);
	Wire.write(a.a8[1]);
	Wire.write(a.a8[0]);
	Wire.endTransmission();

	Wire.requestFrom(this->SRAM_REGISTER, size);
	for (uint16_t i = 0; i < size; i++) {
		values[i] = Wire.read();
	}
	Wire.endTransmission();
}

