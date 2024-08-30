//
//
//

#include "Network.h"
#include "Watchdog.h"
#include "cli.h"
#include "MQTTService.h"


char SSID[CHAR_BUF_SIZE], PASSWORD[CHAR_BUF_SIZE];
IPAddress  GATEWAY_IP, NETMASK, GEOSCOPE_IP, DNS;

void networkSetup() {
	yield();
	WiFi.softAPdisconnect(true);
	WiFi.disconnect();

	WiFi.persistent(false);
	WiFi.setPhyMode(WIFI_PHY_MODE_11N);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true); //we should reconnect if something happens, too

	loadWifiConfig();
	wifiSetup();
}

void setDHCP() {
	GEOSCOPE_IP = IPAddress(0,0,0,0);
	GATEWAY_IP = IPAddress(0,0,0,0);
	NETMASK = IPAddress(0,0,0,0);
	DNS = IPAddress(0,0,0,0);
	MQTT_BROKER_TIMEOUT = 300000;
		//same as for fallbacks. MQTT timeouts are usually too short
		// 	for DHCP to run first, so override it to 5 minutes (300 s)
}

void wifiSetup() {
	yield();
	if (!GEOSCOPE_IP.isSet() || !GATEWAY_IP.isSet() || !NETMASK.isSet())
		setDHCP();
	WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK, DNS);
	WiFi.begin(SSID, PASSWORD);
	yield();
}

void loadWifiConfig() {
	if (!(GEOSCOPE_IP.isSet() && GATEWAY_IP.isSet() && NETMASK.isSet() && strlen(SSID) > 0)) {
		//if there is no current config
		initWifiConfig(); //start with the hard-coded config
	}
	Dir storage = LittleFS.openDir("/config/net");
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
						if (!GEOSCOPE_IP.fromString(temp))
							setDHCP(); //if any of the core config can't be read, force DHCP
						break;
					case 'g': //Gateway
						if (!GATEWAY_IP.fromString(temp))
							setDHCP();
						break;
					case 'm': //subnet Mask
						if (!NETMASK.fromString(temp))
							setDHCP();
						break;
					case 'd': //DNS
						DNS.fromString(temp);
						break;
					default:
						//this file doesn't contain a config we use
						break;
				}
			}
		}
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
	setDHCP();
	WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK);
		//use DHCP by default
}

//TODO: use new StreamLib tee
//TODO: replace with Commander-API built-in ipconfig?
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
