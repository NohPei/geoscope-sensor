#include "ClockControl.h"

AverageClockController::AverageClockController() {}

timestampt_t AverageClockController::convertTime(timestamp_t local_timestamp) {
	timestamp_t r = local_timestamp + currentOffset;

	return r
}

void AverageClockController::addTimeSample(timestamp_t local_timestamp, timestamp_t target_timestamp) {
	if (samples.isFull()) {
		timedelta_t old_delta = samples.shift();
		timedelta_t new_delta = target_timestamp - local_timestamp;
		samples.push(new_delta);
		currentOffset = currentOffset + (new_delta - old_delta) / CLOCK_AVERAGE_COUNT;
	}
	else {
		samples.push(target_timestamp - local_timestamp);
		currentOffset = currentOffset + (target_timestamp - local_timestamp - currentOffset) / samples.size();
		

	}
	
}




timestamp_t LSLRClockController::convertTime(timestamp_t local_timestamp) {
	timestamp_t r = currentSkew * local_timestamp + currentOffset;
	return r

}

void LSLRClockController::addTimeSample(timestamp_t local_timestamp, timestamp_t target_timestamp) {
	ts_pair_t new_pair, old_pair;
	new_pair.local = local_timestamp;
	new_pair.target = target_timestamp;
	if (samples.isEmpty()) {	// It is empty and we cannot do any LR
		samples.push(new_pair);
		x_ave = local_timestamp;
		y_ave = target_timestamp;
		xx = local_timestamp * local_timestamp;
		xy = local_timestamp * target_timestamp;
	}
	else if (samples.isFull()){
		old_pair = samples.shift();
		samples.push(new_pair);
		xx = xx - old_pair.local * old_pair.local + new_pair.local * new_pair.local;
		xy = xy - old_pair.local * old_pair.target + new_pair.local * new_pair.target;
		x_ave=x_ave+(new_pair.local-old_pair.local)/ CLOCK_AVERAGE_COUNT;
		y_ave=y_ave+ (new_pair.target - old_pair.target) / CLOCK_AVERAGE_COUNT;
		currentSkew = (xy - CLOCK_AVERAGE_COUNT * x_ave * y_ave)/(xx- CLOCK_AVERAGE_COUNT * x_ave * x_ave);
		currentOffset = y_ave - currentSkew * x_ave;


	}
	else{	
		samples.push(new_pair);
		xx = xx + new_pair.local * new_pair.local;
		xy = xy + new_pair.local * new_pair.target;
		x_ave = x_ave + (new_pair.local - x_ave) / samples.size();
		y_ave = y_ave + (new_pair.target - y_ave) / samples.size();
		currentSkew = (xy - samples.size() * x_ave * y_ave) / (xx - samples.size() * x_ave * x_ave);
		currentOffset = y_ave - currentSkew * x_ave;

	}

}


