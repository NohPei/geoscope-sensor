// ADCModule.h

#ifndef _ADCMODULE_h
#define _ADCMODULE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <SPI.h>

#define TIMER1_WRITE_TIME 10000 //5MHz/10000 = 500Hz
#define ADC_HOLD_TIME_CYCLES 8 //from MCP3201 datasheet: t_SUCS >= 100ns


void adcSetup();
void adcPoll();
void samplingEnable();
void samplingDisable();
void changeAmplifierGain(int);
void gainSave();
void gainLoad();

#endif
