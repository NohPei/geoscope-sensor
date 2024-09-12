#include "cli.h"

#include "Watchdog.h"
#include "Network.h"
#include "MQTTService.h"
#include "main.h"
#include "ADCModule.h"

// #define TRUE_EQUIV_COUNT 5
// const String TRUE_EQUIV[] = { //strings that will be understood as "true" when setting variables
// 	"yes",
// 	"1",
// 	"true",
// 	"enable",
// 	"on"
// };
//
// #define FALSE_EQUIV_COUNT 5
// const String FALSE_EQUIV[] = { //strings that will be understood as "false" when setting variables
// 	"no",
// 	"0",
// 	"false",
// 	"disable",
// 	"off"
//
// };

//Network Submenu

bool cli_reboot(Commander&);

char 	new_ssid[CHAR_BUF_SIZE],
	new_key[CHAR_BUF_SIZE],
	new_gateway[CHAR_BUF_SIZE],
	new_dns[CHAR_BUF_SIZE],
	new_ip[CHAR_BUF_SIZE],
	new_mask[CHAR_BUF_SIZE];

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
	bool changed = new_ssid[0] && strncpy(SSID, new_ssid, CHAR_BUF_SIZE);
	changed |= new_key[0] && strncpy(PASSWORD, new_key, CHAR_BUF_SIZE);
	changed |= new_ip[0] && GEOSCOPE_IP.fromString(new_ip);
	changed |= new_gateway[0] && GATEWAY_IP.fromString(new_gateway);
	changed |= new_mask[0] && NETMASK.fromString(new_mask);
	new_dns[0] && DNS.fromString(new_dns);


	if (changed) { //if anything vital changed
		wifiSetup(); //reload WiFi config
		saveWifiConfig(); //store the WiFi config in FS
		forceReset(); //restart to apply changes
	}

	WiFi.config(GEOSCOPE_IP, GATEWAY_IP, NETMASK, DNS);
	return net_revert(cmd); //if there were no changes,
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
			cmd.println(new_mask);
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
			cmd.println(new_key);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(PASSWORD);
		}
	}
	return 0;
}

bool inline net_save(Commander &cmd) {
	saveWifiConfig();
	return 0;
}

bool inline net_load(Commander &cmd) {
	loadWifiConfig();
	wifiSetup();
	return 0;
}

bool inline net_dump(Commander &cmd) {
	showWifiConfig();
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
	{"commit", net_commit, "Apply pending changes to Network Config (Requires a Restart)"},
	{"revert", net_revert, "Discard pending changes to Network Config"},
	{"dump", net_dump, "Dump active configuration to Terminal"},
	{"exit", sub_exit, "Return to main prompt"}
};

const uint16_t netCmdCount = sizeof(netCommands);


// MQTT Submenu

char new_mqtt_ip[CHAR_BUF_SIZE];
int new_mqtt_port;
String new_mqtt_client;
int new_mqtt_timeout;

bool mqtt_revert(Commander &cmd) {
	memset(new_mqtt_ip, 0, CHAR_BUF_SIZE);
	new_mqtt_port = 0;
	new_mqtt_client.clear();
	new_mqtt_timeout = -1;
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
	if (new_mqtt_timeout >= 0) {
		MQTT_BROKER_TIMEOUT = new_mqtt_timeout;
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
			cmd.println(new_mqtt_ip);
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
			cmd.println(new_mqtt_port);
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
			cmd.println(new_mqtt_client);
			cmd.println(F("(pending)"));
		}
		else {
			cmd.println(clientId);
		}
	}
	return 0;
}

bool mqtt_tmout(Commander &cmd) {
	int payload;
	if (cmd.getInt(payload) && payload >= 0) {
		new_mqtt_timeout = payload;
	}
	else {
		cmd.rewind();
		cmd.print(F("MQTT Broker Timeout (ms): "));
		if (new_mqtt_timeout >= 0) {
			if (new_mqtt_timeout)
				cmd.println(new_mqtt_timeout);
			else
				cmd.println(F("disabled"));
			cmd.println(F("(pending)"));
		}
		else {
			if (MQTT_BROKER_TIMEOUT)
				cmd.println(MQTT_BROKER_TIMEOUT);
			else
				cmd.println(F("disabled"));
		}
	}
	return 0;
}

bool inline mqtt_save(Commander &cmd) {
	mqttSave();
	return 0;
}

bool inline mqtt_load(Commander &cmd) {
	mqttLoad();
	mqttSetup();
	return 0;
}


const commandList_t mqttCommands[] = {
	{"ip", mqtt_ip, "Broker/Server IP Address"},
	{"port", mqtt_port, "Broker/Server Port (0 will use default)"},
	{"id", mqtt_client, "Device Client ID (0 will use default)"},
	{"timeout", mqtt_tmout, "Timeout for Device Restart without Broker, ms (0 to disable)"},
	{"require", mqtt_tmout, "Alias for `timeout`"},
	{"tm", mqtt_tmout, "Alias for `timeout`"},
	{"client", mqtt_client, "Alias for `id`"},
	{"commit", mqtt_commit, "Apply pending changes to MQTT Config"},
	{"revert", mqtt_revert, "Discard pending changes to MQTT Config"},
	{"save", mqtt_save, "Save configuration to Filesystem"},
	{"load", mqtt_load, "Load configuration from Filesystem"},
	{"exit", sub_exit, "Return to main prompt"}
};

const uint16_t mqttCmdCount = sizeof(mqttCommands);


// ADC Submenu

bool adc_gain(Commander &cmd) {
	float payload;
	if (cmd.getFloat(payload)) {
		amplifierGain = payload;
		changeAmplifierGain(payload);
		mqttReportGain(payload);
	}
	else {
		cmd.rewind();
		cmd.print(F("Gain: "));
		cmd.printf("%.3f\n", amplifierGain);
	}
	return 0;

}

bool adc_rate(Commander &cmd) {
	unsigned int payload;
	if (cmd.getInt(payload)) {
		changeSampleRate(payload);
	}
	else {
		cmd.rewind();
		cmd.print(F("Sample Rate: "));
		cmd.printf("%d Hz\n", sample_rate);
	}
	return 0;

}

bool adc_ratio(Commander &cmd) {
	float payload;
	if (cmd.getFloat(payload)) {
		gainShiftRatio = payload;
		changeAmplifierGain(amplifierGain);
		mqttReportGain(amplifierGain);
	}
	else {
		cmd.rewind();
		cmd.print(F("Gain Shift Ratio (R_pot/R1): "));
		cmd.printf("%f\n", gainShiftRatio);
	}
	return 0;
}

bool adc_min(Commander &cmd) {
	float payload;
	if (cmd.getFloat(payload)) {
		gainMin = payload;
		changeAmplifierGain(amplifierGain);
		mqttReportGain(amplifierGain);
	}
	else {
		cmd.rewind();
		cmd.print(F("Calibrated Minimum Gain: "));
		cmd.printf("%f\n", gainMin);
	}
	return 0;
}


bool adc_val(Commander &cmd) {
	int16_t payload;
	if (cmd.getInt(payload)) {
		setPotValue(payload);
	}
	else {
		cmd.rewind();
		cmd.print(F("Potentiometer set to: "));
		cmd.printf("%d\n", potVal);
	}
	return 0;
}

//TODO: need a better calibration process, doing the math internally. Should be able to just enter the real gain and let the rest get fixed.

bool adc_dump(Commander &cmd) {
	cmd.println(F("Dumping Raw Data to Terminal. Press any key to stop."));
	delay(2000); //wait a couple seconds for the user to read the info
	cmd.startStreaming();
	return 0;
}

const commandList_t adcCommands[] = {
	{"gain", adc_gain, "Get/Set Amplifier gain (rounds to nearest valid value)"},
	{"ratio", adc_ratio, "Get/Set Amplifier gain shifting ration (R_pot/R1)"},
	{"shift", adc_ratio, "Alias for `ratio`"},
	{"resistor", adc_ratio, "Alias for `ratio`"},
	{"res", adc_ratio, "Alias for `ratio`"},
	{"min", adc_min, "Get/Calibrate Minimum Amplifier Gain"},
	{"minimum", adc_min, "Alias for `min`"},
	{"cal", adc_min, "Alias for `min`"},
	{"calibrate", adc_min, "Alias for `min`"},
	{"pos", adc_val, "Get/Set Potentiometer Value directly"},
	{"val", adc_val, "Alias for `pos`"},
	{"rate", adc_rate, "Get/Set Sampling Rate"},
	{"dump", adc_dump, "Start streaming raw ADC data"},
	{"exit", sub_exit, "Return to main prompt"}
};

const uint16_t adcCmdCount = sizeof(adcCommands);


// File System Commands

bool fs_cat(Commander &cmd) {
	String payload;
	if (cmd.getString(payload)) {
		File f = LittleFS.open(payload,"r");
		if (!f) {
			cmd.print(F("Cannot find file \""));
			cmd.print(payload);
			cmd.println(F( "\"" ));
		}
		else {
			cmd.println(f.readString());
		}
	}
	return 0;
}


bool fs_format(Commander &cmd) {
	cmd.println(F("<<Formatting will erase all FS data!>>"));
	String payload;
	if (cmd.getString(payload)) {
		if (payload.equalsIgnoreCase(F("ok"))) {
			cmd.println(F( "> Formatting FS..." ));
			return !LittleFS.format();
			//return 0 on success, 1 otherwise
		}
	}
	cmd.println(F( "To continue, resend as `fs format ok`" ));
	return 0;
}

bool fs_ls(Commander &cmd) {
	String path;
	if (!cmd.getString(path)) {
		path = "/";
	}
	Dir d = LittleFS.openDir(path);
	while (d.next()) {
		cmd.println(d.fileName());
	}
	return 0;
}



const commandList_t fsCommands[] = {
	{"print", fs_cat, "Print file contents to terminal"},
	{"cat", fs_cat, "Alias for `print`"},
	{"ls", fs_ls, "List directory contents"},
	{"format", fs_format, "reformat flash filesystem"},
	{"backup", backup, "Backup all configrations to filesystem"},
	{"restore", restore, "Restore all configiurations from filesystem"},
	{"exit", sub_exit, "Return to main prompt"}
};

const uint16_t fsCmdCount = sizeof(fsCommands);


//Main menu and return functions

bool cli_reboot(Commander &cmd) {
	mqttNotify("CLI Initiated Shutdown");
	mqttShutdown();
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

bool cli_swap(Commander &cmd) {
	Stream* oldAlt = cmd.getAltPort();
	cmd.attachAltPort(cmd.getInputPort());
	cmd.attachInputPort(oldAlt);
	cmd.attachOutputPort(oldAlt);
	return 0;
}

bool cli_fs(Commander &cmd) {
	if (cmd.transferTo(fsCommands, fsCmdCount, "fs")) {
		//exit immediately if a command was found
		sub_exit(cmd);
	}
	return 0;
}


const commandList_t mainCommands[] = {
	{"net", cli_net, "Configure the network connection (changes made are applied by `net commit`)"},
	{"mqtt", cli_mqtt, "Configure broker connection and logging (changes made are applied by `mqtt commit`)"},
	{"adc", cli_adc, "Configure gain and adc debug mode"},
	{"swap", cli_swap, "Switch console input (between local Serial and Telnet)"},
	{"fs", cli_fs, "Filesystem operations"},
	{"reboot", cli_reboot, "Restart this sensor"}
};

const uint16_t mainCmdCount = sizeof(mainCommands);

bool sub_exit(Commander &cmd) {
	cmd.transferBack(mainCommands, mainCmdCount, "CMD");
	return 0;
}

//backs up all configurations to FS
bool backup(Commander &cmd) {
	//ADCModule
	gainSave();
	//Network
	net_save(cmd);
	//MQTTService
	mqtt_save(cmd);
	return 0;
}

//restores all configurations from FS
bool restore(Commander &cmd) {
	//ADCModule
	gainLoad();
	changeAmplifierGain(amplifierGain);
	mqttReportGain(amplifierGain);
	//Network
	net_load(cmd);
	//MQTTService
	mqtt_load(cmd);
	return 0;
}

Commander cli;

//@method: cliInit
//@desc: configures the CLI interface using the standard Serial interface
bool cliInit(Stream &iostream, Stream &altstream) {
	//configure using Serial, start with main command set
	cli.begin(&iostream, mainCommands, mainCmdCount);
	cli.attachAltPort(&altstream);
	cli.echo(true);
	cli.commandPrompt(true);
	cli.copyRepyAlt(true);

	net_revert(cli); //clear pending network configs at start
	mqtt_revert(cli); //clear pending MQTT configs, too

	return true;
}
