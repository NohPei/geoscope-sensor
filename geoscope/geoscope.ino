/*
Name:		geoscope.ino
Created:	6/13/2018 1:34:31 PM
Author:	Sripong
 */

// the setup function runs once when you press reset or power the board
#include "Watchdog.h"
#include "ADCModule.h"
#include "MQTTService.h"
#include "Network.h"
#include "cli.h"
#include <ArduinoOTA.h>
#include <ESPWebDAV.h>
#include "main.h"

#define TIMEZONE "America/New_York" //Eastern Standard/Daylight Time
#define NTP_SERVER "pool.ntp.org" //standard ntp pool server.
// if better guarantees are needed, try "time.nist.gov"


static bool OTA_FS_UPDATE = false;

void ota_startup() {
	OTA_FS_UPDATE = ArduinoOTA.getCommand() == U_FS;
	if (OTA_FS_UPDATE) {
		cli.println(F("<< WARNING: Updating Filesystem via OTA Update >>"));
		LittleFS.end();
	}
	else {
		backup(); //save configurations to FS
		cli.println(F("> Updating Firmware over OTA"));
	}
}

void ota_done() {
	if (OTA_FS_UPDATE) {
		LittleFS.begin();
		backup(); //backup configuration to newly re-written filesystem
		OTA_FS_UPDATE = false; //unset the flag
	}
}

WiFiServer tcp(80);
ESPWebDAV dav;


void setup() {
	Serial.begin(115200);
	Serial.println(F( "\n> Starting Geoscope Boot" ));

	LittleFS.begin();
	Serial.println(F( "> FS Mounted" ));

	loadWifiConfig();
	wifiSetup();
	yield();
	Serial.println(F( "> WiFi Connected" ));

	TelnetStream2.begin();
	Serial.println(F( "> Remote Console Configured" ));
	TelnetStream2.println(F( "> Remote Console Configured" ));

	cliInit();
	cli.println(F( "> CLI Ready" ));

	configTime(TIMEZONE, NTP_SERVER);
	cli.println(F( "> Time Configured" ));

	mqttLoad(); //attempt to load configuration file
	mqttSetup();
	cli.println(F( "> MQTT Configured" ));

	adcSetup();
	cli.println(F( "> ADC Configured" ));

	// OTA Setup
	ArduinoOTA.begin();
	ArduinoOTA.onStart(ota_startup);
	ArduinoOTA.onEnd(ota_done);
	cli.println(F( "> OTA Ready" ));

	tcp.begin();
	dav.begin(&tcp, &LittleFS);
	cli.println(F( "> Remote FS Access Ready" ));

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
	cli.println(F( "> Boot Complete" ));

	cli.printCommandPrompt();
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	adcPoll();
	mqttSend();
	dav.handleClient();

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
