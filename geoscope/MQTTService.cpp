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
	payload,
	CONFIG_TOPIC_PREFIX;

WiFiClient wifiClient;
MQTTClient mqttclient(MQTT_PACKAGE_SIZE);

unsigned long* attemptTimeout = NULL;

float amplifierGain = 100;

void mqttNotify(String message) {
	payload = payloadHeader;
	payload += "\"[" + message + "]\"}";
	mqttclient.publish("geoscope/reply", payload, true, LWMQTT_QOS1);
}


void mqttSetup() {
	MQTT_TOPIC = "geoscope/node1/" + clientId;
	CONFIG_TOPIC_PREFIX = "geoscope/nodeconfig/" + clientId + "/";
	payloadHeader = "{\"uuid\":\"GEOSCOPE-" + clientId + "\",\"data\":";
	WiFi.hostname("GEOSCOPE-"+clientId);
	ArduinoOTA.setHostname(("GEOSCOPE-"+clientId).c_str());


	mqttclient.begin(MQTT_BROKER_IP, MQTT_BROKER_PORT, wifiClient);
	mqttclient.onMessage(mqttOnMessage);

	if (!mqttclient.connected()) {
		if (!attemptTimeout)
			attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
		mqttConnect();
	}

	mqttNotify("Device Started");

	payload = payloadHeader;
	payload += "\"[Unexpected Shutdown]\"}";
	mqttclient.setWill("geoscope/reply", payload.c_str(), true, LWMQTT_QOS1);

	if (!mqttclient.connected()) {
		if (!attemptTimeout)
			attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
		mqttConnect();
	}
	gainLoad();
	mqttNotify("Current gain: " + String(amplifierGain, 3));
}

void mqttShutdown() {
	mqttNotify("Shutting Down Gracefully");
	mqttclient.clearWill();	
	yield();
}

void mqttConnect() {
	// Attempt to connect
	mqttclient.disconnect();
	if (mqttclient.connect(("GEOSCOPE-" + clientId).c_str()))
	{
		delete attemptTimeout;
		attemptTimeout = NULL;
		// Subscribe to topic geoscope/config/gain
		mqttclient.subscribe("geoscope/config/gain", LWMQTT_QOS1);
		mqttclient.subscribe("geoscope/restart",LWMQTT_QOS1);
		mqttclient.subscribe(CONFIG_TOPIC_PREFIX + "#", LWMQTT_QOS1);
	}
	else if (MQTT_BROKER_TIMEOUT && attemptTimeout && millis() > *attemptTimeout) {
		//initiates self reset if we're requiring Broker for operation (can be turned off during testing),
		// 	and there's a set attemptTimeout
		mqttNotify("Device Self Reset");
		forceReset();
	}
}

void mqttReportGain(float newGain) {
		payload = payloadHeader;
		payload += "\"[Set new gain to "+ String(newGain, 3) +"]\"}";
			//only 3 decimal places needed between two smallest steps
		mqttclient.publish("geoscope/reply", payload, false, LWMQTT_QOS1);
		minYield(10);
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
		payload.reserve(payloadHeader.length() + RAW_COL_BUFFER_SIZE*5 + 20);
		//reduce memory fragmentation by pre-reserving the string buffer
		payload = payloadHeader + "[";
		for (int i = 0; i < RAW_COL_BUFFER_SIZE; i++) {
			payload += String(rawBuffer[buffer_row][i]) + ",";
		}
		payload.remove(payload.length()-1); //trash that last ','
		payload += "],\"gain\":" + String(amplifierGain, 3) + "}";

		if (!mqttclient.connected()) {
			if (!attemptTimeout)
				attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
			mqttConnect();
		}

		mqttclient.publish(MQTT_TOPIC, payload, false, LWMQTT_QOS0);
	}

	mqttclient.loop();
	yield();
}

void mqttOnMessage(String & topic, String & in_payload) {
	if (topic.equalsIgnoreCase("geoscope/config/gain")) {
		samplingDisable();
		// Set new amplifier gain value
		amplifierGain = in_payload.toFloat();
		changeAmplifierGain(amplifierGain);
		mqttReportGain(amplifierGain);
		samplingEnable();
	}
	else if (topic.equalsIgnoreCase("geoscope/restart")) {
		samplingDisable();
		mqttNotify("MQTT Initiated Restart");
		mqttShutdown();
		forceReset();
	}
	else if (topic.equalsIgnoreCase("geoscope/hb")) {
		mqttNotify("GEOSCOPE-" + clientId + "working");
	}
	else if (topic.substring(0,CONFIG_TOPIC_PREFIX.length()).equalsIgnoreCase(CONFIG_TOPIC_PREFIX)) {
		cli.feedString(topic.substring(CONFIG_TOPIC_PREFIX.length()) + " " + in_payload);
		//feed the input + payload as a command to the cli (for reconfiguring)
	}
}

void mqttLoad() {
	Dir storage = LittleFS.openDir("/config/mqtt");
	while (storage.next()) {
		if (storage.isFile()) {
			File f = storage.openFile("r");
			if (f) {
				String temp;
				switch (f.name()[0]) {
					case 'i': //IP
						temp = f.readString();
						temp.trim();
						strncpy(MQTT_BROKER_IP, temp.c_str(), CHAR_BUF_SIZE);
						MQTT_BROKER_IP[CHAR_BUF_SIZE-1] = 0;
						break;
					case 'p': //port
						MQTT_BROKER_PORT = f.parseInt();
						break;
					case 't': //Timeout
						if (GEOSCOPE_IP.isSet()) //don't load timeouts on DHCP
							MQTT_BROKER_TIMEOUT = f.parseInt();
						break;
					case 'c': //ClientID
						temp = f.readString();
						temp.trim();
						clientId = temp;
						break;
					default:
						//this file doesn't contain a config we use
						break;
				}
			}
		}
	}
}

void mqttSave() {
	File storage = LittleFS.open("/config/mqtt/ip", "w");
	storage.println(MQTT_BROKER_IP);
	storage.close();

	storage = LittleFS.open("/config/mqtt/port","w");
	storage.println(MQTT_BROKER_PORT);
	storage.close();

	storage = LittleFS.open("/config/mqtt/timeout","w");
	storage.println(MQTT_BROKER_TIMEOUT);
	storage.close();

	storage = LittleFS.open("/config/mqtt/clientid","w");
	storage.println(clientId);
	storage.close();
}
