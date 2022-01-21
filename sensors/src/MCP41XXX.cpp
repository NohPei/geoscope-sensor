/**
 * MCP41xxx Arduino SPI Driver
 * Author: Jesse R Codling
 * Created: 22 Sep 2021
 * Last Modified: 25 Sep 2021
 */


#include "MCP41XXX.h"

#define MCP41_DATA_MASK 	0x00FF
#define MCP41_SHUTDOWN 		0x2100
#define MCP41_WRITE 		0x1100



MCP41xxx::MCP41xxx(uint8_t SSpin) {
	this->pin = SSpin;
	pinMode(SSpin, OUTPUT);
	digitalWrite(SSpin, HIGH);
}


void MCP41xxx::write(uint8_t data) {
	uint16_t outBuf = data;
	outBuf &= MCP41_DATA_MASK;
	outBuf |= MCP41_WRITE;

	SPI.beginTransaction(this->SPIConfig);
	digitalWrite(this->pin, LOW);
	SPI.write16(outBuf);
	digitalWrite(this->pin, HIGH);
	SPI.endTransaction();

}

void MCP41xxx::shutdown() {
	SPI.beginTransaction(this->SPIConfig);
	digitalWrite(this->pin, LOW);
	SPI.write16(MCP41_SHUTDOWN);
	digitalWrite(this->pin, HIGH);
	SPI.endTransaction();
}
