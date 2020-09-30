#include "cli.h"

#include "Watchdog.h"
#include "Network.h"
#include "MQTTService.h"
#include "main.h"

#define CHAR_BUF_SIZE 32

char 	new_ssid[CHAR_BUF_SIZE],
	new_key[CHAR_BUF_SIZE],
	new_gateway[CHAR_BUF_SIZE],
	new_dns[CHAR_BUF_SIZE],
	new_ip[CHAR_BUF_SIZE],
	new_mask[CHAR_BUF_SIZE];

void usage_net() {
	Serial.println(F(
				"command: net\n"
				"configures networking settings\n"
				"------------------------------\n"
				"Usage: net setting <value> OR net subcommand\n"
				"[setting] can be any of: ip, mask, gateway, ssid, psk, dns\n"
				" 	without <value> reads, otherwise sets\n"
				" 	must run `ip commit` after making any changes\n"
				"\n"
				" 	ip: device IP address, for manual IP\n"
				" 		set to 0.0.0.0 to re-enable DHCP\n"
				" 	mask: device subnet mask, for manual IP\n"
				" 	gateway: IP address of default gateway, for manual IP\n"
				" 	dns: IP address of primary DNS server, for manual IP\n"
				" 	ssid: SSID of WiFi network to connect to\n"
				" 	key: Pre-Shared Key for WiFi network\n"
				"[subcommand] can be help, dump, revert, or commit\n"
				" 	help: prints this usage\n"
				" 	print: dump active network config\n"
				" 	revert: reset to active config\n"
				" 	commit: apply changes made to network configuration\n"
				));
}

void cli_net(int argc, char** argv) {
	if (!argc) {
		usage_net();
		return;
	}
	switch (argv[0][0]) {
		default:
			Serial.println(F("Unknown Subcommand"));
		case 'h':
			usage_net();
			break;
		case 'p': //print
			showWifiConfig();
			break;
		case 'c': //commit
			new_ssid[0] && strncpy(SSID, new_ssid, CHAR_BUF_SIZE);
			new_key[0] && strncpy(PASSWORD, new_key, CHAR_BUF_SIZE);
			new_ip[0] && GEOSCOPE_IP.fromString(new_ip);
			new_gateway[0] && GATEWAY_IP.fromString(new_gateway);
			new_mask[0] && NETMASK.fromString(new_mask);
			if (new_dns[0]) { //need special handling for DNS, since it was never configured before
				IPAddress DNS1;
				DNS1.fromString(new_dns);
				WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK, DNS1);
			}
			wifiSetup();
			//clear edits after commit
		case 'r': //revert
			memset(new_ip, 0, CHAR_BUF_SIZE);
			memset(new_dns, 0, CHAR_BUF_SIZE);
			memset(new_gateway, 0, CHAR_BUF_SIZE);
			memset(new_ssid, 0, CHAR_BUF_SIZE);
			memset(new_key, 0, CHAR_BUF_SIZE);
			memset(new_mask, 0, CHAR_BUF_SIZE);
			break;


		case 'i': //ip
			if (argc > 1) {
				if (IPAddress::isValid(argv[1]))
					strncpy(new_ip, argv[1], CHAR_BUF_SIZE);
			}
			else {
				Serial.print("IP: ");
				if (new_ip[0]) {
					Serial.println(new_ip);
				}
				else {
					Serial.println(GEOSCOPE_IP.toString());
				}
			}
			break;
		case 'm': //mask
			if (argc > 1) {
				if (IPAddress::isValid(argv[1]))
					strncpy(new_mask, argv[1], CHAR_BUF_SIZE);
			}
			else {
				Serial.print("NETMASK: ");
				if (new_mask[0]) {
					Serial.println(new_mask);
				}
				else {
					Serial.println(NETMASK.toString());
				}
			}
		case 'g': //gateway
			if (argc > 1) {
				if (IPAddress::isValid(argv[1]))
					strncpy(new_gateway, argv[1], CHAR_BUF_SIZE);
			}
			else {
				Serial.print("GATEWAY IP: ");
				if (new_gateway[0]) {
					Serial.println(new_gateway);
				}
				else {
					Serial.println(GATEWAY_IP.toString());
				}
			}
		case 'd': //dns
			if (argc > 1) {
				if (IPAddress::isValid(argv[1]))
					strncpy(new_dns, argv[1], CHAR_BUF_SIZE);
			}
			else {
				Serial.print("DNS1 IP: ");
				if (new_dns[0]) {
					Serial.println(new_dns);
				}
				else {
					Serial.println(WiFi.dnsIP().toString());
				}
			}
		case 's': //ssid
			if (argc > 1) {
				strncpy(new_ssid, argv[1], CHAR_BUF_SIZE);
			}
			else {
				Serial.print("SSID: ");
				if (new_ssid[0]) {
					Serial.println(new_ssid);
				}
				else {
					Serial.println(SSID);
				}
			}
		case 'k': //key
			if (argc > 1) {
				strncpy(new_key, argv[1], CHAR_BUF_SIZE);
			}
			else {
				Serial.print("Network Key: ");
				if (new_key[0]) {
					Serial.println(new_key);
				}
				else {
					Serial.println(PASSWORD);
				}
			}
			break;


	}


}



void usage_mqtt() {
	Serial.println(F(
				"command: mqtt\n"
				"configures MQTT logging\n"
				"------------------------------\n"
				"Usage: mqtt setting <value> OR net help\n"
				"[setting] can be any of: ip, mask, gateway, ssid, psk, dns\n"
				" 	without <value> reads, otherwise sets\n"
				"\n"
				" 	ip: broker/server IP\n"
				" 	port: broker/server port\n"
				" 	client: device client ID (usually the last octet of the IP address)\n"
				" 		the device hostname will be GEOSCOPE-<client>\n"
				" 		if set to '|', will use the last device IP octet\n"
				"\n"
				" 	help: prints this usage\n"
			));
}

void cli_mqtt(int argc, char** argv) {
	if (!argc) {
		usage_mqtt();
		return;
	}
	switch (argv[0][0]) {
		default:
			Serial.println(F("Unknown Subcommand"));
		case 'h': //help
			usage_mqtt();
			break;
		case 'i': //ip
			if (argc > 1) {
				if (IPAddress::isValid(argv[1])) {
					strncpy(MQTT_BROKER_IP, argv[1], CHAR_BUF_SIZE);
					mqttSetup();
				}

			}
			else {
				Serial.print("Broker IP: ");
				Serial.println(MQTT_BROKER_IP);
			}
			break;

		case 'p': //port
			if (argc > 1) {
				int newPort = atoi(argv[1]);
				if (newPort) {
					MQTT_BROKER_PORT = newPort;
					mqttSetup();
				}
			}
			else {
				Serial.print("Broker Port: ");
				Serial.println(MQTT_BROKER_PORT);
			}
			break;

		case 'c': //client
			if (argc > 1) {
				if (!strcmp(argv[1],"|")) {
					clientId = GEOSCOPE_IP.toString().substring(GEOSCOPE_IP.toString().lastIndexOf('.')+1);
					//extract the last octet (everything after the last '.') and make it the clientId
				}
				else {
					clientId = String(argv[1]);
				}
				mqttSetup();

			}
			else {
				Serial.print("Client ID: ");
				Serial.println(clientId);
			}
			break;


	}

}


void usage_adc() {
	Serial.println(F(
				"command: adc\n"
				"configures signal capture\n"
				"------------------------------\n"
				"Usage: adc setting <value> OR net help\n"
				"[setting] can be any of: ip, mask, gateway, ssid, psk, dns\n"
				" 	without <value> reads, otherwise sets\n"
				"\n"
				" 	gain: amplifier gain\n"
				" 		valid gains are: 0,1,2,5,10,20,50,100\n"
				" 		all other values are equivalent to 100\n"
				"\n"
				" 	dump: starts dumping raw data for testing\n"
				" 		any incoming character will stop the dumping\n"
				" 	help: prints this usage\n"
			));
}

void cli_adc(int argc, char** argv) {
	if (!argc) {
		usage_adc();
		return;
	}

	switch (argv[0][0]) {
		default:
			Serial.println(F("Unknown Subcommand"));
		case 'h': //help
			usage_mqtt();
			break;
		case 'g': //gain
			if (argc > 1) {
				int newGain = atoi(argv[1]);
				if (newGain) {
					amplifierGain = newGain;
					changeAmplifierGain(newGain);
				}
			}
			else {
				Serial.print("Gain: ");
				Serial.println(amplifierGain);
			}
			break;
		case 'd': //dump
			Serial.println(F("Dumping Raw Data to Terminal. Press any key to stop."));
			minYield(2000); //wait a couple seconds for the user to read the info
			dataDump = true; //enables data dumping mode
			break;


	}
}

void cli_reboot(int argc, char** argv) {
	forceReset();
}

void cli_help(int argc, char** argv) {
	Serial.println(F("net\nmqtt\nadc\nreboot\nhelp"));
}

//@method: cliInit
//@desc: configures the CLI interface using the standard Serial interface
bool cliInit() {
	//configure using Serial
	cmdInit(&Serial);
	cmdAdd("net", cli_net);
	cmdAdd("mqtt", cli_mqtt);
	cmdAdd("adc", cli_adc);
	cmdAdd("reboot", cli_reboot);
	cmdAdd("help", cli_help);

	cli_net(1,(char**) "revert"); //clear the new config at start


	return true;
}

