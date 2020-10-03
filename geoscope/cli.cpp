#include "cli.h"

#include "Watchdog.h"
#include "Network.h"
#include "MQTTService.h"
#include "main.h"

char 	new_ssid[CHAR_BUF_SIZE],
	new_key[CHAR_BUF_SIZE],
	new_gateway[CHAR_BUF_SIZE],
	new_dns[CHAR_BUF_SIZE],
	new_ip[CHAR_BUF_SIZE],
	new_mask[CHAR_BUF_SIZE];

//Network Submenu

bool net_revert(Commander &cmd) {
	memset(new_ip, 0, CHAR_BUF_SIZE);
	memset(new_dns, 0, CHAR_BUF_SIZE);
	memset(new_gateway, 0, CHAR_BUF_SIZE);
	memset(new_ssid, 0, CHAR_BUF_SIZE);
	memset(new_key, 0, CHAR_BUF_SIZE);
	memset(new_mask, 0, CHAR_BUF_SIZE);
	return 0;
}

bool net_commit(Commander &cmd) {
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
	return net_revert(cmd);
}

bool net_ip(Commander &cmd) {
	String payload;
	if (cmd.getString(payload) && IPAddress::isValid(payload)) {
		strncpy(new_ip, payload.c_str(), CHAR_BUF_SIZE);
		new_ip[CHAR_BUF_SIZE-1] = 0; //ensure null termination
	}
	else {
		cmd.rewind();
		cmd.print(F("IP: "));
		if (new_ip[0]) { //if there's a new pending value
			cmd.println(new_ip);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(GEOSCOPE_IP);
		}
	}
	return 0;
}

bool net_mask(Commander &cmd) {
	String payload;
	if (cmd.getString(payload) && IPAddress::isValid(payload)) {
		strncpy(new_mask, payload.c_str(), CHAR_BUF_SIZE);
		new_mask[CHAR_BUF_SIZE-1] = 0; //ensure null termination
	}
	else {
		cmd.rewind();
		cmd.print(F("Mask: "));
		if (new_mask[0]) {
			cmd.println(new_mask);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(NETMASK);
		}
	}
	return 0;
}

bool net_gateway(Commander &cmd) {
	String payload;
	if (cmd.getString(payload) && IPAddress::isValid(payload)) {
		strncpy(new_gateway, payload.c_str(), CHAR_BUF_SIZE);
		new_gateway[CHAR_BUF_SIZE-1] = 0; //ensure null termination
	}
	else {
		cmd.rewind();
		cmd.print(F("Default Gateway: "));
		if (new_gateway[0]) {
			cmd.println(new_gateway);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(GATEWAY_IP);
		}
	}
	return 0;
}

bool net_dns(Commander &cmd) {
	String payload;
	if (cmd.getString(payload) && IPAddress::isValid(payload)) {
		strncpy(new_dns, payload.c_str(), CHAR_BUF_SIZE);
		new_dns[CHAR_BUF_SIZE-1] = 0; //ensure null termination
	}
	else {
		cmd.rewind();
		cmd.print(F("Primary DNS: "));
		if (new_dns[0]) {
			cmd.println(new_dns);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(WiFi.dnsIP());
		}
	}
	return 0;
}

bool net_ssid(Commander &cmd) {
	String payload;
	if (cmd.getString(payload) && payload.length() < CHAR_BUF_SIZE) {
		strncpy(new_ssid, payload.c_str(), CHAR_BUF_SIZE);
		new_ssid[CHAR_BUF_SIZE-1] = 0; //ensure null termination
	}
	else {
		cmd.rewind();
		cmd.print(F("SSID: "));
		if (new_ssid[0]) {
			cmd.print(new_mask);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(SSID);
		}
	}
	return 0;
}

bool net_psk(Commander &cmd) {
	String payload;
	if (cmd.getString(payload) && payload.length() < CHAR_BUF_SIZE) {
		strncpy(new_key, payload.c_str(), CHAR_BUF_SIZE);
		new_key[CHAR_BUF_SIZE-1] = 0; //ensure null termination
	}
	else {
		cmd.rewind();
		cmd.print(F("Network Key: "));
		if (new_key[0]) {
			cmd.print(new_key);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(PASSWORD);
		}
	}
	return 0;
}

bool sub_exit(Commander &cmd);


const commandList_t netCommands[] = {
	{"ip", net_ip, "Device IP address (set to 0.0.0.0 for DHCP)"},
	{"mask", net_mask, "Device subnet mask"},
	{"gateway", net_gateway, "Device default gateway"},
	{"dns", net_dns, "Primary DNS server address"},
	{"ssid", net_ssid, "SSID of WiFi network to connect to"},
	{"psk", net_psk, "Pre-Shared Key for WiFi network"},
	{"key", net_psk, "Alias for `psk`"},
	{"commit", net_commit, "Apply pending changes to Network Config"},
	{"revert", net_revert, "Discard pending changes to Network Config"},
	{"exit", sub_exit, "Return to main prompt"}
};

const uint16_t netCmdCount = sizeof(netCommands);


// MQTT Submenu

char new_mqtt_ip[CHAR_BUF_SIZE];
int new_mqtt_port;
String new_mqtt_client;

bool mqtt_revert(Commander &cmd) {
	memset(new_mqtt_ip, 0, CHAR_BUF_SIZE);
	new_mqtt_port = 0;
	new_mqtt_client.clear();
	return 0;
}

bool mqtt_commit(Commander &cmd) {
	if (new_mqtt_ip[0]) {
		strncpy(MQTT_BROKER_IP, new_mqtt_ip, CHAR_BUF_SIZE);
	}
	if (new_mqtt_port) {
		MQTT_BROKER_PORT = new_mqtt_port;
	}
	if (!new_mqtt_client.isEmpty()) {
		clientId = new_mqtt_client;
	}
	mqttSetup();
	return mqtt_revert(cmd);
}

bool mqtt_ip(Commander &cmd) {
	String payload;
	if (cmd.getString(payload) && IPAddress::isValid(payload)) {
		strncpy(new_mqtt_ip, payload.c_str(), CHAR_BUF_SIZE);
		new_mqtt_ip[CHAR_BUF_SIZE-1] = 0; //ensure null termination
	}
	else {
		cmd.rewind();
		cmd.print(F("Broker IP: "));
		if (new_mqtt_ip[0]) {
			cmd.print(new_mqtt_ip);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(MQTT_BROKER_IP);
		}
	}
	return 0;
}

bool mqtt_port(Commander &cmd) {
	int payload;
	if (cmd.getInt(payload)) {
		if (payload) //for non-zero payloads
			new_mqtt_port = payload;
		else //for the default port
			new_mqtt_port = 18884;
	}
	else {
		cmd.rewind();
		cmd.print(F("Broker Port: "));
		if (new_mqtt_port) {
			cmd.print(new_mqtt_port);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(MQTT_BROKER_PORT);
		}
	}
	return 0;
}

bool mqtt_client(Commander &cmd) {
	String payload;
	if (cmd.getString(payload)) {
		if (payload != "0")
			new_mqtt_client = payload;
		else {
			new_mqtt_client = String(GEOSCOPE_IP.v4() & 0x000000FF, DEC);
			//mask out just the last octet of the device IP
		}
	}
	else {
		cmd.rewind();
		cmd.print(F("MQTT Client ID: "));
		if (!new_mqtt_client.isEmpty()) {
			cmd.print(new_mqtt_client);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(clientId);
		}
	}
	return 0;
}


const commandList_t mqttCommands[] = {
	{"ip", mqtt_ip, "Broker/Server IP Address"},
	{"port", mqtt_port, "Broker/Server Port (0 will use default)"},
	{"id", mqtt_client, "Device Client ID (0 will use default)"},
	{"client", mqtt_client, "Alias for `id`"},
	{"commit", mqtt_commit, "Apply pending changes to MQTT Config"},
	{"revert", mqtt_revert, "Discard pending changes to MQTT Config"},
	{"exit", sub_exit, "Return to main prompt"}
};

const uint16_t mqttCmdCount = sizeof(mqttCommands);


// ADC Submenu

bool adc_gain(Commander &cmd) {
	int payload;
	if (cmd.getInt(payload)) {
		amplifierGain = payload;
		changeAmplifierGain(payload);
	}
	else {
		cmd.rewind();
		cmd.print(F("Gain: "));
		cmd.println(amplifierGain);
	}
	return 0;

}

bool adc_dump(Commander &cmd) {
	Serial.println(F("Dumping Raw Data to Terminal. Press any key to stop."));
	minYield(2000); //wait a couple seconds for the user to read the info
	cmd.startStreaming();
	return 0;
}

const commandList_t adcCommands[] = {
	{"gain", adc_gain, "Get/Set Amplifier gain (0,1,2,5,10,20,50,100)"},
	{"dump", adc_dump, "Start streaming raw ADC data"},
	{"exit", sub_exit, "Return to main prompt"}
};

const uint16_t adcCmdCount = sizeof(adcCommands);


//Main menu and return functions

bool cli_reboot(Commander &cmd) {
	forceReset();
	return 0;
}

bool cli_net(Commander &cmd) {
	if (cmd.transferTo(netCommands, netCmdCount, "net")) {
		//exit immediately if a command was found
		sub_exit(cmd);
	}
	return 0;
}

bool cli_adc(Commander &cmd) {
	if (cmd.transferTo(adcCommands, adcCmdCount, "adc")) {
		//exit immediately if a command was found
		sub_exit(cmd);
	}
	return 0;
}

bool cli_mqtt(Commander &cmd) {
	if (cmd.transferTo(mqttCommands, mqttCmdCount, "mqtt")) {
		//exit immediately if a command was found
		sub_exit(cmd);
	}
	return 0;
}

const commandList_t mainCommands[] = {
	{"net", cli_net, "Configure the network connection (changes made are applied by `net commit`)"},
	{"mqtt", cli_mqtt, "Configure broker connection and logging (changes made are applied by `mqtt commit`)"},
	{"adc", cli_adc, "Configure gain and adc debug mode"},
	{"reboot", cli_reboot, "Restart this sensor"}
};

const uint16_t mainCmdCount = sizeof(mainCommands);

bool sub_exit(Commander &cmd) {
	cmd.transferBack(mainCommands, mainCmdCount, "Cmd");
	return 0;
}

Commander cli;

//@method: cliInit
//@desc: configures the CLI interface using the standard Serial interface
bool cliInit() {
	//configure using Serial, start with main command set
	cli.begin(&Serial, mainCommands, mainCmdCount);

	net_revert(cli); //clear pending network configs at start
	mqtt_revert(cli); //clear pending MQTT configs, too

	return true;
}
