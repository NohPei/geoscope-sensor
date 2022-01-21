#ifndef _CLOCKCTRL_H
#define _CLOCKCTRL_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include<stdint.h>
#include<CircularBuffer.h>
#include"esp_undocumented.h"

typedef uint64_t timestamp_t;
typedef int64_t timedelta_t;

typedef struct {
	timestamp_t local;
	timestamp_t target;
} ts_pair_t;

#ifndef CLOCK_AVERAGE_COUNT
#define CLOCK_AVERAGE_COUNT 50
#endif

timestamp_t get_system_timestamp() {
	return (* (timestamp_t*) WDEVTSF0_TIME_LO);
};


class ClockController
{
	public:
		virtual timestamp_t convertTime(timestamp_t local_timestamp) = 0;
		virtual void addTimeSample(timestamp_t local_timestamp,
				timestamp_t target_timestamp) = 0;
	protected:
		timestamp_t currentOffset = 0;
};

class AverageClockController : public ClockController
{
	public:
		AverageClockController();
		timestamp_t convertTime(timestamp_t local_timestamp);
		void addTimeSample(timestamp_t local_timestamp, timestamp_t target_timestamp);
	private:
		CircularBuffer<timedelta_t, CLOCK_AVERAGE_COUNT> samples;
		
};

class LSLRClockController : public ClockController {
	public:
		LSLRClockController();
		timestamp_t convertTime();
		void addTimeSample(timestamp_t local_timestamp, timestamp_t target_timestamp);
	private:
		CircularBuffer<ts_pair_t, CLOCK_AVERAGE_COUNT> samples;
		float currentSkew = 1.0;
		float sumLocal, sumLocal2, sumTarget, sumProd;


};





#endif //define _CLOCKCTRL_H_
