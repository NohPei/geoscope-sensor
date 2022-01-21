#ifndef _RBIS_H_
#define _RBIS_H_

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include<IPAddress.h>

typedef timestamp_t uint64_t;

timestamp_t get_system_timestamp() {
	return (* (timestamp_t*) WDEVTSF0_TIME_LO);
};

const char* DEFAULT_TOPIC = "rbis_time";
#define RBIS_DEFAULT_SYNC_PORT 2323

void RBIS_init(IPAddress& mqtt_broker, uint16_t mqtt_port = 18884, uint8_t averaging_size = 100, uint16_t udp_port = RBIS_DEFAULT_SYNC_PORT);
void RBISMaster_init(uint8_t sample_clump_count, uint16_t sample_wait_time);
void RBIS_set_fUp_topic(const char* mqtt_topic);


void RBIS_loop();
void RBISMaster_loop();


#endif //define _RBIS_H_

