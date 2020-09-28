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

#define TIMER1_WRITE_TIME 10000


void adcSetup();
void ICACHE_RAM_ATTR dataRead_isr();
void inline setDataBits(uint16_t);
void interuptEnable();
void interuptDisable();
void changeAmplifierGain(int);
void gainSave();
void gainLoad();

#endif
