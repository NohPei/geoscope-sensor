// 
// 
// 

#include "Network.h"
#include "Watchdog.h"

char SSID[CHAR_BUF_SIZE], PASSWORD[CHAR_BUF_SIZE];
IPAddress  GATEWAY_IP, NETMASK, GEOSCOPE_IP;

void networkSetup() {
	loadWifiConfig();
	wifiSetup();
}

void wifiSetup() {
	delay(10);
	WiFi.softAPdisconnect(true);
	WiFi.disconnect();
	WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK);
//	Serial.println(SSID);
	WiFi.begin(SSID, PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}
}

//void loadWifiConfig() {
//	uint16_t addressStart = 0;
//	EEPROM.begin(512);
//
//	EEPROM.get(addressStart, SSID);
//	addressStart += sizeof(SSID);
//	EEPROM.get(addressStart, PASSWORD);
//	addressStart += sizeof(PASSWORD);
//	EEPROM.get(addressStart, GEOSCOPE_IP);
//	addressStart += sizeof(GEOSCOPE_IP);
//	EEPROM.get(addressStart, GATEWAY_IP);
//	addressStart += sizeof(GATEWAY_IP);
//	EEPROM.get(addressStart, NETMASK);
//	EEPROM.end();
//	showWifiConfig();
//}
//
//void saveWifiConfig() {
//	uint16_t addressStart = 0;
//	EEPROM.begin(512);
//
//	EEPROM.put(addressStart, SSID);
//	addressStart += sizeof(SSID);
//	EEPROM.put(addressStart, PASSWORD);
//	addressStart += sizeof(PASSWORD);
//	EEPROM.put(addressStart, GEOSCOPE_IP);
//	addressStart += sizeof(GEOSCOPE_IP);
//	EEPROM.put(addressStart, GATEWAY_IP);
//	addressStart += sizeof(GATEWAY_IP);
//	EEPROM.put(addressStart, NETMASK);
//	EEPROM.commit();
//	EEPROM.end();
//}

void initWifiConfig() {
	strcpy(SSID, "The Promised LAN");
	strcpy(PASSWORD, "GoBucks!");
	GEOSCOPE_IP = IPAddress(10,147,20, DEVICE_IP);
	GATEWAY_IP = IPAddress(10, 147, 20, 1);
	NETMASK = IPAddress(255, 255, 255, 0);
	//showWifiConfig();

	//saveWifiConfig();
}

//void showWifiConfig() {
//	Serial.println("## Wifi Configuration.");
//	Serial.print("> SSID: ");
//	Serial.println(SSID);
//	Serial.print("> PASSWORD: ");
//	Serial.println(PASSWORD);
//	Serial.print("> IP: ");
//	Serial.println(GEOSCOPE_IP.toString());
//	Serial.print("> GATEWAY: ");
//	Serial.println(GATEWAY_IP.toString());
//	Serial.print("> NETMASK: ");
//	Serial.println(NETMASK.toString());
//}
