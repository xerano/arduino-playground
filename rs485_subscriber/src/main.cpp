#include <Arduino.h>
#include <SoftwareSerial.h>

#define RS485Transmit    HIGH
#define RS485Receive     LOW

SoftwareSerial mySerial(10, 11);

struct SensorData {
    int hum;
    int temp;
} sensor_data;

void setup() {
    pinMode(3, OUTPUT);
    digitalWrite(3, LOW);
    Serial.begin(9600);
    mySerial.begin(9600);
}

byte buffer[4];

void loop() {
    if(mySerial.available()){
        while(mySerial.peek() > -1){
            mySerial.readBytes((uint8_t *) &sensor_data, sizeof(sensor_data));
        }
        
        Serial.print("Temp:");
        Serial.print(sensor_data.temp);
        Serial.print(" Humidity:");
        Serial.println(sensor_data.hum);
    }
}
