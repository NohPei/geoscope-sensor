/**
 * TI TPL0501 Arduino SPI Driver
 * Author: Jesse R Codling
 * Created: 27 Aug 2024
 * Last Modified: 12 Sep 2024
 */

#ifndef _TPL0501_H_
#define _TPL0501_H_

#include <Arduino.h>
#include <SPI.h>

class TPL0501
{
	public:
		TPL0501(uint8_t SSpin);
		void write(uint8_t data);
		// void configureSPI(); unused while maintaining a unified SPI config
		static uint16_t steps();
	private:
		uint8_t pin;
		// 	Use the same SPI frequency as everything else.
		// 	Per datasheet, can be up to 25 MHz
		const SPISettings SPIConfig = SPISettings(800e3, MSBFIRST, SPI_MODE0);


};



#endif //ndef _TPL0501_H_
