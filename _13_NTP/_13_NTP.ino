#include <EtherCard.h>

#define SECONDS_IN_DAY          86400
#define START_YEAR              1900
#define TIME_ZONE               +1
static int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static byte mymac[] = {0x00,0x1A,0x4B,0x38,0x0C,0x5C};
byte Ethernet::buffer[700];

static byte ntpServer[] = {193,204,114,232};
static byte srcPort = 0;

uint32_t timeStamp;
boolean requestSent;

void setup () {
 
  Serial.begin(57600);
  Serial.println("NTP Demo");
 
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
  Serial.println("Press n to send NTP request");
  requestSent = false;
}
 
void loop() {
 
  ether.packetLoop(ether.packetReceive());
  
  if(requestSent && ether.ntpProcessAnswer(&timeStamp, srcPort)) {
    Serial.println("NTP answer received");
    Serial.println();
    Serial.print("Timestamp: ");
    Serial.println(timeStamp);
    Serial.println();
    printDate(timeStamp + 3600 * TIME_ZONE);
    requestSent = false;
  }
  
  if(Serial.available() > 0) {
    int incomingByte = Serial.read();
    if(incomingByte == 'n') {
      ether.ntpRequest(ntpServer, srcPort);
      Serial.println("NTP request sent");
      requestSent = true;
    }
  }
}

void printDate(uint32_t timeStamp) {
  
  unsigned int year = START_YEAR;
  while(1) {
    uint32_t seconds;
    if(isLeapYear(year)) seconds = SECONDS_IN_DAY * 366;
    else seconds = SECONDS_IN_DAY * 365;
    if(timeStamp >= seconds) {
      timeStamp -= seconds;
      year++;
    } else break;
  }
  
  unsigned int month = 0;
  while(1) {    
    uint32_t seconds = SECONDS_IN_DAY * days_in_month[month];
    if(isLeapYear(year) && month == 1) seconds = SECONDS_IN_DAY * 29;
    if(timeStamp >= seconds) {
      timeStamp -= seconds;
      month++;
    } else break;
  }  
  month++;
  
  unsigned int day = 1;
  while(1) {
    if(timeStamp >= SECONDS_IN_DAY) {
      timeStamp -= SECONDS_IN_DAY;
      day++;
    } else break;
  }  

  unsigned int hour = timeStamp / 3600;
  unsigned int minute = (timeStamp - (uint32_t)hour * 3600) / 60;
  unsigned int second = (timeStamp - (uint32_t)hour * 3600) - minute * 60;
  
  Serial.println("Current date and time:");

  if(day < 10) Serial.print("0");
  Serial.print(day);
  Serial.print("/");
  
  if(month < 10) Serial.print("0");
  Serial.print(month);
  Serial.print("/");  
  
  Serial.println(year);
  
  if(hour < 10) Serial.print("0");
  Serial.print(hour);
  Serial.print(":");
  
  if(minute < 10) Serial.print("0");
  Serial.print(minute);
  Serial.print(":");
  
  if(second < 10) Serial.print("0");
  Serial.println(second);
  
  Serial.println();
}

boolean isLeapYear(unsigned int year) {
  
  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}
