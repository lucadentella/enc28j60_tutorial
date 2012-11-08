#include <EtherCard.h>

#define RELAY_PIN	2

static byte mymac[]  = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
static byte myip[]   = {192,168,1,2};
byte Ethernet::buffer[700];

char* on  = "ON";
char* off = "OFF";

boolean relayStatus;
char* relayLabel;
char* linkLabel;

void setup () {
 
  Serial.begin(57600);
  Serial.println("WebRelay Demo");

  if(!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println( "Failed to access Ethernet controller");
  else
    Serial.println("Ethernet controller initialized");

  if(!ether.staticSetup(myip))
    Serial.println("Failed to set IP address");

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  
  relayStatus = false;
  relayLabel = off;
  linkLabel = on;
}
  
void loop() {
 
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if(pos) {
        
    if(strstr((char *)Ethernet::buffer + pos, "GET /?ON") != 0) {
      relayStatus = true;
      relayLabel = on;
      linkLabel = off;
    } else if(strstr((char *)Ethernet::buffer + pos, "GET /?OFF") != 0) {
      relayStatus = false;
      relayLabel = off;
      linkLabel = on;
    }
    digitalWrite(RELAY_PIN, relayStatus); 
		
    BufferFiller bfill = ether.tcpOffset();
    bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\n"
      "Content-Type: text/html\r\nPragma: no-cache\r\n\r\n"
      "<html><head><meta name='viewport' content='width=200px'/></head><body>"
      "<div style='position:absolute;width:200px;height:200px;top:50%;left:50%;margin:-100px 0 0 -100px'>"
      "<div style='font:bold 14px verdana;text-align:center'>Relay is $S</div>"
      "<br><div style='text-align:center'>"
      "<a href='/?$S'><img src='http://www.lucadentella.it/files/bt_$S.png'></a>"
      "</div></div></body></html>"
      ), relayLabel, linkLabel, linkLabel);

      ether.httpServerReply(bfill.position());
    }
}
