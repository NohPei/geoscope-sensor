/**
 * TI TPL0501 Arduino SPI Driver
 * Author: Jesse R Codling
 * Created: 27 Aug 2024
 * Last Modified: 27 Aug 2024
 */


#include "TPL0501.h"

TPL0501::TPL0501(uint8_t SSpin) {
	this->pin = SSpin;
	pinMode(SSpin, OUTPUT);
	digitalWrite(SSpin, HIGH);
}


void TPL0501::write(uint8_t data) {
	SPI.beginTransaction(this->SPIConfig);
	digitalWrite(this->pin, LOW);
	SPI.write(data);
	digitalWrite(this->pin, HIGH);
	SPI.endTransaction();

}
