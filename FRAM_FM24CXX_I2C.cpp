/**************************************************************************/
/*!
    @file     FRAM_FM24CXX_I2C.cpp
    @author   SOSAndroid (E. Ha.)
    @license  BSD (see license.txt)

    Driver for the MB85RC I2C FRAM from Fujitsu.

    @section  HISTORY

    v1.0 - First release
	v1.0.1 - Robustness enhancement
	v1.0.2 - fix constructor, introducing byte move in memory
	v1.0.3 - fix writeLong() function
	v1.0.4 - fix constructor call error
	v1.0.5 - Enlarge density chip support by making check more flexible, Error codes not anymore hardcoded, add connect example, add Cypress FM24 & CY15B series comment.
	v1.1.0b - adding support for devices without device IDs + 4K & 16 K devices support
	v1.1.0b1 - Fixing checkDevice() + end of range memory map check
	v1.2.0 - Uses reinterpret_cast instead of bit shift / masking for performance. Breaks backward compatibility with previous code - See PR#6
*/
/**************************************************************************/

#include <stdlib.h>
#include <Wire.h>
#include "FRAM_FM24CXX_I2C.h"

/*========================================================================*/
/*                            CONSTRUCTORS                                */
/*========================================================================*/

/**************************************************************************/
/*!
    Constructor
*/
/**************************************************************************/
FRAM_FM24CXX_I2C::FRAM_FM24CXX_I2C(uint8_t i2c_addr, boolean wp, int pin, uint16_t chipDensity) 
{		i2c_addr = i2c_addr;
		wpPin = pin;
		byte result = FRAM_FM24CXX_I2C::initWP(wp);
}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

void FRAM_FM24CXX_I2C::begin(void) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
		if (!Serial) Serial.begin(9600);
		if (Serial){
			Serial.println("FRAM_FM24CXX_I2C object created");
			Serial.print("I2C device address 0x");
			Serial.println(i2c_addr, HEX);
			Serial.print("WP pin number ");
			Serial.println(wpPin, DEC);
			Serial.print("Write protect management: ");
			if(MANAGE_WP) {
				Serial.println("true");
			}
			else {
				Serial.println("false");
			}
			Serial.println("...... ...... ......");
		}
    #endif

	return;
}




/**************************************************************************/
/*!
    @brief  Writes an array of bytes from a specific address
    
    @params[in] i2cAddr
                The I2C address of the FRAM memory chip (1010+A2+A1+A0)
    @params[in] framAddr
                The 16-bit address to write to in FRAM memory
    @params[in] items
                The number of items to write from the array
	@params[in] values[]
                The array of bytes to write
	@returns
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::writeArray (uint16_t framAddr, byte items, uint8_t values[])
{
	if ((framAddr >= MAXADDRESS_04) || ((framAddr + (uint16_t) items - 1) >= MAXADDRESS_04)) return ERROR_11;
	//FRAM_FM24CXX_I2C::I2CAddressAdapt(framAddr);
	#if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
		Serial.print("Calculated address 0x");
		Serial.println((i2c_addr | ((framAddr >> 8) & 0x1)), HEX);
	#endif
	Wire.beginTransmission(i2c_addr | ((framAddr >> 8) & 0x1));
	Wire.write(framAddr & 0xFF);
	
	
	for (byte i=0; i < items ; i++) {
		Wire.write(values[i]);
	}
	return Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Writes a single byte to a specific address
    
    @params[in] i2cAddr
                The I2C address of the FRAM memory chip (1010+A2+A1+A0)
    @params[in] framAddr
                The 16-bit address to write to in FRAM memory
	@params[in] value
                One byte to write
	@returns
				return code of Wire.endTransmission()
*/
/**************************************************************************/

byte FRAM_FM24CXX_I2C::writeByte (uint16_t framAddr, uint8_t value)
{
	uint8_t buffer[] = {value}; 
	return FRAM_FM24CXX_I2C::writeArray(framAddr, 1, buffer);
}



/**************************************************************************/
/*!
    @brief  Reads an array of bytes from the specified FRAM address

    @params[in] i2cAddr
                The I2C address of the FRAM memory chip (1010+A2+A1+A0)
    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
	@params[in] items
				number of items to read from memory chip
	@params[out] values[]
				array to be filled in by the memory read
    @returns    
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::readArray (uint16_t framAddr, byte items, uint8_t values[])
{
	if ((framAddr >= MAXADDRESS_04) || ((framAddr + (uint16_t) items - 1) >= MAXADDRESS_04)) return ERROR_11;
	byte result;
	if (items == 0) {
		result = ERROR_8; //number of bytes asked to read null
	}
	else {
		
		#if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
		Serial.print("Calculated address 0x");
		Serial.println((i2c_addr | ((framAddr >> 8) & 0x1)), HEX);
		#endif
		Wire.beginTransmission(i2c_addr | ((framAddr >> 8) & 0x1));
		Wire.write(framAddr & 0xFF);
		
		
		//FRAM_FM24CXX_I2C::I2CAddressAdapt(framAddr);
		result = Wire.endTransmission();
		
		Wire.requestFrom(i2c_addr, (uint8_t)items);
		for (byte i=0; i < items; i++) {
			values[i] = Wire.read();
		}
	}
	return result;
}

/**************************************************************************/
/*!
    @brief  Reads one byte from the specified FRAM address

    @params[in] i2cAddr
                The I2C address of the FRAM memory chip (1010+A2+A1+A0)
    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
	@params[out] *values
				data read from memory
    @returns    
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::readByte (uint16_t framAddr, uint8_t *value) 
{
	uint8_t buffer[1];
	byte result = FRAM_FM24CXX_I2C::readArray(framAddr, 1, buffer);
	*value = buffer[0];
	return result;
}
/**************************************************************************/
/*!
    @brief  Copy a byte from one address to another in the memory scope

    @params[in] i2cAddr
                The I2C address of the FRAM memory chip (1010+A2+A1+A0)
    @params[in] origAddr
                The 16-bit address to read from in FRAM memory
	@params[in] destAddr
				The 16-bit address to write in FRAM memory
    @returns    
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::copyByte (uint16_t origAddr, uint16_t destAddr) 
{
	uint8_t buffer[1];
	byte result = FRAM_FM24CXX_I2C::readByte(origAddr, buffer);
	result = FRAM_FM24CXX_I2C::writeByte(destAddr, buffer[0]);
	return result;
}


/**************************************************************************/
/*!
    @brief  Reads one bit from the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
    @params[in] bitNb
                The bit position to read
	@params[out] *bit
				value of the bit: 0 | 1
    @returns    
				return code of Wire.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::readBit(uint16_t framAddr, uint8_t bitNb, byte *bit)
{
	byte result;
	if (bitNb > 7) {
		result = ERROR_9;
	}
	else {
		uint8_t buffer[1];
		result = FRAM_FM24CXX_I2C::readArray(framAddr, 1, buffer);
		*bit = bitRead(buffer[0], bitNb);
	}
	return result;
}

/**************************************************************************/
/*!
    @brief  Set one bit to the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
    @params[in] bitNb
                The bit position to set
    @returns    
				return code of Wire.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::setOneBit(uint16_t framAddr, uint8_t bitNb)
{
	byte result;
	if (bitNb > 7)  {
		result = ERROR_9;
	}
	else {
		uint8_t buffer[1];
		result = FRAM_FM24CXX_I2C::readArray(framAddr, 1, buffer);
		bitSet(buffer[0], bitNb);
		result = FRAM_FM24CXX_I2C::writeArray(framAddr, 1, buffer);
	}
	return result;
}
/**************************************************************************/
/*!
    @brief  Clear one bit to the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
    @params[in] bitNb
                The bit position to clear
    @returns    
				return code of Wire.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::clearOneBit(uint16_t framAddr, uint8_t bitNb)
{
	byte result;
	if (bitNb > 7) {
		result = ERROR_9;
	}
	else {
		uint8_t buffer[1];
		result = FRAM_FM24CXX_I2C::readArray(framAddr, 1, buffer);
		bitClear(buffer[0], bitNb);
		result = FRAM_FM24CXX_I2C::writeArray(framAddr, 1, buffer);
	}
	return result;
}
/**************************************************************************/
/*!
    @brief  Toggle one bit to the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
    @params[in] bitNb
                The bit position to toggle
    @returns    
				return code of Wire.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::toggleBit(uint16_t framAddr, uint8_t bitNb)
{
	byte result;
	if (bitNb > 7) {
		result = ERROR_9;
	}
	else {
		uint8_t buffer[1];
		result = FRAM_FM24CXX_I2C::readArray(framAddr, 1, buffer);
		
		if ( (buffer[0] & (1 << bitNb)) == (1 << bitNb) )
		{
			bitClear(buffer[0], bitNb);
		}
		else {
			bitSet(buffer[0], bitNb);
		}
		result = FRAM_FM24CXX_I2C::writeArray(framAddr, 1, buffer);
	}
	return result;
}
/**************************************************************************/
/*!
    @brief  Reads a 16bits value from the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
	@params[out] value
				16bits word
    @returns    
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::readWord(uint16_t framAddr, uint16_t *value)
{
	uint8_t buffer[2];
	byte result = FRAM_FM24CXX_I2C::readArray(framAddr, 2, buffer);
	*value = *reinterpret_cast<uint16_t *>(buffer);
	return result;
}

/**************************************************************************/
/*!
    @brief  Write a 16bits value from the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
	@params[in] value
				16bits word
    @returns    
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::writeWord(uint16_t framAddr, uint16_t value)
{
	uint8_t *buffer = reinterpret_cast<uint8_t *>(&value);
	return FRAM_FM24CXX_I2C::writeArray(framAddr, 2, buffer);
}
/**************************************************************************/
/*!
    @brief  Read a 32bits value from the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from FRAM memory
	@params[in] value
				32bits word
    @returns    
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::readLong(uint16_t framAddr, uint32_t *value)
{
	uint8_t buffer[4];
	byte result = FRAM_FM24CXX_I2C::readArray(framAddr, 4, buffer);
	*value = *reinterpret_cast<uint32_t *>(buffer);
	return result;

}
/**************************************************************************/
/*!
    @brief  Write a 32bits value to the specified FRAM address

    @params[in] framAddr
                The 16-bit address to write to FRAM memory
	@params[in] value
				32bits word
    @returns    
				return code of Wire.endTransmission()
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::writeLong(uint16_t framAddr, uint32_t value)
{
	uint8_t *buffer = reinterpret_cast<uint8_t *>(&value);
	return FRAM_FM24CXX_I2C::writeArray(framAddr, 4, buffer);
}


/**************************************************************************/
/*!
    @brief  Return the readiness of the memory chip

    @params[in]  none
	@returns
				  boolean
				  true : ready
				  false : not ready
*/
/**************************************************************************/



/**************************************************************************/
/*!
    @brief  Return tu Write Protect status

    @params[in]  none
	@returns
				  boolean
				  true : write protect enabled
				  false: wirte protect disabled
*/
/**************************************************************************/
boolean FRAM_FM24CXX_I2C::getWPStatus(void) {
	return wpStatus;
}

/**************************************************************************/
/*!
    @brief  Enable write protect function of the chip by pulling up WP pin

    @params[in]   MANAGE_WP
                  WP management switch defined in header file
    @params[in]   wpPin
                  pin number for WP pin
	@param[out]	  wpStatus
	@returns
				  0: success
				  1: error, WP not managed
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::enableWP(void) {
	byte result;
	if (MANAGE_WP) {
		digitalWrite(wpPin,HIGH);
		wpStatus = true;
		result = ERROR_0;
	}
	else {
		result = ERROR_10;
	}
	return result;
}
/**************************************************************************/
/*!
    @brief  Disable write protect function of the chip by pulling up WP pin

    @params[in]   MANAGE_WP
                  WP management switch defined in header file
    @params[in]   wpPin
                  pin number for WP pin
	@param[out]	  wpStatus
	@returns
				  0: success
				  1: error, WP not managed
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::disableWP() {
	byte result;
	if (MANAGE_WP) {
		digitalWrite(wpPin,LOW);
		wpStatus = false;
		result = ERROR_0;
	}
	else {
		result = ERROR_10;
	}
	return result;
}
/**************************************************************************/
/*!
    @brief  Erase device by overwriting it to 0x00

    @params[in]   SERIAL_DEBUG
                  Outputs erasing results to Serial
	@returns
				  0: success
				  1-4: error writing at a certain position
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::eraseDevice(void) {
		byte result = 0;
		uint16_t i = 0;
		
		#ifdef SERIAL_DEBUG
			if (Serial){
				Serial.println("Start erasing device");
			}
		#endif
		
		while((i < MAXADDRESS_04) && (result == 0)){
		  result = FRAM_FM24CXX_I2C::writeByte(i, 0x00);
		  i++;
		}
		
	
		#if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
			if (Serial){
				if (result !=0) {
						Serial.print("ERROR: device erasing stopped at position ");
						Serial.println(i, DEC);
						Serial.println("...... ...... ......");
				}
				else {
						Serial.println("device erased");
						Serial.println("...... ...... ......");
				}
			}
		#endif
		return result;
}


/*========================================================================*/
/*                           PRIVATE FUNCTIONS                            */
/*========================================================================*/


/**************************************************************************/
/*!
    @brief  Init write protect function for class constructor

    @params[in]   MANAGE_WP
                  WP management switch defined in header file
    @params[in]   wpPin
                  pin number for WP pin
    @params[in]   wp
                  Boolean for startup WP
	@param[out]	  wpStatus
	@returns
				  0: success, init done
				  1: error - should never happen
*/
/**************************************************************************/
byte FRAM_FM24CXX_I2C::initWP(boolean wp) {
	byte result;
	if (MANAGE_WP) {
		pinMode(wpPin,OUTPUT);
		if (wp) {
			result = FRAM_FM24CXX_I2C::enableWP();
		}
		else {
			result = FRAM_FM24CXX_I2C::disableWP();
		}
	}
	else {
		wpStatus = false;
		result = ERROR_0;
	}
	return result;
}

