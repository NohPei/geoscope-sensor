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
// 	original: https://github.com/espressif/ESP8266_RTOS_SDK/blob/v3.4/components/esp8266/include/esp8266/eagle_soc.h:149:150
#define ESP8266_WDEVTSF0_TIME_LO 0x3ff21004
#define ESP8266_WDEVTSF0_TIME_HI 0x3ff21008

extern char SSID[32], PASSWORD[32];
extern IPAddress  GATEWAY_IP, NETMASK, GEOSCOPE_IP, DNS;

void networkSetup();
void wifiSetup();
void loadWifiConfig();
void saveWifiConfig();
void initWifiConfig();
void showWifiConfig();

#endif

