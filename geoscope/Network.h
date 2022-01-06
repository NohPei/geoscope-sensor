// Network.h

#ifndef _NETWORK_h
#define _NETWORK_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <IPAddress.h>

//TSF Register Addresses - imported from eagle_soc.h in RTOS SDK
// 	references:
// 	https://github.com/espressif/ESP8266_RTOS_SDK/blob/v3.4/components/esp8266/include/esp8266/eagle_soc.h
// 	https://github.com/espressif/ESP8266_RTOS_SDK/blob/v3.4/components/esp8266/lib/libnet80211_dbg.a (decompiled)
extern "C" {
	extern int64_t WdevTimOffSet;
}

inline int64_t esp8266_sta_tsf_time() {
	return WdevTimOffSet + (*(int64_t*) 0x3ffe9a94) + (*(uint64_t*)0x3ff20c00);
}

inline int64_t esp8266_ap_tsf_time() {
	return ( *(int64_t*) 0x3ffe21048);
}

//TODO: this still isn't getting us the TSF value. Instead, it's just a running count of microseconds


extern char SSID[32], PASSWORD[32];
extern IPAddress  GATEWAY_IP, NETMASK, GEOSCOPE_IP, DNS;

void networkSetup();
void wifiSetup();
void loadWifiConfig();
void saveWifiConfig();
void initWifiConfig();
void showWifiConfig();

#endif

