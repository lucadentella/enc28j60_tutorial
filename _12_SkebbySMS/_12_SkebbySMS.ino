#include <EtherCard.h>

#define TEXT_BUFFER_SIZE  162

static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
byte Ethernet::buffer[600];
byte session_id;
Stash stash;

char website[] PROGMEM = "gateway.skebby.it";
char username[] = "username";
char password[] = "password";
char sender[]   = "ArduinoSMS";
char sender_num[]   = "xxxxxxxxxxxx";
char recipient[]   = "xxxxxxxxxxxx";

char text_buffer[TEXT_BUFFER_SIZE];
byte buffer_position;

void setup () {
 
  Serial.begin(57600);
  Serial.println("Skebby SMS Demo");
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
  } else ether.printIp("Skebby IP:\t", ether.hisip);

  Serial.println();
  Serial.println("Type 0 followed by your message for 0Cent SMS");
  Serial.println("Type 1 followed by your message for Classic SMS");
  Serial.println();
  
  buffer_position = 0;
}
  
void loop() {

  if (Serial.available() > 0) {
    char incoming_char = Serial.read();
    if(incoming_char == '\n') {
      text_buffer[buffer_position] = '\0';
      boolean send_zero_cent = text_buffer[0] == '0';
      for(int i = 0; i < strlen(text_buffer); i++)
        text_buffer[i] = text_buffer[i+1];
      if(send_zero_cent) send_sms_zerocent(recipient, text_buffer);
      else send_sms_classic(recipient, text_buffer);
      buffer_position = 0;          
    }
    else if(buffer_position < TEXT_BUFFER_SIZE - 2) {
      text_buffer[buffer_position] = incoming_char;
      buffer_position++;
    }
    else {
      text_buffer[0] = incoming_char;
      buffer_position = 1;
    }
  }
 
  
  ether.packetLoop(ether.packetReceive());  
  const char* reply = ether.tcpReply(session_id);
  
  if(reply != 0) {
    if(strstr(reply, "status=failed") != 0) {
      Serial.println("Error sending SMS:");
      char* message_pos = strstr(reply, "&message=");
      Serial.println(message_pos + 9);
    } else Serial.println("SMS sent!");    
    Serial.println();
  }
}

void send_sms_classic(char* recipient, char* text) {
  
  Serial.println("Preparing request for SMS Classic service");
  
  byte sd = stash.create();
  stash.print("method=send_sms_classic&username=");
  stash.print(username);
  stash.print("&password=");
  stash.print(password);
  stash.print("&recipients[]=");
  stash.print(recipient);
  stash.print("&text=");
  stash.print(text);
  stash.print("&sender_string=");
  stash.print(sender);
  stash.save();
  
  Stash::prepare(PSTR("POST /api/send/smseasy/advanced/http.php HTTP/1.1" "\r\n"
    "Host: $F" "\r\n"
    "Content-Type: application/x-www-form-urlencoded" "\r\n"      
    "Content-Length: $D" "\r\n" 
    "User-Agent: Arduino" "\r\n" "\r\n"
    "$H"), website, stash.size(), sd);
  
  session_id = ether.tcpSend();
  Serial.println("Request sent to Skebby SMS Gateway");
}

void send_sms_zerocent(char* recipient, char* text) {
  
  Serial.println("Preparing request for SMS 0Cent service");
  
  byte sd = stash.create();
  stash.print("method=send_sms&username=");
  stash.print(username);
  stash.print("&password=");
  stash.print(password);
  stash.print("&recipients[]=");
  stash.print(recipient);
  stash.print("&text=");
  stash.print(text);
  stash.print("&sender_number=");
  stash.print(sender_num);
  stash.save();
  
  Stash::prepare(PSTR("POST /api/send/smsskebby/advanced/http.php HTTP/1.1" "\r\n"
    "Host: $F" "\r\n"
    "Content-Type: application/x-www-form-urlencoded" "\r\n"      
    "Content-Length: $D" "\r\n" 
    "User-Agent: Arduino" "\r\n" "\r\n"
    "$H"), website, stash.size(), sd);
  
  session_id = ether.tcpSend();
  Serial.println("Request sent to Skebby SMS Gateway");
}
