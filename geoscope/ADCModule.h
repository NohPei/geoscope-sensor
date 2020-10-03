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


void adcSetup();
void adcPoll();
void interuptEnable();
void interuptDisable();
void changeAmplifierGain(int);
void gainSave();
void gainLoad();

#endif
