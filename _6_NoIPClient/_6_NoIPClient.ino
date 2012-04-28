#include <EtherCard.h>

#define STATUS_IDLE                   0
#define STATUS_WAITING_FOR_PUBLIC_IP  1
#define STATUS_NOIP_NEEDS_UPDATE      2
#define STATUS_WAITING_FOR_NOIP       3
#define STATUS_ERROR                  99

#define CHECK_IP_INTERVAL    60000
#define REQUEST_TIMEOUT      10000
#define MAX_ATTEMPTS         3

static byte mymac[] = {0xDD,0xDD,0xDD,0x00,0x00,0x01};
byte Ethernet::buffer[700];

char getIP_web[] PROGMEM = "www.lucadentella.it";
char noIP_web[] PROGMEM = "dynupdate.no-ip.com";
char noIP_host[] PROGMEM = "enctutorial.no-ip.info";
char noIP_auth[] PROGMEM = "XXXXXXXXXX";

byte getIP_address[4];
byte noIP_address[4];

Stash stash;
static uint32_t check_ip_timer;
static uint32_t request_timer;
byte actual_status;
static byte session_id;
int attempt;


void setup () {

  Serial.begin(57600);
  Serial.println("NoIP Client Demo");
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

  actual_status = STATUS_IDLE;
  attempt = 0;

  if (!ether.dnsLookup(getIP_web)) {
    Serial.print("Unable to resolve IP for ");
    SerialPrint_P(getIP_web);
    actual_status = STATUS_ERROR;
  } 
  else {
    ether.copyIp(getIP_address, ether.hisip);
    SerialPrint_P(getIP_web);
    ether.printIp(" resolved to:\t", ether.hisip);
  }

  if (!ether.dnsLookup(noIP_web)) {
    Serial.print("Unable to resolve IP for ");
    SerialPrint_P(noIP_web);
    actual_status = STATUS_ERROR;
  } 
  else {
    ether.copyIp(noIP_address, ether.hisip);
    SerialPrint_P(noIP_web);
    ether.printIp(" resolved to:\t", ether.hisip);    
  }

  Serial.println();
}

void loop() {

  ether.packetLoop(ether.packetReceive());

  switch(actual_status) {

  case STATUS_IDLE: checkPublicIP(); break;
  case STATUS_WAITING_FOR_PUBLIC_IP: checkPublicIPResponse(); break;
  case STATUS_NOIP_NEEDS_UPDATE: updateNoIP(); break;
  case STATUS_WAITING_FOR_NOIP: checkNoIPResponse(); break;
  }
}

void checkPublicIP() {

  if(millis() > check_ip_timer) {

    Serial.print("Checking public IP... ");

    Stash::prepare(PSTR("GET /demo/getip.php HTTP/1.1" "\r\n"
      "Host: $F" "\r\n" "\r\n"),
    getIP_web);

    ether.copyIp(ether.hisip, getIP_address);
    session_id = ether.tcpSend();
    actual_status = STATUS_WAITING_FOR_PUBLIC_IP;
    request_timer = millis() + REQUEST_TIMEOUT;
    attempt++;
  }
}

void checkPublicIPResponse() {

  String actualIp, dnsIp;

  const char* reply = ether.tcpReply(session_id);

  if(reply != 0) {

    for(int i = 0; i < strlen(reply) - 189; i++) actualIp = actualIp + reply[187 + i];
    Serial.println(actualIp);

    if(!ether.dnsLookup(noIP_host)) {
      Serial.println("Unable to resolve actual IP for ");
      SerialPrint_P(noIP_host);
      Serial.println();
      actual_status = STATUS_IDLE;
      attempt = 0;
      check_ip_timer = millis() + CHECK_IP_INTERVAL;
    } 
    else {

      for(int i = 0; i < 4; i++) {
        dnsIp = dnsIp + String(ether.hisip[i]);
        if(i < 3) dnsIp = dnsIp + ".";
      }
      SerialPrint_P(noIP_host);
      Serial.print(" resolved to ");
      Serial.println(dnsIp);
      
      if(actualIp.compareTo(dnsIp) == 0) {
        Serial.println("No update needed :)");
        actual_status = STATUS_IDLE;
        attempt = 0;
        check_ip_timer = millis() + CHECK_IP_INTERVAL;        
      }
      else {
        Serial.println("Update needed :(");
        actual_status = STATUS_NOIP_NEEDS_UPDATE;
        attempt = 0;
      }
    }    
  } 
  else {

    if(millis() > request_timer) {
      Serial.println(" no response :(");    
      actual_status = STATUS_IDLE;
      if(attempt == MAX_ATTEMPTS) {
        Serial.println("Max number of attempts reached");
        attempt = 0;
        check_ip_timer = millis() + CHECK_IP_INTERVAL;
      }
    }
  }
}

void updateNoIP() {

  Serial.print("Updating NoIP...");

  Stash::prepare(PSTR("GET /nic/update?hostname=$F HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "Authorization: Basic $F" "\r\n"
    "User-Agent: NoIP_Client" "\r\n" "\r\n"),
  noIP_host, noIP_web, noIP_auth);

  ether.copyIp(ether.hisip, noIP_address);
  session_id = ether.tcpSend();
  actual_status = STATUS_WAITING_FOR_NOIP;
  request_timer = millis() + REQUEST_TIMEOUT;
  attempt++;
}

void checkNoIPResponse() {

  const char* reply = ether.tcpReply(session_id);
  boolean done;

  if(reply != 0) {

    if(strstr(reply, "good") != 0) {
      Serial.println(F(" done!"));
      done = true;
    } 
    else if(strstr(reply, "nochg") != 0) {
      Serial.println(F(" no change required!"));
      done = true;
    }
    else if(strstr(reply, "nohost") != 0) Serial.println(F(" host not found :("));
    else if(strstr(reply, "badauth") != 0) Serial.println(F(" invalid username or password :("));
    else if(strstr(reply, "badagent") != 0) Serial.println(F(" agent banned :("));
    else if(strstr(reply, "!donator") != 0) Serial.println(F(" feature not available for specified username :("));
    else if(strstr(reply, "abuse") != 0) Serial.println(F(" username blocked due to abuse :("));
    else Serial.println(F(" generic error :("));
    
    if(done) {   
      actual_status = STATUS_IDLE;
      attempt = 0;
      check_ip_timer = millis() + CHECK_IP_INTERVAL;
    }
  } 
  else {

    if(millis() > request_timer) {
      Serial.println("No response from NoIP");
      if(attempt == MAX_ATTEMPTS) {
        Serial.println("Max number of attempts reached");
        actual_status = STATUS_IDLE;
        attempt = 0;
        check_ip_timer = millis() + CHECK_IP_INTERVAL;
      } 
      else
        actual_status = STATUS_NOIP_NEEDS_UPDATE;
    }
  }
}

void SerialPrint_P(PGM_P str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
}
