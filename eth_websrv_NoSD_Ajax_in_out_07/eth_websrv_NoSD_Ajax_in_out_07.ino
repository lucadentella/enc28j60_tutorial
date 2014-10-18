/*--------------------------------------------------------------
  Program:      eth_websrv_NO_SD_Ajax_in_out versione 07
--------------------------------------------------------------*/

#include <SPI.h>
//#include <Ethernet.h>
#include <UIPEthernet.h>
//#include <SD.h>

//PAGINA HTML
#include <avr/pgmspace.h>
const char pagehtm[] PROGMEM =
"<!DOCTYPE html> <html> <head> <title>Arduino Ajax I/O</title> \n"
" <script> strLED1 = \"\"; strLED2 = \"\"; strLED3 = \"\"; strLED4 = \"\"; var LED3_state = 0; var LED4_state = 0;\n"
" function GetArduinoIO() { nocache = \"&nocache=\" + Math.random() * 1000000; var request = new XMLHttpRequest(); request.onreadystatechange = function() {\n"
" if (this.readyState == 4) { if (this.status == 200) { if (this.responseXML != null) {\n"
" if (this.responseXML.getElementsByTagName('LED')[0].childNodes[0].nodeValue === \"checked\") { document.LED_form.LED1.checked = true; }\n"
" else { document.LED_form.LED1.checked = false; }\n" 
" if (this.responseXML.getElementsByTagName('LED')[1].childNodes[0].nodeValue === \"checked\") { document.LED_form.LED2.checked = true; }\n"
" else { document.LED_form.LED2.checked = false; }\n"
" if (this.responseXML.getElementsByTagName('LED')[2].childNodes[0].nodeValue === \"on\") { document.getElementById(\"LED3\").innerHTML = \"LED 3 is ON (D8)\"; LED3_state = 1; }\n"
" else { document.getElementById(\"LED3\").innerHTML = \"LED 3 is OFF (D8)\"; LED3_state = 0; }\n"
" if (this.responseXML.getElementsByTagName('LED')[3].childNodes[0].nodeValue === \"on\") { document.getElementById(\"LED4\").innerHTML = \"LED 4 is ON (D9)\"; LED4_state = 1; }\n"
" else { document.getElementById(\"LED4\").innerHTML = \"LED 4 is OFF (D9)\"; LED4_state = 0; }\n"
"  } } } }\n"
" request.open(\"GET\", \"ajax_inputs\" + strLED1 + strLED2 + strLED3 + strLED4 + nocache, true);\n"
" request.send(null);\n"
//Aumentato il periodo di refresh a 5000 ms per non stressare ENC
" setTimeout('GetArduinoIO()', 5000);\n"
" strLED1 = \"\"; strLED2 = \"\"; strLED3 = \"\"; strLED4 = \"\"; } \n "
" function GetCheck() { if (LED_form.LED1.checked) { strLED1 = \"&LED1=1\"; } else { strLED1 = \"&LED1=0\"; } if (LED_form.LED2.checked) { strLED2 = \"&LED2=1\"; } else { strLED2 = \"&LED2=0\"; } }\n"
" function GetButton1() { if (LED3_state === 1) { LED3_state = 0; strLED3 = \"&LED3=0\"; } else { LED3_state = 1; strLED3 = \"&LED3=1\"; } } \n"
" function GetButton2() { if (LED4_state === 1) { LED4_state = 0; strLED4 = \"&LED4=0\"; } else { LED4_state = 1; strLED4 = \"&LED4=1\"; } }\n"
" </script>\n"
" <style> .IO_box { float: left; margin: 0 20px 20px 0; border: 1px solid blue; padding: 0 5px 0 5px; width: 120px; } h1 { font-size: 120%; color: blue; margin: 0 0 10px 0; } h2 { font-size: 85%; color: #5734E6; margin: 5px 0 5px 0; } p, form, button { font-size: 80%; color: #252525; } .small_text { font-size: 70%; color: #737373; } </style>\n"
" </head> <body onload=\"GetArduinoIO()\"> <h1>Arduino Ajax I/O</h1> <div class=\"IO_box\"> <h2>LEDs Using Checkboxes</h2> <form id=\"check_LEDs\" name=\"LED_form\"> <input type=\"checkbox\" name=\"LED1\" value=\"0\" onclick=\"GetCheck()\" />LED 1 (D6)<br /><br /> <input type=\"checkbox\" name=\"LED2\" value=\"0\" onclick=\"GetCheck()\" />LED 2 (D7)<br /><br /> </form> </div> <div class=\"IO_box\"> <h2>LEDs Using Buttons</h2> <button type=\"button\" id=\"LED3\" onclick=\"GetButton1()\">LED 3 is OFF (D8)</button><br /><br /> <button type=\"button\" id=\"LED4\" onclick=\"GetButton2()\">LED 4 is OFF (D9)</button><br /><br /> <p class=\"small_text\">D10 to D13 used by Ethernet shield</p> </div> </body> </html>\n";


// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ 120 // aumentato da 60 originale

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 200); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80
EthernetClient client;
//File webFile;               // the web page file on the SD card
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer
boolean LED_state[4] = {0}; // stores the states of the LEDs

void setup()
{
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    Serial.begin(9600);       // for debugging
 	
    // LEDs
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    Serial.println(F("Ethernet Test"));
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
}

void loop()
{
 char stato;
// Test memoria
//Serial.println(F("ok loop")); 
//Serial.print(F("SRAM left: "));
//Serial.println(freeRam());

// print your local IP address:
  //Serial.println(Ethernet.localIP());
  
  client = server.available();  // try to get client

    if (client>0) {  // got client?

// Echo client ok su Serial Monitor
        //Serial.println(F("ok client"));
        
        boolean currentLineIsBlank = true;
        while (client.connected()) {
          stato=1;  
          if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // limit the size of the stored received HTTP request
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    // remainder of header follows below, depending on if
                    // web page or XML page is requested
                    // Ajax request - send XML file
                    if (StrContains(HTTP_req, "ajax_inputs")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        SetLEDs();
                        // AGGIUNTO DELAY per non bloccare ENC prima di inviare il file XML
                        delay(10);
                        // send XML file containing input states
                        XML_response(client);
                        //Serial.println("ok XML");
                    }
                    else {  // web page request
                        // send rest of HTTP header
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();

						//CODICE LETTURA PAGINA DA PROGMEM
						printPage(pagehtm);
                    }
					// display received HTTP request on serial port
                    Serial.println(HTTP_req);
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
        stato = 0;
    } // end if (client)
 if (stato == 1) {  
// verifica se client è in stop
 Serial.println(F("NO client stop"));
 }
} //end loop

// checks if received HTTP request is switching on/off LEDs
// also saves the state of the LEDs
void SetLEDs(void)
{
    // LED 1 (pin 6)
    if (StrContains(HTTP_req, "LED1=1")) {
        LED_state[0] = 1;  // save LED state
        digitalWrite(6, HIGH);
    }
    else if (StrContains(HTTP_req, "LED1=0")) {
        LED_state[0] = 0;  // save LED state
        digitalWrite(6, LOW);
    }
    // LED 2 (pin 7)
    if (StrContains(HTTP_req, "LED2=1")) {
        LED_state[1] = 1;  // save LED state
        digitalWrite(7, HIGH);
    }
    else if (StrContains(HTTP_req, "LED2=0")) {
        LED_state[1] = 0;  // save LED state
        digitalWrite(7, LOW);
    }
    // LED 3 (pin 8)
    if (StrContains(HTTP_req, "LED3=1")) {
        LED_state[2] = 1;  // save LED state
        digitalWrite(8, HIGH);
    }
    else if (StrContains(HTTP_req, "LED3=0")) {
        LED_state[2] = 0;  // save LED state
        digitalWrite(8, LOW);
    }
    // LED 4 (pin 9)
    if (StrContains(HTTP_req, "LED4=1")) {
        LED_state[3] = 1;  // save LED state
        digitalWrite(9, HIGH);
    }
    else if (StrContains(HTTP_req, "LED4=0")) {
        LED_state[3] = 0;  // save LED state
        digitalWrite(9, LOW);
    }
}

// send the XML file with analog values, switch status
//  and LED status
void XML_response(EthernetClient cl)
{
 //   int analog_val;            // stores value read from analog inputs
    int count;                 // used by 'for' loops
   // int sw_arr[] = {2, 3, 5};  // pins interfaced to switches
    
    cl.print("<?xml version = \"1.0\" ?>");
    cl.print("<inputs>");
 
    // checkbox LED states
    // LED1
    cl.print("<LED>");
    if (LED_state[0]) {
        cl.print("checked");
    }
    else {
        cl.print("unchecked");
    }
    cl.println("</LED>");
    // LED2
    cl.print("<LED>");
    if (LED_state[1]) {
        cl.print("checked");
    }
    else {
        cl.print("unchecked");
    }
     cl.println("</LED>");
    // button LED states
    // LED3
    cl.print("<LED>");
    if (LED_state[2]) {
        cl.print("on");
    }
    else {
        cl.print("off");
    }
    cl.println("</LED>");
    // LED4
    cl.print("<LED>");
    if (LED_state[3]) {
        cl.print("on");
    }
    else {
        cl.print("off");
    }
    cl.println("</LED>");
    
    cl.print("</inputs>");
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}

void printPage(const char *str)
{
  // copy data out of program memory into local storage, write out in
  // chunks of 32 bytes to avoid extra short TCP/IP packets
  // from webduino library Copyright 2009 Ben Combee, Ran Talbott
  uint8_t buffer[32];
  size_t bufferEnd = 0;

  while (buffer[bufferEnd++] = pgm_read_byte(str++))
  {
    if (bufferEnd == 32)
    {
      //Serial.write(buffer, 32);
      client.write(buffer, 32);
      bufferEnd = 0;
    }
  }

  // write out everything left but trailing NUL
  if (bufferEnd > 1)
    //Serial.write(buffer, bufferEnd - 1);
	    client.write(buffer, bufferEnd - 1);
}

// Verifica memoria RAM
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


