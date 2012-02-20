#include <EtherCard.h>
static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
static byte myip[] = {192,168,1,10};
byte Ethernet::buffer[700];
 
void setup () {
 
  Serial.begin(57600);
  Serial.println("PING Demo");
 
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0)
    Serial.println( "Failed to access Ethernet controller");
 
  if (!ether.staticSetup(myip))
    Serial.println("Failed to set IP address");
}
 
void loop() {
 
  ether.packetLoop(ether.packetReceive());
}
