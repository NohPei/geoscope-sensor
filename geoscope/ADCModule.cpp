//
//
//

#include "ADCModule.h"
#include "cli.h"
#include "Network.h"


bool fullfilledBuffer = false;
unsigned int currentBufferRow = 0;
unsigned int currentBufferPosition = 0;
uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];
volatile uint32_t adcReadableTime_cycles = 0;
volatile uint64_t last_macTime = 0;
uint64_t packet_macTime = 0;

//SPI Chip Select pins
#define adcSSpin 4 // ADC slave select pin
#define potSSpin 5 // digital pot slave select pin

MCP41xxx* gainPot = NULL;
double gainShiftRatio = 0;


const SPISettings adcConfig = SPISettings(0.8e6, MSBFIRST, SPI_MODE0);
//enale SPI at 800kHz max rate (from MCP3201 datasheet)

void adcSetup() {
	// SPI Setup
	SPI.begin();

	pinMode(adcSSpin,OUTPUT); //enable SS pin for manual operation
	digitalWrite(adcSSpin,HIGH); //ensure that SS is disabled (until the interrupt triggers)

	gainPot = new MCP41xxx(potSSpin);

	// Digital pot setup
	gainLoad();
	changeAmplifierGain(amplifierGain);

	// TIMER1 ISR Setup
	timer1_isr_init();
	samplingEnable();
}

union wide_reg_t {
		uint64_t r64;
		struct {
			uint32_t hi;
			uint32_t lo;
		} r32; 
};

//ADC sampling interrupt handler
void IRAM_ATTR adcEnable_isr() {
	// wide_reg_t tsf;

	digitalWrite(adcSSpin,LOW); //dropping the SS pin enables the ADC, captures and holds one sample

	//capture the sample timestamp
	last_macTime = esp8266_sta_tsf_time();


	//TODO: implement PD control for TIMER1_WRITE_TIME based on d/dt (last_macTime)
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
	if (!digitalRead(adcSSpin)) { //if we've enabled the ADC and it's ready to be read
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
			packet_macTime = last_macTime;
			if (currentBufferRow == RAW_ROW_BUFFER_SIZE) {
				currentBufferRow = 0;
			}
		}

		digitalWrite(adcSSpin, HIGH); //cut the ADC back off

	}
}

void changeAmplifierGain(float val) {
	amplifierGain = val;
	static int16_t potValue = 0;
	static const uint16_t potSteps = UINT8_MAX;

	if (val <= 1.0) {
		gainPot->shutdown();
		//minimum gain (1) comes by shutting down the pot
		amplifierGain = 1;
	}
	else {

		// calculate the nearest gain resistor value
		potValue = round(potSteps * (1-(1+gainShiftRatio)/val));
		if (potValue >= potSteps) {
			potValue = potSteps-1;
		}
		else if (potValue < 0) {
			potValue = 0;
		}
		double gainDivisor = potSteps-potValue;
		amplifierGain = 1.0 + potValue/gainDivisor + potSteps*gainShiftRatio/gainDivisor;
		//correct the stored gain value to the actual set value
		//TODO: re-work this math. The numbers are still coming out way wrong when gainShiftRatio=0

		if (timer1_enabled()) { //if the sampling is running
			samplingDisable(); //for SPI safety (and to keep the signals on known gain), disable interrupts here.
			gainPot->write(potValue);
			samplingEnable();
		}
		else
			gainPot->write(potValue);
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

	storage = LittleFS.open("/config/rPotRatio", "r");
	if (storage)
		gainShiftRatio = storage.readString().toFloat();
	storage.close();
}

void gainSave() {
	File storage = LittleFS.open("/config/gain", "w");
	storage.printf("%.3f\n", amplifierGain);
	storage.close();

	storage = LittleFS.open("/config/rPotRatio", "w");
	storage.printf("%f\n", gainShiftRatio);
	storage.close();
}
