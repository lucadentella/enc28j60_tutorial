#include <EtherCard.h>
static byte mymac[] = {0x00,0x1A,0x4B,0x38,0x0C,0x5C};
static byte targetmac[] = {0x00,0x1A,0x4B,0x38,0x0F,0x5C};
byte Ethernet::buffer[700];
 
void setup () {
 
  Serial.begin(57600);
  Serial.println("WOL Demo");
 
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println( "Failed to access Ethernet controller");
 else
   Serial.println("Ethernet controller initialized");
 
  if (!ether.dhcpSetup())
    Serial.println("Failed to get configuration from DHCP");
  else
    Serial.println("DHCP configuration done");
 
  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.mymask);
  ether.printIp("Gateway:\t", ether.gwip);
  
  Serial.println();
  Serial.println("Press w to send Magic Packet");
}
 
void loop() {
 
  ether.packetLoop(ether.packetReceive());
  
  if(Serial.available() > 0) {
    int incomingByte = Serial.read();
    if(incomingByte == 'w') {
      ether.sendWol(targetmac);
      Serial.println("Magic packet sent");
    }
  }
}
