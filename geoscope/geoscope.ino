/*
 Name:		geoscope.ino
 Created:	6/13/2018 1:34:31 PM
 Author:	Sripong
*/

// the setup function runs once when you press reset or power the board
#include "Watchdog.h"
#include "Timer_c.h"
#include "ADCModule.h"
#include "MQTTService.h"
#include "Network.h"
#include <ArduinoOTA.h>

void setup() {
	Serial.begin(115200);
	initWifiConfig();
	//networkSetup();
	wifiSetup();
	delay(1000);
	timeSetup();
	mqttInit();
	mqttSetup();
	adcSetup();
	fetchTime();

	// OTA Setup
	Serial.println("======================================================================");
	Serial.println("## OTA Setup.");
	ArduinoOTA.setHostname(MQTT_CLIENT_ID_FULL);
	// ArduinoOTA.setPassword((const char *)"soupgeoscope");

	ArduinoOTA.onStart([]() {
		Serial.println("Start");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();
	Serial.println("----------------------------------------------------------------------");

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	mqttSend();
	ESP.wdtFeed();
}
