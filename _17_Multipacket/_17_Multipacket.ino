#include <EtherCard.h>
#include "like.h"

#define PAYLOAD_SIZE 500

static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
byte Ethernet::buffer[700];

void setup () {
 
  Serial.begin(57600);
  Serial.println("Multipacket DEMO");
 
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10)) {
    Serial.println( "Failed to access Ethernet controller");
    while(1);
  }
  else
    Serial.println("Ethernet controller initialized");
 
  if (!ether.dhcpSetup()) {
    Serial.println("Failed to get configuration from DHCP");
    while(1);
  }
  else
    Serial.println("DHCP configuration done");
 
  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.netmask);
  ether.printIp("Gateway:\t", ether.gwip);
}
 
void loop() {
 
  word pos = ether.packetLoop(ether.packetReceive());
  
  // if a valid packet was received
  if(pos) {
    
    Serial.println("---------------------------------------- NEW REQUEST----------------------------------------");
    Serial.println((char *)Ethernet::buffer + pos);
    Serial.println("--------------------------------------------------------------------------------------------");
    Serial.println();    
    
    // send the ACK
    ether.httpServerReplyAck();
    Serial.println(" -> ACK sent");
    
    // send the image
    int like_index = 0;
    int packet_count = 1;
    
    while(1) {
      
      // is the last packet?
      if(like_size - like_index < PAYLOAD_SIZE) {
        int packet_size = like_size - like_index;
        memcpy_P(ether.tcpOffset(), like + like_index, packet_size);
        ether.httpServerReply_with_flags(packet_size, TCP_FLAGS_ACK_V|TCP_FLAGS_FIN_V);
        Serial.println(" -> last packet sent");
        break;  
      }
      
      else {
        memcpy_P(ether.tcpOffset(), like + like_index, PAYLOAD_SIZE);
        ether.httpServerReply_with_flags(PAYLOAD_SIZE, TCP_FLAGS_ACK_V);
        Serial.print(" -> sent packet ");
        Serial.println(packet_count);        
        like_index += PAYLOAD_SIZE;
        packet_count += 1;
      }
    }
    
    Serial.println("--------------------------------------------------------------------------------------------");
    Serial.println();
  }
}
