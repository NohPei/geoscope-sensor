//
//
//

#include "ADCModule.h"
#include "cli.h"
#include "AD5270.h"

// spi cs pin
const uint8_t adcSSpin = 4; // ADC slave select pin
const uint8_t potSSpin = 5; // digital pot slave select pin

bool fullfilledBuffer = false;
unsigned int currentBufferRow = 0;
unsigned int currentBufferPosition = 0;
uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];
volatile uint32_t adcReadableTime_cycles = 0;
AD5270 gainPot(potSSpin);

const SPISettings adcConfig = SPISettings(0.8e6, MSBFIRST, SPI_MODE0);
//enale SPI at 800kHz max rate (from MCP3201 datasheet)

void adcSetup() {
	// SPI Setup
	SPI.begin();
	pinMode(adcSSpin,OUTPUT); //enable SS pin for manual operation

	digitalWrite(adcSSpin,HIGH); //ensure that SS is disabled (until the interrupt triggers)


	// Digital pot setup
	gainLoad();
	changeAmplifierGain(amplifierGain);

	// TIMER1 ISR Setup
	timer1_isr_init();
	samplingEnable();
}

//ADC sampling interrupt handler
void IRAM_ATTR adcEnable_isr() {
	digitalWrite(adcSSpin,LOW); //dropping the SS pin enables the ADC, captures and holds one sample
	adcReadableTime_cycles = ESP.getCycleCount() + ADC_HOLD_TIME_CYCLES;
}

//disable and reset the sampling
void samplingDisable() {
	timer1_detachInterrupt();
	timer1_disable();
	fullfilledBuffer = false;
	currentBufferPosition = 0;
	digitalWrite(adcSSpin, HIGH); //make sure the ADC isn't waiting to be read
	SPI.endTransaction();
}

//enable sampling by interrupt
void samplingEnable() {
	timer1_attachInterrupt(adcEnable_isr);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);			// 80MHz / 16 = 5MHz
	timer1_write(TIMER1_WRITE_TIME);
	SPI.beginTransaction(adcConfig);
}

void adcPoll() {
	if (!digitalRead(adcSSpin) && ESP.getCycleCount() > adcReadableTime_cycles) { //if we've enabled the ADC and it's ready to be read
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

void changeAmplifierGain(float val) {
	amplifierGain = val;
	static uint16_t potValue = 0;

	if (val <= 1) {
		gainPot.shutdown(true);
		//minimum gain (1) comes by shutting down the 
		amplifierGain = 1;
	}
	else {
		// calculate the nearest gain resistor value
		potValue = round(1024/(val-1));
		if (potValue >= 1024) {
			potValue = 1023;
			//the max input value is 2^10-1
		}
		amplifierGain = 1 + 1024.0/potValue;
		//correct the stored gain value to the actual set value
		
		gainPot.write(RDAC_WRITE, potValue);
		gainPot.shutdown(false);
		//make sure the resistor is enabled, too.
	}

	gainSave();
}

void gainLoad() {
	File storage = LittleFS.open("/config/gain", "r");
	if (!storage)
		amplifierGain = 0;
	else
		amplifierGain = storage.readString().toFloat();
	storage.close();
}

void gainSave() {
	File storage = LittleFS.open("/config/gain", "w");
	storage.printf("%.3f\n", amplifierGain);
	storage.close();
}
