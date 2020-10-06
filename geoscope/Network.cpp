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
	minYield(10);
	WiFi.softAPdisconnect(true);
	WiFi.disconnect();
	WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK);
		//use gateway as DNS by default
	WiFi.begin(SSID, PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		yield();
	}
}

void loadWifiConfig() {
	File storage = LittleFS.open("config/net", "r");
	if (!storage.isFile()) { //if the config file doesn't exist
		initWifiConfig(); //load the hard-coded config
	}
	IPAddress DNS;
	while (storage.available()) {
		String line = storage.readStringUntil('\n');
		line.trim(); //cut off any trailing whitespace, hopefully including extraneous newlines
		const char* arg = line.substring(line.lastIndexOf(' ')+1).c_str();
		switch (line[0]) {
			case 's': //ssid
				strncpy(SSID, arg, CHAR_BUF_SIZE);
				SSID[CHAR_BUF_SIZE-1] = 0;
				break;
			case 'k': //key
				strncpy(PASSWORD, arg, CHAR_BUF_SIZE);
				PASSWORD[CHAR_BUF_SIZE-1] = 0;
				break;
			case 'i': //IP
				GEOSCOPE_IP.fromString(arg);
				break;
			case 'g': //Gateway
				GATEWAY_IP.fromString(arg);
				break;
			case 'm': //subnet Mask
				NETMASK.fromString(arg);
				break;
			case 'd': //DNS
				DNS.fromString(arg);
				break;
			default:
				//ignore. This line is invalid
				break;

		}
	}
	if (DNS.isSet()) { //configure DNS now if we got one
		WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK, GATEWAY_IP);
	}
	storage.close();
}

void saveWifiConfig() {
	File storage = LittleFS.open("config/net", "w");
	storage.print("s ");
	storage.println(SSID);
	storage.print("k ");
	storage.println(WiFi.psk());
	storage.print("i ");
	storage.println(GEOSCOPE_IP);
	storage.print("g ");
	storage.println(GATEWAY_IP);
	storage.print("m ");
	storage.println(NETMASK);
	storage.print("d ");
	storage.println(WiFi.dnsIP());

	storage.close();
}

void initWifiConfig() {
	strncpy_P(SSID, PSTR("The Promised LAN"), CHAR_BUF_SIZE);
	strncpy_P(PASSWORD, PSTR("GoBucks!"), CHAR_BUF_SIZE);
	GEOSCOPE_IP = IPAddress(10,147,20, DEVICE_IP);
	GATEWAY_IP = IPAddress(10, 147, 20, 1);
	NETMASK = IPAddress(255, 255, 255, 0);
	WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK, GATEWAY_IP);
		//set Gateway as DNS by default
}

void showWifiConfig() {
	Serial.println("## Active Wifi Configuration.");
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());
	Serial.print("Key: ");
	Serial.println(WiFi.psk());
	Serial.print("IP: ");
	Serial.println(WiFi.localIP());
	Serial.print("Default Gateway: ");
	Serial.println(WiFi.gatewayIP());
	Serial.print("Subnet Mask: ");
	Serial.println(WiFi.subnetMask());
	Serial.print("Primary DNS: ");
	Serial.println(WiFi.dnsIP());
}
