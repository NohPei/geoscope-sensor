// Timer_c.h

#ifndef _TIMER_C_h
#define _TIMER_C_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#define TIMEZONE 7							// UTC +7 BANGKOK
#define OFFSET 0							// daylight offset

void timeSetup();
void fetchTime();

#endif

