#include <EtherCard.h>

static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x02};
byte Ethernet::buffer[700];

const int led_pin = 2;
char* led_password = "SesamE";
boolean led_status;

void setup () {
 
  Serial.begin(57600);
  Serial.println("Protected LED demo");
 
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println( "Failed to access Ethernet controller");
  else
    Serial.println("Ethernet controller initialized");
 
  if (!ether.dhcpSetup())
    Serial.println("Failed to get configuration from DHCP");
  else
    Serial.println("DHCP configuration done");

  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.netmask);
  ether.printIp("Gateway:\t", ether.gwip);
  
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  led_status = false;
}
  
void loop() {
 
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if(pos) {
    
    Serial.println();
    boolean password_valid = true; 
    
    // is it a POST request?
    if(strstr((char *)Ethernet::buffer + pos, "POST /") != 0) {
    
      Serial.println("New POST request!");
      
      // search and verify the password
      char password[20];      
      char* password_position = strstr((char *)Ethernet::buffer + pos, "&pwd=");
      if(password_position != 0) {
        strcpy(password, password_position + 5);
        Serial.print("Found password=");
        Serial.println(password);
        if(strcmp(password, led_password) == 0) Serial.println("Valid password :)");                  
        else {
          Serial.println("Wrong password :(");
          password_valid = false;
        }
      }
      
      // search for ON= or OFF= command
      if(password_valid) {
        
        // OFF command
        if(strstr((char *)Ethernet::buffer + pos, "OFF=") != 0) {
          Serial.println("Performing OFF command");
          digitalWrite(led_pin, LOW);
          led_status = false;
        
        // ON command
        } else if(strstr((char *)Ethernet::buffer + pos, "ON=") != 0) {
          Serial.println("Performing ON command");
          digitalWrite(led_pin, HIGH);
          led_status = true;          
        } else Serial.println("Unknown command :(");
      }
    }      
    
    // Output HTML page        
    BufferFiller bfill = ether.tcpOffset();
    bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\n"
      "Content-Type: text/html\r\nPragma: no-cache\r\n\r\n"
      "<html><head><title>Protected LED</title></head>"
      "<html><body><form method=\"POST\">"));

    // Enable / disable buttons based on the output status
    if(led_status == true) bfill.emit_p(PSTR("<div><button name=\"ON\" disabled>ON</button><button name=\"OFF\">OFF</button></div>"));
    else bfill.emit_p(PSTR("<div><button name=\"ON\">ON</button><button name=\"OFF\" disabled>OFF</button></div>"));

    // A wrong password was entered?
    if(password_valid == true) bfill.emit_p(PSTR("<div><input type=\"password\" name=\"pwd\"></div></form></body></html>"));
    else bfill.emit_p(PSTR("<div><input type=\"password\" name=\"pwd\">Wrong password</div></form></body></html>"));
      
    ether.httpServerReply(bfill.position());
  }
}

