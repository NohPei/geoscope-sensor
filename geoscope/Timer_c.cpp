// 
// 
// 

#include "Timer_c.h"


String timerNow = "";
const char* server_1 = "pool.ntp.org";
const char* server_2 = "time.nist.gov";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, server_2, 25200);

void timeSetup() {
	Serial.println("======================================================================");
	Serial.println("## TIME Setup.");
	Serial.println("> TIME Server Configuration.");
	timeClient.begin();
	configTime(TIMEZONE * 3600, OFFSET, server_1, server_2);
	Serial.println("> TIME Server Connected.");
	Serial.println("----------------------------------------------------------------------");
}

void fetchTime() {
	Serial.println("======================================================================");
	Serial.println("## TIME Fetch.");
	//timerNow = time(nullptr);
	timeClient.update();
	Serial.print("> Timestamp: ");
	timerNow = String(timeClient.getEpochTime(), DEC);
	Serial.println(timerNow);
	Serial.print("> Timestamp: ");
	Serial.println(timeClient.getFormattedTime());
	Serial.println("----------------------------------------------------------------------");
}