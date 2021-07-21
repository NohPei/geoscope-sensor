/**
 * AD5270 Arduino SPI Driver (Implementation)
 * Author: Jesse R Codling
 * Created: 14 Jul 2021
 * Last Modified: 20 Jul 2021
 */

#include "AD5270.h"


AD5270::AD5270(uint8_t SSpin, bool writeProtect) {
	this->pin = SSpin;
	pinMode(SSpin, OUTPUT);
	digitalWrite(SSpin, HIGH);

	if (!writeProtect) {
		this->write(CTRL_WRITE, 0b010 & AD5270_DATA_MASK);
		//RDAC register is write-protected by default. This command
		// 	enables writing to it.
	}

}

void AD5270::write(AD5270_Command_t cmd, uint16_t data) {
	SPI.beginTransaction(this->SPIConfig);
	digitalWrite(this->pin, LOW);
	data &= AD5270_DATA_MASK;
	data &= cmd << 10;
	SPI.write16(data);
	digitalWrite(this->pin, HIGH);
	SPI.endTransaction();
}

uint16_t AD5270::read(AD5270_Command_t cmd, uint16_t data) {
	this->write(cmd, data);
	SPI.beginTransaction(this->SPIConfig);
	digitalWrite(this->pin, LOW);
	uint16_t readVal = SPI.transfer16(NOOP << 10);
	//send a NOOP command during response read
	digitalWrite(this->pin, HIGH);
	SPI.endTransaction();

	readVal &= AD5270_FRAME_MASK;

	AD5270_Command_t readCmd = (readVal & ~AD5270_DATA_MASK) >> 10;
	switch (readCmd) {
		case CTRL_READ:
			return readVal & 0x0F; //Control Register is only 4 bits
		case RDAC_READ:
		case PROM_READ:
		case PROM_READ_CURR:
		default:
			return readVal & AD5270_DATA_MASK;

	}
}

void AD5270::shutdown(bool offline) {
	this->write(SHUTDOWN, (uint16_t)offline);
}


