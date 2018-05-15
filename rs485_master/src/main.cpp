#include <Arduino.h>
#include <SoftwareSerial.h>

#define RS485Transmit    HIGH
#define RS485Receive     LOW

SoftwareSerial mySerial(10, 11);

void setup() {
    pinMode(3, OUTPUT);
    digitalWrite(3, LOW);
    mySerial.begin(9600);
    Serial.begin(9600);
}

uint64_t last = 0;

char buffer[20];

void loop() {
    uint64_t now = millis();
    if(now - last > 2000){
        Serial.print("Sending: ");
        char *s = ltoa(now, buffer, 10);
        Serial.println(s);
        last = now;
        digitalWrite(3, RS485Transmit);
        mySerial.print(s);
        digitalWrite(3, RS485Receive);
    }
}