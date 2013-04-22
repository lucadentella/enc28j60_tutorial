#include <EtherCard.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SEND_INTERVAL  30000
#define TIMEOUT        5000
#define ONE_WIRE_BUS   2
#define STATUS_IDLE    0
#define STATUS_SENT    1

static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
byte Ethernet::buffer[700];

char website[] PROGMEM = "www.lucadentella.it";
char password[] PROGMEM = "password";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long previousMillis = 0;
static byte session_id;
byte actual_status;

void setup () {
 
  Serial.begin(57600);
  Serial.println("WebTemperature demo");
  Serial.println();
 
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10)) {
    Serial.println( "Failed to access Ethernet controller");
    while(1); 
  } else Serial.println("Ethernet controller initialized");
 
  if (!ether.dhcpSetup()) {
    Serial.println("Failed to get configuration from DHCP");
    while(1);
  } else Serial.println("DHCP configuration done"); 

  if (!ether.dnsLookup(website)) {
    Serial.print("Unable to resolve Website IP");
    while(1);
  } else Serial.println("Website IP resolved");
  
  Serial.println();
  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.mymask);
  ether.printIp("Gateway:\t", ether.gwip);
  ether.printIp("Website IP:\t", ether.hisip);
  Serial.println();
}
  
void loop() {

  ether.packetLoop(ether.packetReceive());
  unsigned long currentMillis = millis();
  
  switch(actual_status) {
    case STATUS_IDLE: 
      if(currentMillis - previousMillis > SEND_INTERVAL) {
        previousMillis = currentMillis;
        sendTemperature();        
      }
      break;
    case STATUS_SENT:
      if(currentMillis - previousMillis > TIMEOUT) {
        Serial.println("No response");
        previousMillis = currentMillis;
        actual_status = STATUS_IDLE;
      }
      checkResponse();
  }
}   
  
void sendTemperature() {
  
  sensors.requestTemperatures();
  float float_temp = sensors.getTempCByIndex(0);
  
  char string_temp[7];
  dtostrf(float_temp, 4, 2, string_temp);
  
  Stash stash;
  byte sd = stash.create();
  stash.print(string_temp);
  stash.save();

  Stash::prepare(PSTR("GET /demo/saveTemp.php?temp=$H&pwd=$F HTTP/1.0" "\r\n"
    "Host: $F" "\r\n" "\r\n"),
    sd, password, website);
  session_id = ether.tcpSend();
  Serial.print("Temperature ");
  Serial.print(string_temp);
  Serial.print(" sent to website... ");
  actual_status = STATUS_SENT;
}

void checkResponse() {
  
  const char* reply = ether.tcpReply(session_id);
  if(reply > 0) {
    if(strstr(reply, "KO - ") != 0) Serial.println(strstr(reply, "KO - "));
    else Serial.println("OK");
    actual_status = STATUS_IDLE;  
  }
}
