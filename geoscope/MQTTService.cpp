// 
// 
//


#include "MQTTService.h"
#include <ArduinoOTA.h>
#include "Network.h"
#include "cli.h"

char MQTT_BROKER_IP[CHAR_BUF_SIZE] = "192.168.60.100";
int MQTT_BROKER_PORT = 18884;
int MQTT_BROKER_TIMEOUT = 30000; //timeout without reaching the broker before rebooting the sensor

String clientId = "152";
String 	MQTT_TOPIC,
	payloadHeader,
	payload;

WiFiClient wifiClient;
MQTTClient mqttclient(MQTT_PACKAGE_SIZE);

unsigned long* attemptTimeout = NULL;

int amplifierGain = 100;

void mqttSetup() {
	MQTT_TOPIC = "geoscope/node1/" + clientId;
	payloadHeader = "{\"uuid\":\"GEOSCOPE-" + clientId + "\",\"data\":\"[";
	WiFi.hostname("GECOSCOPE-"+clientId);
	ArduinoOTA.setHostname(("GEOSCOPE-"+clientId).c_str());


	mqttclient.begin(MQTT_BROKER_IP, MQTT_BROKER_PORT, wifiClient);
	mqttclient.onMessage(mqttOnMessage);

	if (!mqttclient.connected()) {
		if (!attemptTimeout)
			attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
		mqttConnect();
	}

	payload = payloadHeader;
	payload += "Device Started]\"}";
	mqttclient.publish("geoscope/reply", payload);

	if (!mqttclient.connected()) {
		if (!attemptTimeout)
			attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
		mqttConnect();
	}
	gainLoad();
	payload = payloadHeader;
	payload += "Current gain: " + String(amplifierGain) + "]\"}";
	mqttclient.publish("geoscope/reply", payload);
}

void mqttConnect() {
	if (MQTT_BROKER_TIMEOUT && attemptTimeout && millis() > *attemptTimeout) {
		//initiates self reset if we're requiring Broker for operation (can be turned off during testing),
		// 	and there's a set attemptTimeout
		payload = payloadHeader;
		payload += "Device Self Reset]\"}";
		mqttclient.publish("geoscope/reply", payload);
		forceReset();
	}
	// Attempt to connect
	mqttclient.disconnect();
	if (mqttclient.connect(("GEOSCOPE-" + clientId).c_str()))
	{
		delete attemptTimeout;
		attemptTimeout = NULL;
		// Subscribe to topic geoscope/config/gain
		mqttclient.subscribe("geoscope/config/gain");
		mqttclient.subscribe("geoscope/restart");
	}
	else
	{
		minYield(20);
	}
}

void mqttSend() {
	if (fullfilledBuffer) {
		int buffer_row = 0;
		if (currentBufferRow == 0) {
			buffer_row = RAW_ROW_BUFFER_SIZE - 1;
		}
		else {
			buffer_row = currentBufferRow - 1;
		}
		fullfilledBuffer = false;
		payload = payloadHeader;
		for (int i = 0; i < RAW_COL_BUFFER_SIZE; i++) {
			payload += String(rawBuffer[buffer_row][i]) + ",";
		}
		payload += "]\",\"gain\":" + String(amplifierGain) + "}";

		if (!mqttclient.connected()) {
			if (!attemptTimeout)
				attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
			mqttConnect();
		}

		mqttclient.publish(MQTT_TOPIC, payload);
	}

	mqttclient.loop();
	delay(10);
}

void mqttOnMessage(String & topic, String & in_payload) {
	String payload;
	if (topic.equalsIgnoreCase("geoscope/config/gain")) {
		interuptDisable();
		// Set new amplifier gain value
		amplifierGain = in_payload.toInt();
		changeAmplifierGain(amplifierGain);
		String payloads = payloadHeader;
		payloads += "Set new gain to "+ in_payload +"]\"}";
		mqttclient.publish("geoscope/reply", payloads);
		minYield(10);
		interuptEnable();
	}
	else if (topic.equalsIgnoreCase("geoscope/restart")) {
		interuptDisable();
		String payloads = payloadHeader;
		payloads += "Restart]\"}";
		mqttclient.publish("geoscope/reply", payloads);
		minYield(10);
		forceReset();
	}
	else if (topic.equalsIgnoreCase("geoscope/hb")) {
		payload = payloadHeader;
		payload += "GEOSCOPE-" + clientId;
		payload += " working.]\"}";
		mqttclient.publish("geoscope/reply", payload);
	}
}

void mqttLoad() {
	File storage = LittleFS.open("config/mqtt", "r");
	while (storage.available()) {
		String line = storage.readStringUntil('\n');
		line.trim(); //cut off any trailing whitespace, hopefully including extraneous newlines
		String arg = line.substring(line.lastIndexOf(' ')+1);
		switch (line[0]) {
			case 'i': //IP (for Broker)
				strncpy(MQTT_BROKER_IP, arg.c_str(), CHAR_BUF_SIZE);
				MQTT_BROKER_IP[CHAR_BUF_SIZE-1] = 0;
				break;
			case 'p': //Port (for Broker)
				MQTT_BROKER_PORT = arg.toInt();
				break;
			case 't': //Timeout
				MQTT_BROKER_TIMEOUT = arg.toInt();
				break;
			case 'c': //Client ID
				clientId = arg;
			default:
				//ignore. This line is invalid
				break;
		}
	}
	storage.close();
}

void mqttSave() {
	File storage = LittleFS.open("config/mqtt", "w");
	storage.print("i ");
	storage.println(MQTT_BROKER_IP);
	storage.print("p ");
	storage.println(MQTT_BROKER_PORT);
	storage.print("t ");
	storage.println(MQTT_BROKER_TIMEOUT);
	storage.print("c ");
	storage.println(clientId);

	storage.close();
}
