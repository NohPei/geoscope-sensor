// 
// 
// 

#include "Network.h"

char SSID[32], PASSWORD[32];
IPAddress  GATEWAY_IP, NETMASK, GEOSCOPE_IP;

void networkSetup() {
	loadWifiConfig();
	wifiSetup();
}

void wifiSetup() {
	delay(10);

	Serial.println("======================================================================");
	Serial.println("## Wifi Setup.");
	Serial.println("> Disconnect from current Network.");
	WiFi.disconnect();
	Serial.println("> Wifi Configure.");
	WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK);
	Serial.print("> Start Connecting to ");
	Serial.println(SSID);
	WiFi.begin(SSID, PASSWORD);
	Serial.print("> ");
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.println(".");
	Serial.println("> Wifi Connected.");
	Serial.println("----------------------------------------------------------------------");
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
	Serial.println("======================================================================");
	Serial.println("## Initial Wifi Configuration.");
	strcpy(SSID, "GEOSCOPE");
	strcpy(PASSWORD, "soupgeoscope");
	GEOSCOPE_IP = IPAddress(192,168,60, DEVICE_IP);
	GATEWAY_IP = IPAddress(192, 168, 60, 1);
	NETMASK = IPAddress(255, 255, 255, 0);
	showWifiConfig();
	Serial.println("----------------------------------------------------------------------");

	//saveWifiConfig();
}

void showWifiConfig() {
	Serial.println("## Wifi Configuration.");
	Serial.print("> SSID: ");
	Serial.println(SSID);
	Serial.print("> PASSWORD: ");
	Serial.println(PASSWORD);
	Serial.print("> IP: ");
	Serial.println(GEOSCOPE_IP.toString());
	Serial.print("> GATEWAY: ");
	Serial.println(GATEWAY_IP.toString());
	Serial.print("> NETMASK: ");
	Serial.println(NETMASK.toString());
}