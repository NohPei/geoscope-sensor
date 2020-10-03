// 
// 
//


#include "MQTTService.h"
#include <ArduinoOTA.h>
#include "Network.h"
#include "cli.h"

char MQTT_BROKER_IP[CHAR_BUF_SIZE] = "192.168.60.100";
int MQTT_BROKER_PORT = 18884;


String clientId = "152";
String 	MQTT_TOPIC,
	payloadHeader,
	payload;

WiFiClient wifiClient;
MQTTClient mqttclient(MQTT_PACKAGE_SIZE);

int reAttempCounter = 0;

int amplifierGain = 100;

void mqttSetup() {
	MQTT_TOPIC = "geoscope/node1/" + clientId;
	payloadHeader = "{\"uuid\":\"GEOSCOPE-" + clientId + "\",\"data\":\"[";
	WiFi.hostname("GECOSCOPE-"+clientId);
	ArduinoOTA.setHostname(("GEOSCOPE-"+clientId).c_str());


	mqttclient.begin(MQTT_BROKER_IP, MQTT_BROKER_PORT, wifiClient);
	mqttclient.onMessage(mqttOnMessage);

	if (!mqttclient.connected()) {
		mqttConnect();
	}

	payload = payloadHeader;
	payload += "Device Started]\"}";
	mqttclient.publish("geoscope/reply", payload);

	if (!mqttclient.connected()) {
		mqttConnect();
	}
	gainLoad();
	payload = payloadHeader;
	payload += "Current gain: " + String(amplifierGain) + "]\"}";
	mqttclient.publish("geoscope/reply", payload);
}

void mqttConnect() {
	if (reAttempCounter > 5) {
		payload = payloadHeader;
		payload += "Device Self Reset]\"}";
		mqttclient.publish("geoscope/reply", payload);
		forceReset();
	}
	// Attempt to connect
	mqttclient.disconnect();
	if (mqttclient.connect(("GEOSCOPE-" + clientId).c_str()))
	{
		reAttempCounter = 0;
		// Subscribe to topic geoscope/config/gain
		mqttclient.subscribe("geoscope/config/gain");
		mqttclient.subscribe("geoscope/restart");
	}
	else
	{
		reAttempCounter++;
		minYield(1000);
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

//void mqttInit() {
//}

//void mqttLoad() {
//	Serial.println("======================================================================");
//	Serial.println("## Load MQTT Configuration to EEPROM.");
//	uint16_t addressStart = 512;
//	EEPROM.begin(1024);
//
//	Serial.println("> GET MQTT BROKER IP");
//	EEPROM.get(addressStart, MQTT_BROKER_IP);
//	addressStart += sizeof(MQTT_BROKER_IP);
//	Serial.println("> GET MQTT BROKER PORT");
//	EEPROM.get(addressStart, MQTT_BROKER_PORT);
//	Serial.println("## MQTT Configuration.");
//	Serial.print("> BROKER IP: ");
//	Serial.println(MQTT_BROKER_IP);
//	Serial.print("> BROKER PORT: ");
//	Serial.println(MQTT_BROKER_PORT);
//	EEPROM.end();
//	Serial.println("----------------------------------------------------------------------");
//}
//
//void mqttSave() {
//	Serial.println("======================================================================");
//	Serial.println("## Save MQTT Configuration to EEPROM.");
//	uint16_t addressStart = 512;
//	EEPROM.begin(1024);
//
//	Serial.println("> PUT MQTT BROKER IP");
//	EEPROM.put(addressStart, MQTT_BROKER_IP);
//	addressStart += sizeof(MQTT_BROKER_IP);
//	Serial.println("> PUT MQTT BROKER PORT");
//	EEPROM.put(addressStart, MQTT_BROKER_PORT);
//	Serial.println("> COMMIT");
//	EEPROM.commit();
//	EEPROM.end();
//	Serial.println("----------------------------------------------------------------------");
//}
