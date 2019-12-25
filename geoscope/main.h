// main.h

#ifndef _MAIN_h
#define _MAIN_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <string.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#define RAW_ROW_BUFFER_SIZE 2
#define RAW_COL_BUFFER_SIZE 500

#define DEVICE_IP 152
#define MQTT_CLIENT_ID "152"
#define MQTT_CLIENT_ID_FULL "GEOSCOPE-" MQTT_CLIENT_ID
#define MQTT_PUB_TOPIC "geoscope/node1/" MQTT_CLIENT_ID
#define MQTT_PAYLOAD_HEADER "{\"uuid\":\"" MQTT_CLIENT_ID_FULL "\",\"data\":\"["



extern String timerNow;

extern int amplifierGain;

extern bool fullfilledBuffer;
extern int currentBufferRow;
extern uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];

#endif
