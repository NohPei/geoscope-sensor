// 
// 
// 

#include "Watchdog.h"

void forceReset() {
	ESP.wdtDisable();
	delay(10);
	ESP.restart();
}
