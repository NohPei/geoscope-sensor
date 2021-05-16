//
//
//

#include "Watchdog.h"
#include "logfile.h"
#include "MQTTService.h"

void forceReset() {
	allOutputs.print(timestamp());
	allOutputs.println(F("<< forceReset Triggered! >>"));
	LocalLogs.close(); //gracefully flush the log file
	LittleFS.end(); //and unmount the filesystem
	mqttShutdown(); //and disconnect from the Broker
	ESP.wdtDisable();
	minYield(10); //give a little time for any last wireless processing to finish
	ESP.restart();
}

//produces an unbounded, non-busy minYield using yield() and millis()
void minYield(unsigned long time_ms) {
	unsigned long start_time = millis();
	while (millis() < time_ms + start_time) {
		yield();
	}
}
