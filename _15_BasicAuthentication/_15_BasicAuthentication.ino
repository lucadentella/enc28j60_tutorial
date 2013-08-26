#include <EtherCard.h>

static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
byte Ethernet::buffer[600];
byte session_id;
Stash stash;

char website[] PROGMEM = "www.lucadentella.it";
char authorization[] PROGMEM = "bHVjYTpNeVMzY3Izdb==";

void setup () {
 
  Serial.begin(57600);
  Serial.println("Basic authentication demo");
  Serial.println();
 
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10)) {
    Serial.println( "Failed to access Ethernet controller");
    while(1); 
  } else Serial.println("Ethernet controller initialized");
  Serial.println();
 
  if (!ether.dhcpSetup()) {
    Serial.println("Failed to get configuration from DHCP");
    while(1);
  } else Serial.println("DHCP configuration done:"); 
 
  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.mymask);
  ether.printIp("Gateway:\t", ether.gwip);
  
  if (!ether.dnsLookup(website)) {
    Serial.println("DNS failed");    
    while(1);
  } else ether.printIp("Website IP:\t", ether.hisip);
  Serial.println();
  
  Stash::prepare(PSTR("GET /demo/secure/ HTTP/1.1" "\r\n"
    "Host: $F" "\r\n" 
    "Authorization: Basic $F" "\r\n"
    "\r\n"), website, authorization);
  
  session_id = ether.tcpSend();
  Serial.println("Request sent");
}
  
void loop() {

  ether.packetLoop(ether.packetReceive());  
  const char* reply = ether.tcpReply(session_id);
  
  if(reply != 0) {
    
    if(strstr(reply, "HTTP/1.1 401") != 0) Serial.println("Authorization required :(");
    else if(strstr(reply, "HTTP/1.1 200") != 0) Serial.println("Access granted! :)");
    Serial.println();
    Serial.println("---------- RAW RESPONSE ----------");
    Serial.println(reply);   
    Serial.println("----------------------------------");    
  }
}
