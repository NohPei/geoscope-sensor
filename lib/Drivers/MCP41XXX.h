/**
 * MCP41xxx Arduino SPI Driver
 * Author: Jesse R Codling
 * Created: 22 Sep 2021
 * Last Modified: 27 Aug 2024
 */

#ifndef _MCP41XXX_H_
#define _MCP41XXX_H_

#include <Arduino.h>
#include <SPI.h>

class MCP41xxx
{
	public:
		MCP41xxx(uint8_t SSpin);
		void write(uint8_t data);
		// void configureSPI(); unused while maintaining a unified SPI config
		void shutdown();
	private:
		uint8_t pin;
		// 	Use the same SPI frequency as everything else.
		const SPISettings SPIConfig = SPISettings(800e3, MSBFIRST, SPI_MODE0);


};



#endif //ndef _MCP41XXX_H_
