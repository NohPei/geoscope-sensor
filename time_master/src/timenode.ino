#include<slip/Stream.h>
#include<PacketListener.h>
#include<ClockControl.h>
#include<cstdlib>
#include<esp_undocumented.h>
#include<ESP8266Wifi.h>
#include<PubSubClient.h>
#include<RBIS.h>

packetio::SLIPStream timeStream = packetio::SLIPStream(Serial);
packetio::PacketListener timeListener = packetio::PacketListener(timeStream);
LSLRClockController realTime();
RBIS_UDP timeSyncClient();
volatile timestamp_t next_pps_time = 0;
volatile struct ready_sample {
	ts_pair_t sample;
	bool ready;
} new_sample;

const char MQTT_BROKER_IP[] = "10.244.1.1";
const int MQTT_BROKER_PORT = 18884;
const char MQTT_CLIENT_ID[] = "RBIS_Master";
const char WIFI_SSID[] = "The Promised LAN";
const char WIFI_PSK[] = "GoBucks!";

#define PPS_INTERRUPT digitalPinToInterrupt(D0)

void IRAM_CACHE_ATTR pps_isr_handler() {
	timestamp_t system_ts = ESP_WDEV_TIMESTAMP();
	if (next_pps_time != 0) {
		next_pps_time = 0;

		//disable my interrupt, don't need to go again
		detachInterrupt(PPS_INTERRUPT)


		new_sample.sample.local = system_ts;
		new_sample.sample.target = next_pps_time;
		new_sample.ready = true;
	}
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
	packet[len] = '\0'; //force a null terminator after the end of the packet
	next_pps_time = strtoull((char*) packet, NULL, 16);
	next_pps_time -= (next_pps_time % 1000000); //eliminate the subsecond values
	next_pps_time += 1000000; //next pulse comes at the start of the next second

	//arm the PPS interrupt now
	attachInterrupt(PPS_INTERRUPT, pps_isr_handler, RISING);
		//trigger the real time sample on the next PPS rising edge
	
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


}


void loop() {
	timeListener.handle();


	if (new_sample.ready) { //check to see if we have a new timestamp pair ready
		realTime.addTimeSample(new_sample.sample.local, new_sample.sample.target); //if so, tell the clock about it
		new_sample.ready = NULL;
	}

}
