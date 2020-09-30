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
	initWifiConfig();
	wifiSetup();
	delay(1000);
	//timeSetup();
	mqttSetup();
	adcSetup();
	//fetchTime();
	cliInit();
	dataDump = false;

	// OTA Setup
	ArduinoOTA.begin();

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	mqttSend();
	ESP.wdtFeed();
	if (dataDump) {
		if (Serial.available()) { //if a character has come in
			dataDump = false; //stop dumping
			while (Serial.available()) //dump out the incoming buffer
				Serial.read();
		}
	}
	else { //if we're not dumping data
		cmdPoll();

	}
}
