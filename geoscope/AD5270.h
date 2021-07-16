/**
 * AD5270 Arduino SPI Driver (Headers)
 * Author: Jesse R Codling
 * Created: 14 Jul 2021
 * Last Modified: 14 Jul 2021
 */

#ifndef _AD5270_H
#define _AD5270_H

#include <Arduino.h>

#define AD5270_FRAME_MASK 	0x3FFF
#define AD5270_DISABLE 		0x1
#define AD5270_ENABLE 		0x0

enum AD5270_Command {
	NOOP 		= 	0x0,
	RDAC_WRITE 	= 	0x1,
	RDAC_READ 	= 	0x2,
	RDAC_STORE 	= 	0x3,
	RESET 		= 	0x4,
	PROM_READ 	= 	0x5,
	PROM_READ_CURR 	= 	0x6,
	CTRL_WRITE 	= 	0x7,
	CTRL_READ 	= 	0x8,
	SHUTDOWN 	= 	0x9

};
typedef uint8_t AD5270_Command_t;

class AD5270
{

	public:
		AD5270(uint8_t pin);
		void write(AD5270_Command_t cmd, uint16_t data);
		uint16_t read(AD5270_Command_t cmd, uint16_t data);
		void configureSPI();
	private:

};


#endif //ndef _AD5270_H
