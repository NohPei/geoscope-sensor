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
#include <FTPServer.h>
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

void ota_error(ota_error_t error) {
	if (OTA_FS_UPDATE) {
		cli.print("<< FS UPDATE ERROR ");
		cli.print(error);
		cli.println(" >>");
		ota_done();
	}
	else {
		cli.print("<< FIRMWARE UPDATE ERROR ");
		cli.print(error);
		cli.println(" >>");
		ESP.restart();
	}
}

FTPServer ftp(LittleFS);

void setup() {
	Serial.begin(115200);
	Serial.println(F( "\n> Starting Geoscope Boot" ));

	LittleFS.begin();
	Serial.println(F( "> FS Mounted" ));

	loadWifiConfig();
	wifiSetup();
	yield();
	Serial.println(F( "> WiFi Connected" ));

	TelnetStream.begin();
	Serial.println(F( "> Remote Console Configured" ));
	TelnetStream.println(F( "> Remote Console Configured" ));

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
	ArduinoOTA.onStart(ota_startup);
	ArduinoOTA.onEnd(ota_done);
	ArduinoOTA.onError(ota_error);
	ArduinoOTA.setHostname(("GEOSCOPE_"+clientId).c_str());
	ArduinoOTA.begin();
	cli.println(F( "> OTA Ready" ));

	ftp.begin("","");
	cli.println(F( "> Remote FS Access Ready" ));

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
	cli.println(F( "> Boot Complete" ));

	cli.printCommandPrompt();
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	ftp.handleFTP();
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
