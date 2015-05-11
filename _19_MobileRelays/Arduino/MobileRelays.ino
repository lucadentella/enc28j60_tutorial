#include <EtherCard.h>
#include <tinyFAT.h>

#define SD_CS_PIN   4
#define ETH_CS_PIN  10
//#define DEBUG_MODE

static byte mymac[] = {0xA0,0xA8,0xCD,0x01,0x02,0x03};
byte Ethernet::buffer[600];
byte res;

static char* defaultPage = "index.htm";

void setup () {
 
  Serial.begin(57600);
  while(!Serial);
  Serial.println(F("Mobile Relays"));

  // Initialize the Ethernet shield
 if (!ether.begin(sizeof Ethernet::buffer, mymac, ETH_CS_PIN)) {
    Serial.println(F("Failed to initialize Ethernet controller"));
    while(1);
  }
  Serial.println(F("Ethernet controller initialized"));
 
  if (!ether.dhcpSetup()) {
    Serial.println(F("Failed to set IP address"));
    while(1);
  }
    
  ether.printIp("Configured IP address:\t", ether.myip);
  Serial.println();
  
  //Initialize the SD card shield
  file.setSSpin(SD_CS_PIN);
  res=file.initFAT();
  if (res != NO_ERROR) {
    Serial.println(F("Failed to initialize SD card"));
    while(1);
  }
  Serial.println(F("SD card initialized"));
  Serial.print(F("Card size: "));
  Serial.print(file.BS.partitionSize);
  Serial.println(F("MB"));
  
  // configure the relay PINs
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
}
  
void loop() {
 
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if(pos) {

    char* request = (char *)Ethernet::buffer + pos;
    
    #ifdef DEBUG_MODE
    Serial.println(F("---------------------------------------- NEW PACKET ----------------------------------------"));
    Serial.println(request);
    Serial.println(F("--------------------------------------------------------------------------------------------"));
    Serial.println();
    #endif
    
    // if the request is for the "default" page
    if(strncmp("GET / ", request, 6) == 0) {
      
      Serial.print(F("Requested default page: "));
      Serial.print(defaultPage);
      
      sendFile(defaultPage);
    }

    // if the request is for the switch page, get the parameters and change the relay status
    else if(strncmp("GET /switch.ino?", request, 16) == 0) {
      
      Serial.print(F("Changing status to relay "));
      
      // the request is switch.ino?<relayId>|<relayStatus
      int relayId = request[16] - '0';
      int relayStatus = request[18] - '0';
      Serial.print(relayId);
      Serial.print(F(", new status: "));
      Serial.println(relayStatus);
      if(relayStatus == 0) digitalWrite(relayId, LOW);
      else digitalWrite(relayId, HIGH);
      
      BufferFiller bfill = ether.tcpOffset();
      bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"
                        "OK"));
      ether.httpServerReply(bfill.position());
      Serial.println(F("Done!"));
    }

    
    // else verify if the request is ok, then extract the requested page
    else if(strncmp("GET /", request, 5) == 0) {
      
      // DOS format only: 8 + 3 chars, add the "." and the terminator "\0"
      char filename[13];
      
      // search for the first space (0x32), that is the end of the name (filename starts at position 5)
      int i = 0;
      while((request[5 + i] != 32) && (i < 12)) {      
        filename[i] = request[5 + i];
        i++;
      }
      filename[i] = '\0';
      
      Serial.print(F("Requested: "));
      Serial.print(filename);
      
      sendFile(filename);
    }
    
    else {
      Serial.println(F("Unknown request"));
      
      BufferFiller bfill = ether.tcpOffset();
      bfill.emit_p(PSTR("HTTP/1.0 400 Bad Request\r\n\r\nBad Request"));
      ether.httpServerReply(bfill.position());      
    }      
  }
}

void sendFile(char* filename) {
      
  // check if the requested file exists on the SD
  if(!file.exists(filename)) {
        
    Serial.println(F(" ...not found :("));
        
    // file does not exists, send 404 error
    BufferFiller bfill = ether.tcpOffset();
    bfill.emit_p(PSTR("HTTP/1.0 404 Not Found\r\n\r\nNot Found"));
    ether.httpServerReply(bfill.position());
  } 
  
  else  {
    
    // file exists, read from the SD card and send it
    res = file.openFile(filename, FILEMODE_BINARY);
    
    // send the ACK
    ether.httpServerReplyAck();
    
    // TinyFAT reads blocks of 512 bytes
    int bytesRead = 512;
    while(bytesRead == 512) {
      
      bytesRead = file.readBinary();
      uint8_t* payloadBuffer = ether.tcpOffset();
      for(int i = 0; i < bytesRead; i++) 
        payloadBuffer[i] = file.buffer[i];
      
      // an entire block was read, we're not at the end of file
      if(bytesRead == 512) ether.httpServerReply_with_flags(bytesRead, TCP_FLAGS_ACK_V);
      else ether.httpServerReply_with_flags(bytesRead, TCP_FLAGS_ACK_V+TCP_FLAGS_FIN_V);
    }
    file.closeFile();
    Serial.println(F(" ...sent :)"));
  }
}
