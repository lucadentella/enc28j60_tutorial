#include <UIPEthernet.h>
#include <PubSubClient.h>
#include "DHT.h"

#define CLIENT_ID       "ArduinoMQTT"
#define USERNAME        "my_username"
#define PASSWORD        "my_password"
#define PUB_TOPIC       "my_username/f/temperature"
#define SUB_TOPIC       "my_username/f/control"
#define PUBLISH_DELAY   5000
#define DHTPIN          3
#define DHTTYPE         DHT11
#define RELAY_PIN       6

uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

EthernetClient ethClient;
PubSubClient mqttClient;
DHT dht(DHTPIN, DHTTYPE);

long previousMillis;


void setup() {

  // setup serial communication
  Serial.begin(9600);
  while(!Serial) {};
  Serial.println(F("enc28j60 and Adafruit IO"));
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
  mqttClient.setServer("io.adafruit.com", 1883);
  mqttClient.setCallback(mqttCallback);
  Serial.println(F("MQTT client configured"));

  // setup DHT sensor
  dht.begin();
  Serial.println(F("DHT sensor initialized"));

  // setup relay PIN
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.println(F("Relay PIN configured"));
  mqttConnect();

  Serial.println();
  Serial.println(F("Ready!"));
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

void mqttConnect() {

  while(!mqttClient.connected()) {
    
    if(mqttClient.connect(CLIENT_ID, USERNAME, PASSWORD)) {

      Serial.println(F("MQTT client connected"));
      mqttClient.subscribe(SUB_TOPIC);
      Serial.println(F("Topic subscribed"));
    } else {
      Serial.println(F("Unable to connect, retry in 5 seconds"));
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  if(strncmp((const char*)payload, "ON", 2) == 0) {
    Serial.println("ON message received, turning relay ON");
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    Serial.println("OFF message received, turning relay OFF");
    digitalWrite(RELAY_PIN, LOW);
  }
}

void sendData() {

  char msgBuffer[20];
  
  float t = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(t);
    
  if(!mqttClient.connected()) mqttConnect();
  mqttClient.publish(PUB_TOPIC, dtostrf(t, 6, 2, msgBuffer));
}
