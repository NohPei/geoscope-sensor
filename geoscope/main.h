// main.h

#ifndef _MAIN_h
#define _MAIN_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <string.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#define RAW_ROW_BUFFER_SIZE 2
#define RAW_COL_BUFFER_SIZE 500

#define CHAR_BUF_SIZE 32

#define DEVICE_IP 152



extern String timerNow;

extern int amplifierGain;

extern bool fullfilledBuffer;
extern int currentBufferRow;
extern uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];

#endif
