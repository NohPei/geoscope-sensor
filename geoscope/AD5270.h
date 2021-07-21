/**
 * AD5270 Arduino SPI Driver (Headers)
 * Author: Jesse R Codling
 * Created: 14 Jul 2021
 * Last Modified: 20 Jul 2021
 */

#ifndef _AD5270_H
#define _AD5270_H

#include <Arduino.h>
#include <SPI.h>

#define AD5270_FRAME_MASK 	0x3FFF
#define AD5270_DATA_MASK 	0x03FF

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
		AD5270(uint8_t SSpin, bool writeProtect = false);
		void write(AD5270_Command_t cmd, uint16_t data = 0x0000);
		uint16_t read(AD5270_Command_t cmd, uint16_t data = 0x0000);
		void configureSPI();
		void shutdown(bool offline);
	private:
		uint8_t pin;
		// AD5270 can handle up to 50MHz(!), data clocks in ON falling edge
		const SPISettings SPIConfig = SPISettings(20e6, MSBFIRST, SPI_MODE2);


};


#endif //ndef _AD5270_H
