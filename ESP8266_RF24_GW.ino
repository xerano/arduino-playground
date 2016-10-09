#include <ArduinoJson.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "RF24.h"

#define ENDL "\r\n"


template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

byte addresses[][6] = {"1Node","2Node", "3Node", "4Node", "5Node", "6Node"};

RF24 radio(4, 15);

const char* ssid = "*****";
const char* password = "****";
const char* mqtt_server = "mqtt.local";

WiFiClient espClient;
PubSubClient client(espClient);

struct SensorData {
  float temp;
  int vcc;
} sensorData;

char topicName[50];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial << "Connecting to " << ssid << ENDL;

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial << ".";
  }
  Serial << ENDL << "WiFi connected" << ENDL << "IP address: " << WiFi.localIP() << ENDL;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial << "failed, rc=" << client.state() << " try again in 5 seconds" << ENDL;
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

StaticJsonBuffer<200> jsonBuffer;
JsonObject &root = jsonBuffer.createObject();;

void setup() {

  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  radio.begin();
  
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
  radio.openReadingPipe(2,addresses[2]);
  radio.openReadingPipe(3,addresses[3]);
  radio.openReadingPipe(4,addresses[4]);
  radio.openReadingPipe(5,addresses[5]);

  radio.printDetails();
  
  radio.startListening();
}


char buf[256];

void loop() {
  uint8_t pipeNum;
  if( radio.available(&pipeNum)){
    radio.read(&sensorData, sizeof(sensorData));
    Serial << "Received data on pipe " << pipeNum << ENDL;
    Serial << "vcc: " << sensorData.vcc << " temp: " << sensorData.temp << ENDL;

    
    snprintf (topicName, 49, "sensors/temp/%d", pipeNum);
    
    root["vcc"] = sensorData.vcc;
    root["temp"] = sensorData.temp;

    root.printTo(buf, sizeof(buf));

    Serial << "Sending " << buf << " to " << topicName << ENDL;

    if(!client.connected()){
      reconnect();
    }
    
    client.publish(topicName, buf);
  }
  delay(100);
}
