#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include "ESPAsyncUDP.h"

#define HIGHER 0x3ff21008
#define LOWER 0x3ff21004
int64_t time_last=0;
int64_t time_now=0;
unsigned int UDP_lc_port=5888;
int nowp=0;
unsigned int UDP_rm_port;
int ave=0;
int buf[5];
AsyncUDP udp;

char ssid[]="escher.icc.coop";// Change it!!!
char pwd[]="guffinternet";

void udpcallback(AsyncUDPPacket packet){
  /*Serial.print("UDP数据包来源类型: ");
  Serial.println(packet.isBroadcast() ? "广播数据" : (packet.isMulticast() ? "组播" : "单播"));
  Serial.print("远端地址及端口号: ");
  Serial.print(packet.remoteIP());
  Serial.print(":");
  Serial.println(packet.remotePort());
  Serial.print("目标地址及端口号: ");
  Serial.print(packet.localIP());
  Serial.print(":");
  Serial.println(packet.localPort());
  Serial.print("数据长度: ");
  Serial.println(packet.length());
  Serial.print("数据内容: ");*/
  //Serial.write(packet.data(), packet.length());
  /*Serial.println();
  packet.printf("Got it!");
  UDP_rm_port=packet.remotePort();*/
  time_last=time_now;
  time_now=*(volatile int64_t *)LOWER;
  if (nowp!=5){
  buf[nowp]=int((time_now-time_last)/1000);
  nowp++;
  }
  else {
    nowp=0;
    for (int k=0;k<=4;k++){
      ave=ave+buf[k];
      }
    ave=int(ave/5);
    Serial.println(ave);
    ave=0;
    }
}
void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pwd);
  Serial.println("Connecting...");
  while (WiFi.status()!=WL_CONNECTED){
    Serial.println("waiting");
    }
  
  if (WiFi.status()==WL_CONNECTED){
    Serial.println("Connected!!!");
    Serial.println(WiFi.localIP());
    }
  
 while (!udp.listen(UDP_lc_port)) 
  {
  }
  udp.onPacket(udpcallback);

}
void loop() {
  
  //Serial.print("Higher:");
  //Serial.print(*(volatile unsigned int *)HIGHER);
  //Serial.print(" Lower:");
  //Serial.println(*(volatile int64_t *)LOWER);
  //delay(5000);
  //udp.broadcastTo("Still Alive!",UDP_rm_port);
  // put your main code here, to run repeatedly:

}
