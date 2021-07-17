//
//
//

#include "ADCModule.h"
#include "cli.h"

// spi cs pin
const uint8_t adcSSpin = 4; // ADC slave select pin
const uint8_t potSSpin = 5; // digital pot slave select pin

bool fullfilledBuffer = false;
unsigned int currentBufferRow = 0;
unsigned int currentBufferPosition = 0;
uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];
volatile uint32_t adcReadableTime_cycles = 0;

const SPISettings adcConfig = SPISettings(0.8e6, MSBFIRST, SPI_MODE0);
//enale SPI at 800kHz max rate (from MCP3201 datasheet)
// AD5270 can handle up to 50MHz(!), data clocks in ON falling edge
const SPISettings potConfig = SPISettings(20e6, MSBFISRT, SPI_MODE2);

void adcSetup() {
	// SPI Setup
	SPI.begin();
	pinMode(adcSSpin,OUTPUT); //enable SS pin for manual operation
	pinMode(potSSpin, OUTPUT); //''

	digitalWrite(adcSSpin,HIGH); //ensure that SS is disabled (until the interrupt triggers)
	digitalWrite(potSSpin, HIGH);

	// Digital pot setup
	setResistorControl();
	gainLoad();
	changeAmplifierGain(amplifierGain);

	// TIMER1 ISR Setup
	timer1_isr_init();
	samplingEnable();
}

//ADC sampling interrupt handler
void ICACHE_RAM_ATTR adcEnable_isr() {
	digitalWrite(adcSSpin,LOW); //dropping the SS pin enables the ADC, captures and holds one sample
	adcReadableTime_cycles = ESP.getCycleCount() + ADC_HOLD_TIME_CYCLES;
}

//disable and reset the sampling
void samplingDisable() {
	timer1_detachInterrupt();
	timer1_disable();
	fullfilledBuffer = false;
	currentBufferPosition = 0;
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

void setResistorControl() {
	// I assume this won't interrupt the ADC but 
	// just in case, at least SPI will be in a clean state
	digitalWrite(adcSSpin, HIGH);
	SPI.endTransaction();
	// ---
	SPI.beginTransaction(potConfig);
	digitalWrite(potSSpin, LOW);
	SPI.transfer(0x1C01); // allow digital adjustment of wiper
	SPI.endTransaction();
	digitalWrite(potSSPin, LOW);
	// ---
}

void changeAmplifierGain(int val) {
	static const int commandBase = 0x0400; // top bits to signify gain change command
	static int gainOut = 0;

	switch (val)
	{
	case 0:
		gainOut = 0x0;
		break;
	case 1:
		gainOut = 0xA; // 1/1024 Mohms
		break;
	case 2:
		gainOut = 0x14;
		break;
	case 5:
		gainOut = 0x33;
		break;

	case 10:
		gainOut = 0x66;
		break;
	
	case 20:
		gainOut = 0xCC;
		break;
	
	case 50:
		gainOut = 0x200;
		break;
	
	case 100:
	default:
		gainOut = 0x3FF;
		break;
	}

	// I assume this won't interrupt the ADC but 
	// just in case, at least SPI will be in a clean state
	digitalWrite(adcSSpin, HIGH);
	SPI.endTransaction();
	// ---
	val = (val & 0x3ff) | commandBase; // bottom 10 bits for gain

	SPI.beginTransaction(potConfig);
	digitalWrite(potSSpin, LOW);
	SPI.transfer(val); 
	SPI.endTransaction();
	digitalWrite(potSSPin, LOW);
	// ---

	gainSave();
}

void gainLoad() {
	File storage = LittleFS.open("/config/gain", "r");
	if (!storage)
		amplifierGain = 0;
	else
		amplifierGain = storage.readString().toInt();
	storage.close();
}

void gainSave() {
	File storage = LittleFS.open("/config/gain", "w");
	storage.println(amplifierGain, DEC);
	storage.close();
}
