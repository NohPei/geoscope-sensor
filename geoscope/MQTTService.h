// MQTTSERVICE.h

#ifndef _MQTTSERVICE_h
#define _MQTTSERVICE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
//#include "Timer_c.h"
#include "Watchdog.h"
#include "ADCModule.h"
#include <MQTT.h>						// arduino editor
//#include <MQTT/src/MQTT.h>				// visual stuio editor

#define MQTT_PACKAGE_SIZE 15000

extern char MQTT_BROKER_IP[CHAR_BUF_SIZE];
extern int MQTT_BROKER_PORT;
extern String clientId;
extern int MQTT_BROKER_TIMEOUT;

void mqttSetup();
void mqttConnect();
void mqttSend();
void mqttLoad();
void mqttSave();
void mqttOnMessage(String &topic, String &payload);
void mqttReportGain(float newGain);
void mqttShutdown();
void mqttNotify(String message);

#endif

