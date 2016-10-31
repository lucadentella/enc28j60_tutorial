#include <UIPEthernet.h>
#include <PubSubClient.h>
#include "DHT.h"

#define CLIENT_ID       "ArduinoMQTT"
#define TOPIC           "temperature"
#define PUBLISH_DELAY   5000
#define DHTPIN          3
#define DHTTYPE         DHT11

uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
IPAddress mqttServer(192,168,1,4);

EthernetClient ethClient;
PubSubClient mqttClient;
DHT dht(DHTPIN, DHTTYPE);

long previousMillis;

void setup() {

  // setup serial communication
  Serial.begin(9600);
  while(!Serial) {};
  Serial.println(F("MQTT Arduino Demo"));
  Serial.println();
  
  // setup ethernet communication using DHCP
  if(Ethernet.begin(mac) == 0) {
    Serial.println(F("Unable to configure Ethernet using DHCP"));
    for(;;);
  }
  Serial.println(F("Ethernet configured via DHCP"));
  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println();

  // setup mqtt client
  mqttClient.setClient(ethClient);
  mqttClient.setServer(mqttServer, 1883);
  Serial.println(F("MQTT client configured"));

  // setup DHT sensor
  dht.begin();
  Serial.println(F("DHT sensor initialized"));

  Serial.println();
  Serial.println(F("Ready to send data"));
  previousMillis = millis();
}

void loop() {

  // it's time to send new data?
  if(millis() - previousMillis > PUBLISH_DELAY) {
    sendData();
    previousMillis = millis();
  }

  mqttClient.loop();
}

void sendData() {

  char msgBuffer[20];
  
  float t = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(t);
    
  if(mqttClient.connect(CLIENT_ID)) {

    mqttClient.publish(TOPIC, dtostrf(t, 6, 2, msgBuffer));
  }
}

