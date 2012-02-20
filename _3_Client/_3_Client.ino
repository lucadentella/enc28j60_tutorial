#include <EtherCard.h>

static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
char website[] PROGMEM = "www.lucadentella.it";

byte Ethernet::buffer[700];
static uint32_t timer;

static void response_callback (byte status, word off, word len) {
  
  Serial.print((const char*) Ethernet::buffer + off + 207);
} 
 
void setup () {
 
  Serial.begin(57600);
  Serial.println("Client Demo");
  Serial.println();
 
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println( "Failed to access Ethernet controller");
 else
   Serial.println("Ethernet controller initialized");
 Serial.println();
 
  if (!ether.dhcpSetup())
    Serial.println("Failed to get configuration from DHCP");
  else
    Serial.println("DHCP configuration done");
 
  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.mymask);
  ether.printIp("Gateway:\t", ether.gwip);
  Serial.println();
  
  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
  else 
    Serial.println("DNS resolution done");  
  ether.printIp("SRV IP:\t", ether.hisip);
  Serial.println();
}
 
void loop() {
 
  ether.packetLoop(ether.packetReceive());
  
  if (millis() > timer) {
    timer = millis() + 5000;
    ether.browseUrl(PSTR("/demo/"), "aphorisms.php", website, response_callback);
  }
}

