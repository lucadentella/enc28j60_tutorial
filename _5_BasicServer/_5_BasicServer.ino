#include <EtherCard.h>
static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
byte Ethernet::buffer[700];

void setup () {
 
  Serial.begin(57600);
  Serial.println("BasicServer Demo");
 
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
}
  
void loop() {
 
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if(pos) {
    
    Serial.println("---------------------------------------- NEW PACKET ----------------------------------------");
    Serial.println((char *)Ethernet::buffer + pos);
    Serial.println("--------------------------------------------------------------------------------------------");
    Serial.println();
    
    BufferFiller bfill = ether.tcpOffset();
    bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"
                      "<html><body><h1>BasicServer Demo</h1></body></html>"));
    ether.httpServerReply(bfill.position());
  }
}

