#include<slip/Stream.h>
#include<PacketListener.h>
#include<ClockControl.h>
#include<cstdlib>
#include<algorithm>
#include<esp_undocumented.h>
#include<ESP8266Wifi.h>
#include<PubSubClient.h>
#include<RBIS.h>

#if (*(uint16_t)*"\0\xff") < 0x0100
#define BIG_ENDIAN
#else
#define LITTLE_ENDIAN
#endif

packetio::SLIPStream timeStream = packetio::SLIPStream(Serial);
packetio::PacketListener timeListener = packetio::PacketListener(timeStream);
LSLRClockController realTime();
RBIS_UDP timeSyncClient();
volatile timestamp_t last_pulse_time;

const char MQTT_BROKER_IP[] = "10.244.1.1";
const int MQTT_BROKER_PORT = 18884;
const char MQTT_CLIENT_ID[] = "RBIS_Master";
const char WIFI_SSID[] = "The Promised LAN";
const char WIFI_PSK[] = "GoBucks!";

#define SYNC_INTERRUPT digitalPinToInterrupt(D0)

void IRAM_CACHE_ATTR sync_pulse_isr() {
	last_pulse_time = ESP_WDEV_TIMESTAMP();
}

WifiClient dataClient();
PubSubClient mqtt(dataClient);

void mqttSetup() {
	mqtt.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
	while (!mqtt.connect(MQTT_CLIENT_ID)) {
		yield();
	}

}

void mqttSendFUp(byte[] msg, size_t len) {
	mqtt.publish("geoscope/rbis", msg, len);
}


void clockUpdate(byte* packet, size_t len) {
	union {
		uint64_t timestamp;
		byte[sizeof(uint64_t)] bytes;
	} byte_converter;
	byte_converter.timestamp = 0UL;
	std::copy(packet, packet+len, bytes);

#ifdef BIG_ENDIAN //incoming data is little endian, so we need to swap it
	std::reverse(bytes, bytes+sizeof(uint64_t));
#endif

	realTime.addTimeSample(last_pulse_time, byte_converter.timestamp);
}

void setup() {
	new_sample.ready = false;

	WiFi.setMode(WIFI_STA);
	WiFi.setPhyMode(WIFI_PHY_MODE_11N);
	WiFi.begin(WIFI_SSID, WIFI_PSK);


	mqttSetup();
	timeSyncClient.masterSetCb(&mqttSendFUp);
	timeSyncClient.masterSetConversion(&(realTime.convertTime));

	Serial.begin(256000);
	timeListener.onMessage(&clockUpdate);

	//arm the PPS interrupt
	attachInterrupt(SYNC_INTERRUPT, sync_pulse_isr, RISING);
		//trigger the real time sample on the next PPS rising edge


}


void loop() {
	timeListener.handle();


}
