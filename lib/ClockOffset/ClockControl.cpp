#include "ClockControl.h"

#include<cmath>

AverageClockController::AverageClockController() {}

timestamp_t AverageClockController::convertTime(timestamp_t local_timestamp) {
	return local_timestamp + this->currentOffset;
}

void AverageClockController::addTimeSample(timestamp_t local_timestamp, timestamp_t target_timestamp) {
	if (this->samples.isFull()) { // if the buffer's full, use an optimized averaging
		this->currentOffset -= this->samples.first()/this->samples.size(); //remove the oldest sample from the average
		timedelta_t new_delta = target_timestamp - local_timestamp;
		this->samples.push(new_delta); //store the new sample
		this->currentOffset += this->samples.last()/this->samples.size(); //add the newest sample to the average
	}
	else { //when the buffer is still filling, recalculate the average each time
		this->samples.push(target_timestamp - local_timestamp); //store the new offset
		this->currentOffset = 0;
		for (int i = 0; i < this->samples.size(); i++) { //recalculate the average from scratch for the current buffer contents
			this->currentOffset += this->samples[i]/this->samples.size();
		}
	}

}


timestamp_t LSLRClockController::convertTime(timestamp_t local_timestamp) {
	return llround(currentSkew * local_timestamp) + this->currentOffset;
}


void LSLRClockController::addTimeSample(timestamp_t local_timestamp, timestamp_t target_timestamp) {
	ts_pair_t new_pair;
	new_pair.local = local_timestamp;
	new_pair.target = target_timestamp;
	if (this->samples.isEmpty()) {
			this->samples.push(new_pair);
			this->sum_x = new_pair.local;
			this->sum_y = new_pair.local;
			this->sum_xx = new_pair.local * new_pair.local;
			this->sum_xy = new_pair.local * new_pair.target;

			//we can't truly do LR with only one sample
			this->currentSkew = 1;
			this->currentOffset = new_pair.local - new_pair.target;
				//choose the only available offset: the offset of this sample

	}
	else {
		if (this->samples.isFull()) {
			ts_pair_t oldest_pair = this->samples.first(); //find the oldest sample
			//remove it from all of the tallies
			this->sum_xx -= oldest_pair.local * oldest_pair.local;
			this->sum_x -= oldest_pair.local;
			this->sum_xy -= oldest_pair.target * oldest_pair.local;
			this->sum_y -= oldest_pair.target;
		}

		this->samples.push(new_pair); //store the new sample
		//update the running tallies
		this->sum_xx += new_pair.local * new_pair.local;
		this->sum_x += new_pair.local;
		this->sum_xy += new_pair.local * new_pair.target;
		this->sum_y += new_pair.target;

		//calculate the new coefficient estimates
		float n = (float)this->samples.size();
		this->currentSkew = (n*this->sum_xy - this->sum_x * this->sum_y) / (n * this->sum_xx - this->sum_x * this->sum_x);
		this->currentOffset = llround(this->sum_y/n - this->currentSkew * this->sum_x/n);
	}


}


