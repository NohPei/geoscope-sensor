#include<slip/Stream.h>
#include<PacketListener.h>
#include<ClockControl.h>
#include<cstdlib>
#include<esp_undocumented.h>

packetio::SLIPStream timeStream = packetio::SLIPStream(Serial);
packetio::PacketListener timeListener = packetio::PacketListener(timeStream);
LSLRClockController realTime();
volatile timestamp_t last_time_from_server = 0;
volatile ts_pair_t* new_sample = NULL;

void IRAM_CACHE_ATTR pps_isr_handler() {
	timestamp_t system_ts = ESP_WDEV_TIMESTAMP();
	if (last_time_from_server != 0) {
		timestamp_t server_time = last_time_from_server - (last_time_from_server % 1000000) + 1000000;
		last_time_from_server = 0;
		//disable my interrupt here, too
		new_sample = new ts_pair_t();
		new_sample->local = system_ts;
		new_sample->target = server_time;
	}
}


void clockUpdate(byte* packet, size_t len) {
	packet[len] = '\0'; //force a null terminator after the end of the packet
	last_time_from_server = strtoull((char*) packet, NULL, 16);

	//arm the PPS interrupt now
	
}

void setup() {
	//set up RBIS as master
	//set up an LSLRClockController to manage our local time

	Serial.begin(256000);
	timeListener.onMessage(&clockUpdate);


}


void loop() {
	timeListener.handle();

	if (new_sample) { //check to see if we have a new timestamp pair ready
		realTime.addTimeSample(new_sample.local, new_sample.target); //if so, tell the clock about it
		delete new_sample; //and clear it so we're ready for the next one
		new_sample = NULL;
	}

}
