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
#include <AsyncPing.h>

#define TIMEZONE "America/New_York" //Eastern Standard/Daylight Time
#define NTP_SERVER "pool.ntp.org" //standard ntp pool server.
// if better guarantees are needed, try "time.nist.gov"


static bool OTA_FS_UPDATE = false;

#define WIFI_PING_INTERVAL_MS 60000
static unsigned long nextPingTime = 0;

void ota_startup() {
	OTA_FS_UPDATE = ArduinoOTA.getCommand() == U_FS;
	if (OTA_FS_UPDATE) {
		cli.println(F("<< WARNING: Updating Filesystem via OTA Update >>"));
		mqttNotify("OTA Filesystem Update");
		yield();
		LittleFS.end();
	}
	else {
		backup(); //save configurations to FS
		mqttNotify("OTA Firmware Update");
		mqttShutdown();
		yield();
		cli.println(F("> Updating Firmware over OTA"));
	}
}

void ota_done() {
	if (OTA_FS_UPDATE) {
		LittleFS.begin();
		OTA_FS_UPDATE = false; //unset the flag
	}
}

void ota_error(ota_error_t error) {
	if (OTA_FS_UPDATE) {
		cli.print(F("<< FS UPDATE ERROR "));
		cli.print(error);
		cli.println(F( " >>" ));
		mqttNotify("OTA FS Update Error");
		ota_done();
	}
	else {
		cli.print(F( "<< FIRMWARE UPDATE ERROR " ));
		cli.print(error);
		cli.println(F( " >>" ));
		ESP.restart();
	}
}

//TODO: should we just remove WebDAV since it's been buggy?
WiFiServer tcp(80);
ESPWebDAV dav;


AsyncPing ping;

bool ping_received(const AsyncPingResponse& response) {
	return false; //keep going
}

bool ping_done(const AsyncPingResponse& response) {
	if (response.total_recv == 0) { //if we got no responses
		IPAddress nullIP = IPAddress(0,0,0,0);
		WiFi.config(nullIP, nullIP, nullIP); //force WiFi back to DHCP
		MQTT_BROKER_TIMEOUT = 300000; //disable MQTT timeout because DHCP is slower
		if (WiFi.status() == WL_CONNECTED)
			WiFi.reconnect(); //and reconnect (or just connect freshly)
		return false;
	}
	return true;
}



void setup() {
	Serial.begin(115200);
	Serial.println(F( "\n> Starting Geoscope Boot" ));

	LittleFS.begin();
	Serial.println(F( "> FS Mounted" ));

	loadWifiConfig();
	wifiSetup();
	yield();
	Serial.println(F( "> WiFi Configured" ));

	TelnetStream.begin();
	Serial.println(F( "> Remote Console Configured" ));
	TelnetStream.println(F( "> Remote Console Configured" ));

	cliInit();
	cli.println(F( "> CLI Ready" ));

	//TODO: Why is this still here? These have no time concept that matters?
	configTime(TIMEZONE, NTP_SERVER);
	cli.println(F( "> Time Configured" ));

	mqttLoad(); //attempt to load configuration file
	mqttSetup();
	cli.println(F( "> MQTT Configured" ));

	//TODO: do this before MQTT so gain can be loaded in the right places
	adcSetup();
	cli.println(F( "> ADC Configured" ));

	// OTA Setup
	ArduinoOTA.onStart(ota_startup);
	ArduinoOTA.onEnd(ota_done);
	ArduinoOTA.onError(ota_error);
	ArduinoOTA.setHostname(("GEOSCOPE_"+clientId).c_str()); //TODO: didn't we do this once already?
	ArduinoOTA.begin();
	cli.println(F( "> OTA Ready" ));

	tcp.begin();
	dav.begin(&tcp, &LittleFS);
	cli.println(F( "> Remote FS Access Ready" ));

	ping.on(true, ping_received);
	ping.on(false, ping_done);
	nextPingTime = millis() + WIFI_PING_INTERVAL_MS;
	cli.println(F("> Keepalive Ping Enabled"));

	ESP.wdtDisable();
	ESP.wdtEnable(5000);
	cli.println(F( "> Boot Complete" ));

	cli.printCommandPrompt();
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	dav.handleClient();
	adcPoll();
	mqttSend();

	if (millis() >= nextPingTime ) { //every interval
		nextPingTime = millis() + WIFI_PING_INTERVAL_MS;
		ping.begin(WiFi.gatewayIP(),5); //try pinging the gateway
	}

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
