#ifndef _RBIS_H_
#define _RBIS_H_
#include "ESPAsyncUDP.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include<stdint.h>
#include "ClockControl.h"

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
	RBIS_UDP(uint16_t udp_port = RBIS_DEFAULT_SYNC_PORT, AsyncUDP udp); //set the udp port and the handler    When initialize, should it process "while(!udp.listen(port))"?
	void set_ClkController(ClockController* clk_addr); // set the clockcontroller address
	void SetUDPhandler(void (*udp_callback)(AsyncUDPPacket));
	//we set it as udp.onPacket(udp_callback) 

protected:
	ClockController* clockcontroller;
	CircularBuffer<ts_indexed_t, RBIS_BUF_SIZE> waiting_timestamps;
	//for both, it stores the local timestamps when receiving a UDP packet
	//for the master, this buffer has timestamps waiting to be sent in a follow-up packet
	//for the client, this buffer has timestamps waiting to be matched to a follow-up packet
	AsyncUDP udp;
	void default_udp_handler(); // what if we set do nothing as a destructor?


}; 

class RBIS_UDP_Slave : public RBIS_UDP {
public:
	RBIS_UDP_Slave(uint16_t udp_port = RBIS_DEFAULT_SYNC_PORT, AsyncUDP udp );
	// udp_callback: read udp packets ,get the time (after converting to Unix time by Clockcontroller) and add it into CircularBuffer
	//the same as the parent class constructor so we will call parent construcor explicitly

	void handelFollowUp(byte[] payload, unsigned int length);
	// handle the payload in the messages of MQTT form the master node ,convert it to ts_index_t type, match them in the CircularBuffer
	//and send them into clockcontroller

	int8_t ClientMatch(ts_indexed_t tsp_mqtt);
	// try matching the timestamp from MQTT in the circularbuffer. If succeed, return the index. Otherwise, return -1
	
private:
	// add something. I haven't come out what to add

	

};

class RBIS_UDP_Master :public RBIS_UDP {
public:
	RBIS_UDP_Master(uint16_t udp_port = RBIS_DEFAULT_SYNC_PORT, AsyncUDP udp));

	void sendFollowUp(byte[] payload, unsigned int length);
	// convert timestamps to byte type payload and send it out through MQTT

	void SetFollowUpSize(uint8_t size); // set the followup size to send out

private:
	uint8_t FollowUpSize= RBIS_DEFAULT_FUP_SIZE; //the size of the publishing frame

	
};