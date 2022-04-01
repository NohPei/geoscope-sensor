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

extern char SSID[32], PASSWORD[32];
extern IPAddress  GATEWAY_IP, NETMASK, GEOSCOPE_IP, DNS;

# ifdef ESP8266
//Undocumented ESP8266 registers
#define WDEVTSF0_TIME_LO 			0x3ff21004
#define WDEVTSF0_TIME_HI 			0x3ff21008
#define WDEVTSFSW0_LO 				0x3ff21018
#define WDEVTSFSW0_HI 				0x3ff2101C
#define WDEVTSF0_TIMER_LO 		0x3ff2109c
#define WDEVTSF0_TIMER_HI 		0x3ff210a0
#define WDEVTSF0_TIMER_ENA 		0x3ff21098
#define WDEVTSF0_TIM_EN_MASK 	0x80000000	
#define WDEV_COUNT_REG 				0x3ff21004

#define ESP_WDEV_TIMESTAMP() (* (volatile uint64_t*) WDEVTSF0_TIME_LO)

extern int wDev_MacTimSetFunc(void (*handle)(void));
# endif

# ifdef ESP32

#include <esp_wifi.h>
#define ESP_WDEV_TIMESTAMP() esp_wifi_get_tsf_time((wifi_interface_t)0)

# endif

void networkSetup();
void wifiSetup();
void loadWifiConfig();
void saveWifiConfig();
void initWifiConfig();
void showWifiConfig();

#endif

