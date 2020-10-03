// 
// 
// 

#include "Watchdog.h"
#include "cli.h"

void forceReset() {
	cli.println(F("<< forceReset Triggered! >>"));
	ESP.wdtDisable();
	minYield(10);
	ESP.restart();
}

//produces an unbounded, non-busy minYield using yield() and millis()
void minYield(unsigned long time_ms) {
	unsigned long start_time = millis();
	while (millis() < time_ms + start_time) {
		yield();
	}
}
