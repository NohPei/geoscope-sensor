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

	TelnetStream2.begin();
	Serial.println(F( "> Remote Console Configured" ));
	TelnetStream2.println(F( "> Remote Console Configured" ));
	//	//timeSetup();
	cliInit();
	cli.println("> CLI Ready");
	mqttSetup();
	cli.println("> MQTT Configured");
	adcSetup();
	cli.println("> ADC Configured");
	//	//fetchTime();

	// OTA Setup
	ArduinoOTA.begin();
	cli.println("> OTA Ready");

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
	cli.println("> Boot Complete");

	cli.printCommandPrompt();
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	adcPoll();
	mqttSend();

	if (cli.isStreaming() && cli.getInputPort()->available()) {
		while(cli.getInputPort()->available()) //clear the input buffer
			cli.getInputPort()->read();
		cli.stopStreaming();
	}
	cli.update();

	if (cli.getAltPort()->available()) { //if we get input on the other port
		cli_swap(); //swap to that port
		while(cli.getInputPort()->available()) //clear the input buffer
			cli.getInputPort()->read();
	}
	ESP.wdtFeed();
}
