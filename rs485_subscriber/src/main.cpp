#include <Arduino.h>
#include <SoftwareSerial.h>

#define RS485Transmit    HIGH
#define RS485Receive     LOW

SoftwareSerial mySerial(10, 11);

void setup() {
    pinMode(3, OUTPUT);
    digitalWrite(3, LOW);
    Serial.begin(9600);
    mySerial.begin(9600);
}

byte buffer[4];

void loop() {
    if(mySerial.available()){
        Serial.print("Received: ");
        String recv = mySerial.readString();
        Serial.println(recv);
    }
}