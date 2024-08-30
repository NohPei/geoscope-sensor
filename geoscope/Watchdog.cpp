//
//
//

#include "Watchdog.h"
#include "cli.h"

void forceReset() {
	//TODO: use new StreamLib tee
	cli.println(F("<< forceReset Triggered! >>"));
	ESP.wdtDisable();
	delay(10);
	ESP.restart();
}
