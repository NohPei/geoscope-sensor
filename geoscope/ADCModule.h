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
#include <TPL0501.h>

#define ADC_HOLD_TIME_CYCLES 8 //from MCP3201 datasheet: t_SUCS >= 100ns
#define SAMP_CLK_ERROR_THRESHOLD 10 //number of too-short or too-long samples to wait before adjusting the sample clock

extern TPL0501* gainPot;
extern double gainShiftRatio;
extern double gainMin;
extern uint8_t potVal;
extern unsigned int sample_rate;

void adcSetup();
void adcPoll();
void samplingEnable();
void samplingDisable();
void changeAmplifierGain(float);
void setPotValue(uint16_t);
void changeSampleRate(unsigned int);
void gainSave();
void gainLoad();

#endif
