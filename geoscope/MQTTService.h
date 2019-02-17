// MQTTSERVICE.h

#ifndef _MQTTSERVICE_h
#define _MQTTSERVICE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include "Timer_c.h"
#include "Watchdog.h"
#include "ADCModule.h"
#include <MQTT.h>						// arduino editor
//#include <MQTT/src/MQTT.h>				// visual stuio editor

#define MQTT_PACKAGE_SIZE 15000

extern char MQTT_BROKER_IP[32];
extern int MQTT_BROKER_PORT;
//extern String MQTT_TOPIC;

void mqttSetup();
void mqttConnect();
void mqttSend();
void mqttInit();
void mqttLoad();
void mqttSave();
void mqttOnMessage(String &topic, String &payload);

#endif

