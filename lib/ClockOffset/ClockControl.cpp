#include "ClockControl.h"

#include<cmath>

AverageClockController::AverageClockController() {}

timestamp_t AverageClockController::convertTime(timestamp_t local_timestamp) {
	return local_timestamp + this->currentOffset;
}

void AverageClockController::addTimeSample(timestamp_t local_timestamp, timestamp_t target_timestamp) {
	ts_pair_t new_sample;
	new_sample.local = local_timestamp;
	new_sample.target = target_timestamp;
	if (this->samples.isFull()) { // if the buffer's full, use an optimized averaging
		this->running_averages.local -= this->samples.first().local/this->samples.size(); //remove the oldest sample from the averages
		this->running_averages.target -= this->samples.first().target/this->samples.size();
		this->samples.push(new_sample); //store the new sample
		this->running_averages.local += new_sample.local/this->samples.size(); //add the newest sample to the average
		this->running_averages.target += new_sample.target/this->samples.size();
	}
	else if (this->samples.isEmpty()) {
		this->running_averages.local = new_sample.local;
		this->running_averages.target = new_sample.target;
		this->samples.push(new_sample);
	}
	else { //when the buffer is still filling, recalculate the average each time
		this->samples.push(new_sample); //store the new offset

		this->running_averages.target /= this->samples.size(); //first, apply the new divisor (do this first to prevent overflows)
		this->running_averages.local /= this->samples.size();

		this->running_averages.local *= this->samples.size()-1; //then, un-apply the old divisor
		this->running_averages.target *= this->samples.size()-1;

		this->running_averages.local += new_sample.local/this->samples.size(); //finally, add the new averaged sample
		this->running_averages.target += new_sample.target/this->samples.size();
			//this implementation adds an extra divison step, but reduces the likelihood of integer overflow
	}

	this->currentOffset = running_averages.target - running_averages.local;

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
