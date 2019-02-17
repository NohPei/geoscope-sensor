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

#define RAW_ROW_BUFFER_SIZE 3
#define RAW_COL_BUFFER_SIZE 500

#define LOGITUEDE "15.298453"
#define LATITUDE "101.152397"

#define DEVICE_IP 210
#define MQTT_CLIENT_ID "210"
#define MQTT_CLIENT_ID_FULL "GEOSCOPE-" MQTT_CLIENT_ID
#define MQTT_PUB_TOPIC "geoscope/node1/" MQTT_CLIENT_ID
#define MQTT_PAYLOAD_HEADER "{\"uuid\":\"" MQTT_CLIENT_ID_FULL "\",\"lat\":" LATITUDE ",\"lng\":" LOGITUEDE ",\"data\":\"["



extern String timerNow;

extern int amplifierGain;

extern bool fullfilledBuffer;
extern int currentBufferRow;
extern uint16_t rawBuffer[RAW_ROW_BUFFER_SIZE][RAW_COL_BUFFER_SIZE];

#endif

