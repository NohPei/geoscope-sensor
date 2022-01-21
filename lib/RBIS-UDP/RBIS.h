#ifndef _RBIS_H_
#define _RBIS_H_

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include<stdint.h>
#include<ClockControl.h>

const char* DEFAULT_TOPIC = "rbis_time";
#define RBIS_DEFAULT_SYNC_PORT 2323
#define RBIS_DEFAULT_FUP_SIZE 5
#ifndef RBIS_BUF_SIZE
#define RBIS_BUF_SIZE 10
#endif

typedef struct {
	uint8_t sync_id;
	timestamp_t timestamp;
} ts_indexed_t;

class RBIS_UDP {
	public:
		RBIS_UDP(uint16_t udp_port = RBIS_DEFAULT_SYNC_PORT);
		void masterSetCb(void (*sendFunc)(byte[],unsigned int)); //sets the callback function for sending follow-up packets
		void masterSetFollowUpSize(uint8_t timestamps);
		void masterSetConversion(timestamp_t (*convFunc)(timestamp_t)); //sets the function for converting system timer values to real timestamps
			//defaults to just passing
			//replace this with the clock controller's conversion to send UNIX time instead
		void handleFollowUp(byte[] payload, unsigned int length); //call this function with a follow-up packet for the client
		void setClientClockControl(ClockController* ctrl); //sets this RBIS instance to be a slave governing a clock

	private:
		static timestamp_t convertPass(timestamp_t in) {
			return in;
		};

		bool master = false; //set to true when masterSetCb is called
		uint8_t followUpSize = RBIS_DEFAULT_FUP_SIZE; //how many timestamps to send in one followUp packet
		ClockController* clientController = NULL;
			//needs to be set for the client, handles time offset calculations through its addTimeSample() method
		CircularBuffer<ts_indexed_t, RBIS_BUF_SIZE> waiting_timestamps;
			//for the master, this buffer has timestamps waiting to be sent in a follow-up packet
			//for the client, this buffer has timestamps waiting to be matched to a follow-up packet
		void (*masterSend)(byte[],unsigned int) = NULL;
			//needs to be set for the master node. Calls a function with the serialized follow-up packet
		timestamp_t (*masterConvert)(timestamp_t) = &convertPass;



		void sendFollowUp(); //called by master nodes when it's time to send a follow-up packet

};


#endif //define _RBIS_H_

