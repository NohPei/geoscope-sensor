// 
// 
// 

#include "Network.h"
#include "Watchdog.h"
#include "cli.h"


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
	WiFi.begin(SSID, PASSWORD);
	// while (WiFi.status() != WL_CONNECTED) {
		yield();
	// }
}

void loadWifiConfig() {
	if (!(GEOSCOPE_IP.isSet() && GATEWAY_IP.isSet() && NETMASK.isSet() && strlen(SSID) > 0)) {
		//if there is no current config
		initWifiConfig(); //start with the hard-coded config
	}
	Dir storage = LittleFS.openDir("/config/net");
	IPAddress newDNS;
	while (storage.next()) {
		if (storage.isFile()) {
			File f = storage.openFile("r");
			if (f) {
				String temp = f.readString();
				temp.trim();
				switch (f.name()[0]) {
					case 's': //SSID
						strncpy(SSID, temp.c_str(), CHAR_BUF_SIZE);
						SSID[CHAR_BUF_SIZE-1] = 0;
						break;
					case 'k': //network Key
						strncpy(PASSWORD, temp.c_str(), CHAR_BUF_SIZE);
						PASSWORD[CHAR_BUF_SIZE-1] = 0;
						break;
					case 'i': //IP
						GEOSCOPE_IP.fromString(temp);
						break;
					case 'g': //Gateway
						GATEWAY_IP.fromString(temp);
						break;
					case 'm': //subnet Mask
						NETMASK.fromString(temp);
						break;
					case 'd': //DNS
						newDNS.fromString(temp);
						break;
					default:
						//this file doesn't contain a config we use
						break;
				}
			}
		}
	}

	if (newDNS.isSet()) { //if we loaded a DNS config
		WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK, newDNS); //start using it
	}
}

void saveWifiConfig() {

	File storage = LittleFS.open("/config/net/ssid", "w");
	storage.println(SSID);
	storage.close();

	storage = LittleFS.open("/config/net/key","w");
	storage.println(PASSWORD);
	storage.close();
	
	storage = LittleFS.open("/config/net/ip", "w");
	storage.println(GEOSCOPE_IP);
	storage.close();

	storage = LittleFS.open("/config/net/gateway","w");
	storage.println(GATEWAY_IP);
	storage.close();
	
	storage = LittleFS.open("/config/net/mask","w");
	storage.println(NETMASK);
	storage.close();
	
	if (WiFi.dnsIP().isSet()) {
		storage = LittleFS.open("/config/net/dns","w");
		storage.println(WiFi.dnsIP());
		storage.close();
	}
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
	cli.println("## Active Wifi Configuration.");
	cli.print("SSID: ");
	cli.println(WiFi.SSID());
	cli.print("Key: ");
	cli.println(WiFi.psk());
	cli.print("IP: ");
	cli.println(WiFi.localIP());
	cli.print("Default Gateway: ");
	cli.println(WiFi.gatewayIP());
	cli.print("Subnet Mask: ");
	cli.println(WiFi.subnetMask());
	cli.print("Primary DNS: ");
	cli.println(WiFi.dnsIP());
}
