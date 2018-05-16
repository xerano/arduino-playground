#include <Arduino.h>
#include <SoftwareSerial.h>

#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPIN 2


#define RS485Transmit    HIGH
#define RS485Receive     LOW

SoftwareSerial mySerial(10, 11);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    pinMode(3, OUTPUT);
    digitalWrite(3, LOW);
    mySerial.begin(9600);
    Serial.begin(9600);
    dht.begin();
}

uint64_t last = 0;

char buffer[20];

struct SensorData {
    int hum;
    int temp;
} sensor_data;

void loop() {
    uint64_t now = millis();
    if(now - last > 5000){
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        sensor_data.hum = (int) h * 100;
        sensor_data.temp = (int) t * 100;

        last = now;


        Serial.print("Sending ");
        Serial.print("Temp:");
        Serial.print(sensor_data.temp);
        Serial.print(" Humidity:");
        Serial.println(sensor_data.hum);

        digitalWrite(3, RS485Transmit);
        mySerial.write((uint8_t *) &sensor_data, sizeof(sensor_data));
        digitalWrite(3, RS485Receive);
    }
}
