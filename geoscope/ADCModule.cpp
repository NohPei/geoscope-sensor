// 
// 
// 

#include "ADCModule.h"

// spi cs pin
const int8_t scePin = 15;									// ADC slave select pin
const int8_t gain_d0 = 5;
const int8_t gain_d1 = 4;
const int8_t gain_d2 = 0;

bool fullfilledBuffer = false;
int currentBufferRow = 0;
int currentBufferPosition = 0;
uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];

void adcSetup() {
	// SPI Setup
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV64);					// 80MHz/64 = 1.25MHz
	SPI.setHwCs(1);											// set slave select pin to GPIO15
	setDataBits(0x10);										// set data bit to 16bits
	
	// Gain setup
	gainLoad();
	changeAmplifierGain(amplifierGain);

	// TIMER1 ISR Setup
	timer1_isr_init();
	interuptEnable();

}

void interuptDisable() {
	timer1_detachInterrupt();
	timer1_disable();
	fullfilledBuffer = false;
	currentBufferPosition = 0;
}

void interuptEnable() {
	timer1_attachInterrupt(dataRead_isr);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);			// 80MHz / 16 = 5MHz
	//timer1_write(2500);									// 0.2us * 2500 500us -> 2K sample per secound
	timer1_write(TIMER1_WRITE_TIME);
}

void ICACHE_RAM_ATTR dataRead_isr() {
	uint16_t rawVal;
	uint16_t tmpVal;

	while (SPI1CMD & SPIBUSY)
	{
	}
	SPI1W0 = 0x00; // send 0x00 to adc
	SPI1CMD |= SPIBUSY;
	while (SPI1CMD & SPIBUSY)
	{
	}

	tmpVal = SPI1W0 & 0xFF00;
	tmpVal = tmpVal >> 8;
	rawVal = SPI1W0 << 8;
	rawVal = rawVal + tmpVal;
	rawVal = rawVal >> 1;
	rawVal = rawVal & 0x0FFF;

	rawBuffer[currentBufferRow][currentBufferPosition++] = rawVal;

	if (currentBufferPosition == RAW_COL_BUFFER_SIZE)
	{
		fullfilledBuffer = true;
		currentBufferPosition = 0;
		currentBufferRow++;
		if (currentBufferRow == RAW_ROW_BUFFER_SIZE) {
			currentBufferRow = 0;
		}
	}
}

void inline setDataBits(uint16_t bits) {
	const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
	bits--;
	SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}


void changeAmplifierGain(int val) {

	switch (val)
	{
	case 0:
		digitalWrite(gain_d0, LOW);
		digitalWrite(gain_d1, LOW);
		digitalWrite(gain_d2, LOW);
		break;
	case 1:
		digitalWrite(gain_d0, HIGH);
		digitalWrite(gain_d1, LOW);
		digitalWrite(gain_d2, LOW);
		break;
	case 2:
		digitalWrite(gain_d0, LOW);
		digitalWrite(gain_d1, HIGH);
		digitalWrite(gain_d2, LOW);
		break;
	case 5:
		digitalWrite(gain_d0, HIGH);
		digitalWrite(gain_d1, HIGH);
		digitalWrite(gain_d2, LOW);
		break;
	case 10:
		digitalWrite(gain_d0, LOW);
		digitalWrite(gain_d1, LOW);
		digitalWrite(gain_d2, HIGH);
		break;
	case 20:
		digitalWrite(gain_d0, HIGH);
		digitalWrite(gain_d1, LOW);
		digitalWrite(gain_d2, HIGH);
		break;
	case 50:
		digitalWrite(gain_d0, LOW);
		digitalWrite(gain_d1, HIGH);
		digitalWrite(gain_d2, HIGH);
		break;
	case 100:
		digitalWrite(gain_d0, HIGH);
		digitalWrite(gain_d1, HIGH);
		digitalWrite(gain_d2, HIGH);
		break;
	default:
		digitalWrite(gain_d0, HIGH);
		digitalWrite(gain_d1, HIGH);
		digitalWrite(gain_d2, HIGH);
		break;
	}

	gainSave();
}

void gainLoad() {
	uint16_t addressStart = 0;
	EEPROM.begin(1024);
	EEPROM.get(addressStart, amplifierGain);
	EEPROM.end();
}

void gainSave() {
	uint16_t addressStart = 0;
	EEPROM.begin(1024);

	EEPROM.put(addressStart, amplifierGain);
	EEPROM.commit();
	EEPROM.end();
}
