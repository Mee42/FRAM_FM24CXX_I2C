/**************************************************************************/
/*! 
    @file     FRAM_FM24CXX_I2C.h
    @author   SOSAndroid.fr (E. Ha.)
	
    @section  HISTORY

    v1.0 - First release
	v1.0.1 - Robustness enhancement
	v1.0.2 - fix constructor, introducing byte move in memory
	v1.0.3 - fix writeLong() function
	v1.0.4 - fix constructor call error
	v1.0.5 - Enlarge density chip support by making check more flexible, Error codes not anymore hardcoded, add connect example, add Cypress FM24 & CY15B series comment.
	v1.1.0b - adding support for devices without device IDs + 4K & 16 K devices support
	v1.2.0 - Uses reinterpret_cast instead of bit shift / masking for performance. Breaks backward compatibility with previous code - See PR#6

	
	v1.0 - First rework release fot FM24CXX_I2c	 fram.
	
    Driver for the FM24CXX_I2C I2C FRAM 
	
    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, SOSAndroid.fr (E. Ha.)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#ifndef _FRAM_FM24CXX_I2C_
#define _FRAM_FM24CXX_I2C_

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include <Wire.h>

// Enabling debug I2C - comment to disable / normal operations
#ifndef SERIAL_DEBUG
#define SERIAL_DEBUG 1
#endif


#define MAXADDRESS_04 512
//#define MAXADDRESS_16 2048
//#define MAXADDRESS_64 8192


// Adresses
//#define FM24CXX_ADDRESS_A000   0x50
//#define FM24CXX_ADDRESS_A001   0x51
//#define FM24CXX_ADDRESS_A010   0x52
//#define FM24CXX_ADDRESS_A011   0x53
//#define FM24CXX_ADDRESS_A100   0x54
//#define FM24CXX_ADDRESS_A101   0x55
//#define FM24CXX_ADDRESS_A110   0x56
//#define FM24CXX_ADDRESS_A111   0x57
//#define FM24CXX_DEFAULT_ADDRESS   FM24CXX_ADDRESS_A000

//Special commands
//#define MASTER_CODE	0xF8
//#define SLEEP_MODE	0x86 //Cypress codes, not used here	
//#define HIGH_SPEED	0x08 //Cypress codes, not used here

// Managing Write protect pin
#define MANAGE_WP false //false if WP pin remains not connected
#define DEFAULT_WP_PIN	13 //write protection pin - active high, write enabled when low
#define DEFAULT_WP_STATUS  false //false means protection is off - write is enabled

// Error management
#define ERROR_0 0 // Success    
#define ERROR_1 1 // Data too long to fit the transmission buffer on Arduino
#define ERROR_2 2 // received NACK on transmit of address
#define ERROR_3 3 // received NACK on transmit of data
#define ERROR_4 4 // Serial seems not available
#define ERROR_5 5 // Not referenced device ID
#define ERROR_6 6 // Unused
#define ERROR_7 6 // Fram chip unidentified
#define ERROR_8 8 // Number of bytes asked to read null
#define ERROR_9 9 // Bit position out of range
#define ERROR_10 10 // Not permitted opération
#define ERROR_11 11 // Memory address out of range


class FRAM_FM24CXX_I2C {
 public:
	FRAM_FM24CXX_I2C(uint8_t address, boolean wp, int pin, uint16_t chipDensity);
	
	void	begin(void);
	byte	readBit(uint16_t framAddr, uint8_t bitNb, byte *bit);
	byte	setOneBit(uint16_t framAddr, uint8_t bitNb);
	byte	clearOneBit(uint16_t framAddr, uint8_t bitNb);
	byte	toggleBit(uint16_t framAddr, uint8_t bitNb);
	byte	readArray (uint16_t framAddr, byte items, uint8_t value[]);
	byte	writeArray (uint16_t framAddr, byte items, uint8_t value[]);
	byte	readByte (uint16_t framAddr, uint8_t *value);
	byte	writeByte (uint16_t framAddr, uint8_t value);
	byte	copyByte (uint16_t origAddr, uint16_t destAddr);
	byte	readWord(uint16_t framAddr, uint16_t *value);
	byte	writeWord(uint16_t framAddr, uint16_t value);
	byte	readLong(uint16_t framAddr, uint32_t *value);
	byte	writeLong(uint16_t framAddr, uint32_t value);
	boolean	getWPStatus(void);
	byte	enableWP(void);
	byte	disableWP(void);
	byte	eraseDevice(void);
  
 private:
	uint8_t	i2c_addr;
	int	wpPin;
	boolean	wpStatus;

	
	byte	initWP(boolean wp);
	void	I2CAddressAdapt(uint16_t framAddr);
};

#endif
