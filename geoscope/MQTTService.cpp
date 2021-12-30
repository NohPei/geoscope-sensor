//
//
//


#include "MQTTService.h"

#define MQTT_MAX_PACKET_SIZE 15000
#define MQTT_MAX_TRANSFER_SIZE WIFICLIENT_MAX_PACKET_SIZE
#include <PubSubClient.h>
//TODO: should we move to a more standard MQTT library? If so, when? Either beween cycles or after second cycle.

#include <string.h>
#include <strings.h>
#include "Network.h"
#include "cli.h"

//TODO: Should we switch the MQTT message format to MessagePack or similar?

char MQTT_BROKER_IP[CHAR_BUF_SIZE] = "192.168.60.100";
int MQTT_BROKER_PORT = 18884;
int MQTT_BROKER_TIMEOUT = 30000; //timeout without reaching the broker before rebooting the sensor

String clientId = "UNNAMED";
String 	MQTT_TOPIC,
	payloadHeader,
	payload,
	CONFIG_TOPIC_PREFIX;

WiFiClient wifiClient;
PubSubClient mqttclient(wifiClient);

unsigned long* attemptTimeout = NULL;

float amplifierGain = 100;

void mqttNotify(String message) {
	payload = payloadHeader;
	payload += "\"[" + message + "]\"}";
	mqttclient.publish("geoscope/reply", payload.c_str());
	yield();
}

void mqttMessageHandler(char* topic, byte* payload, unsigned int len) {
	if (strcasecmp_P(topic, PSTR("geoscope/config/gain")) == 0) {
		samplingDisable();
		// Set new amplifier gain value
		payload[len-1] = '\0'; //ensures there's a null terminator
		amplifierGain = atof((char*)payload);
		changeAmplifierGain(amplifierGain);
		mqttReportGain(amplifierGain);
		samplingEnable();
	}
	else if (strcasecmp_P(topic, PSTR("geoscope/restart")) == 0) {
		samplingDisable();
		mqttNotify("MQTT Initiated Restart");
		mqttShutdown();
		forceReset();
	}
	else if (strcasecmp_P(topic, PSTR("geoscope/hb")) == 0) {
		mqttNotify("GEOSCOPE-" + clientId + "working");
	}
	else if (strncasecmp(topic, CONFIG_TOPIC_PREFIX.c_str(), CONFIG_TOPIC_PREFIX.length()) == 0) {
		char cmdbuf[CHAR_BUF_SIZE];
		strncpy(cmdbuf, topic+CONFIG_TOPIC_PREFIX.length(), CHAR_BUF_SIZE);
		strcat(cmdbuf, " ");
		strncat(cmdbuf, (char*)payload, len);

		cli.feedString(cmdbuf);
		//feed the input + payload as a command to the cli (for reconfiguring)
	}
}



void mqttSetup() {
	MQTT_TOPIC = "geoscope/node1/" + clientId;
	CONFIG_TOPIC_PREFIX = "geoscope/nodeconfig/" + clientId + "/";
	//TODO: leave manual JSON until we swtich the msgpack. Prep to do this beween farrowing cycles.
	payloadHeader = "{\"uuid\":\"GEOSCOPE-" + clientId + "\",\"data\":";
	WiFi.hostname("GEOSCOPE-"+clientId);


	mqttclient.setCallback(mqttMessageHandler);
	mqttclient.setBufferSize(MQTT_MAX_PACKET_SIZE);

	if (!mqttclient.connected()) {
		if (!attemptTimeout)
			attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
		mqttConnect();
	}

	mqttNotify(F("Current gain: ") + String(amplifierGain, 3));
}

void mqttShutdown() {
	mqttNotify(F("Shutting Down Gracefully"));
	yield();
}

void mqttConnect() {
	// Attempt to connect
	mqttclient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
	if (mqttclient.connect(("GEOSCOPE-" + clientId).c_str()))
	{
		delete attemptTimeout;
		attemptTimeout = NULL;
		// Subscribe to topic geoscope/config/gain
		mqttclient.subscribe("geoscope/config/gain", 1);
		mqttclient.subscribe("geoscope/restart", 1);
		mqttclient.subscribe((CONFIG_TOPIC_PREFIX + "#").c_str(), 1);
	}
	else if (MQTT_BROKER_TIMEOUT && attemptTimeout && millis() > *attemptTimeout) {
		//initiates self reset if we're requiring Broker for operation (can be turned off during testing),
		// 	and there's a set attemptTimeout
		mqttNotify(F("Device Self Reset"));
		forceReset();
	}
}

void mqttReportGain(float newGain) {
		mqttNotify(F("Set new gain to ") + String(newGain, 3));
		//only 3 decimal places needed between two smallest steps
}

void mqttSend() {

	if (!mqttclient.connected()) {
		if (!attemptTimeout)
			attemptTimeout = new unsigned long(millis() +  MQTT_BROKER_TIMEOUT);
		mqttConnect();
	}

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

		mqttclient.publish(MQTT_TOPIC.c_str() , payload.c_str());
	}

	mqttclient.loop();
	yield();
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
