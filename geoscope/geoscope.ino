/*
Name:		geoscope.ino
Created:	6/13/2018 1:34:31 PM
Author:	Sripong
 */

// the setup function runs once when you press reset or power the board
#include "Watchdog.h"
//#include "Timer_c.h"
#include "ADCModule.h"
#include "MQTTService.h"
#include "Network.h"
#include "cli.h"
#include <ArduinoOTA.h>
#include "main.h"

void setup() {
	Serial.begin(115200);
	Serial.println("\n> Starting Geoscope Boot");
	initWifiConfig();
	wifiSetup();
	minYield(1000);
	//timeSetup();
	Serial.println("> WiFi Connected");
	//	//timeSetup();
	mqttSetup();
	Serial.println("> MQTT Configured");
	adcSetup();
	Serial.println("> ADC Configured");
	//	//fetchTime();
	cliInit();
	Serial.println("> CLI Ready");

	// OTA Setup
	ArduinoOTA.begin();
	Serial.println("> OTA Ready");

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
	Serial.println("> Boot Complete");

	cli.printCommandPrompt();
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	adcPoll();
	mqttSend();

	if (cli.isStreaming() && cli.getInputPort()->available()) {
		cli.stopStreaming();
	}
	cli.update();
	ESP.wdtFeed();
}
