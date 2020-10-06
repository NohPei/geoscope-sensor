// 
// 
// 

#include "ADCModule.h"
#include "cli.h"

// spi cs pin
const uint8_t adcSSpin = 15; // ADC slave select pin
const int8_t gain_d0 = D1;
const int8_t gain_d1 = D2;
const int8_t gain_d2 = D3;

bool fullfilledBuffer = false;
int currentBufferRow = 0;
int currentBufferPosition = 0;
uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];

void adcSetup() {
	// SPI Setup
	SPI.begin();
	pinMode(adcSSpin,OUTPUT); //enable SS pin for manual operation
	
	// Gain setup
	pinMode(gain_d0, OUTPUT);
	pinMode(gain_d1, OUTPUT);
	pinMode(gain_d2, OUTPUT);
	gainLoad();
	changeAmplifierGain(amplifierGain);

	// TIMER1 ISR Setup
	timer1_isr_init();
	interuptEnable();
	digitalWrite(adcSSpin,HIGH); //ensure that SS is disabled (until the interrupt triggers)
}

//ADC sampling interrupt handler
void ICACHE_RAM_ATTR adcEnable_isr() {
	digitalWrite(adcSSpin,LOW); //dropping the SS pin enables the ADC, captures and holds one sample
}

//disable and reset the sampling
void interuptDisable() {
	timer1_detachInterrupt();
	timer1_disable();
	fullfilledBuffer = false;
	currentBufferPosition = 0;
	SPI.endTransaction();
}

//enable sampling by interrupt
void interuptEnable() {
	timer1_attachInterrupt(adcEnable_isr);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);			// 80MHz / 16 = 5MHz
	timer1_write(TIMER1_WRITE_TIME);
	SPI.beginTransaction(SPISettings(0.8e6, MSBFIRST, SPI_MODE0));
	//enale SPI at 800kHz max rate (from MCP3201 datasheet)
}

void adcPoll() {
	if (!digitalRead(adcSSpin)) { //if we've enabled the ADC
		uint16_t rawVal = SPI.transfer16(0); //the ADC doesn't take input, but we have to send something
		rawVal = rawVal >> 1; //16 bits transferred: 1 null, 12 data, 3 junk. Throw away the junk
		rawVal &= 0x0FFF; //only the lower 12 bits are valid, so toss the high 4 bits
		rawBuffer[currentBufferRow][currentBufferPosition++] = rawVal;

		if (cli.isStreaming()) {
			cli.println(rawVal,DEC);
		}
		

		if (currentBufferPosition == RAW_COL_BUFFER_SIZE) { //check for filled buffers
			fullfilledBuffer = true;
			currentBufferPosition = 0;
			currentBufferRow++;
			if (currentBufferRow == RAW_ROW_BUFFER_SIZE) {
				currentBufferRow = 0;
			}
		}

		digitalWrite(adcSSpin, HIGH); //cut the ADC back off

	}
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
	default:
		digitalWrite(gain_d0, HIGH);
		digitalWrite(gain_d1, HIGH);
		digitalWrite(gain_d2, HIGH);
		break;
	}

	gainSave();
}

void gainLoad() {
	File storage = LittleFS.open("config/gain", "r");
	if (storage.isFile())
		amplifierGain = storage.readString().toInt();
	storage.close();
}

void gainSave() {
	File storage = LittleFS.open("config/gain", "w");
	storage.print(amplifierGain, DEC);
	storage.close();
}
