#include <SoftwareSerial.h>

#define PAYLOAD_BUFFER_SIZE 32
#define CHECK_PERIOD 1000

#define S88_LOAD 8
#define S88_DATA 9
#define S88_CLK 7

//#define DEBUG

typedef struct sensor {
  uint8_t address;
  uint8_t high;   // upper byte
  uint8_t low;    // lower byte
} Sensor;

const uint8_t num_sensors = 3;
Sensor sensors[num_sensors];

int incomingByte;

void setup() {

  pinMode(S88_LOAD, OUTPUT);
  pinMode(S88_CLK, OUTPUT);

  //enable pullup on data pin
  digitalWrite(S88_DATA, HIGH);
  pinMode(S88_DATA, INPUT);
  
  Serial.begin(9600);
  while(!Serial){};

  for(uint8_t i = 0; i < num_sensors; i++){
    sensors[i].address = i+1;
    sensors[i].high = 0xff;
    sensors[i].low = 0xff;
  }
}

uint16_t last_check = 0;
uint8_t val = 0;

void s88_load(){
  //Pulse the latch pin:
  //set it to 1 to collect parallel data
  digitalWrite(S88_LOAD,HIGH);
  delayMicroseconds(200);
  digitalWrite(S88_CLK,HIGH);
  //set it to 1 to collect parallel data, wait
  delayMicroseconds(200);
  digitalWrite(S88_CLK, LOW);
  delayMicroseconds(600);
  
  //set it to 0 to transmit data serially  
  digitalWrite(S88_LOAD, LOW);

  delayMicroseconds(200);
}

byte shiftIn() {

  #ifdef DEBUG
  Serial.println("SHIFT");
  #endif
  
  int i;
  uint8_t temp = 0;
  uint8_t pinState;
  byte myDataIn = 0;

  pinMode(S88_CLK, OUTPUT);
  
  for (i=7; i>=0; i--)
  {
    digitalWrite(S88_CLK, HIGH);
    delayMicroseconds(200);
    temp = digitalRead(S88_DATA);
    if (temp) {
      pinState = 1;
      myDataIn = myDataIn | (1 << i);
    }
    else {
      pinState = 0;
    }

    
    digitalWrite(S88_CLK, LOW);
    delayMicroseconds(200);
  }
  
  return myDataIn;
}

void readSensors() {

  #ifdef DEBUG
  Serial.println("Reading S88");
  #endif
  
  s88_load();
  for(uint8_t i = 0; i < num_sensors; i++){
    #ifdef DEBUG
    Serial.print("Reading sensor: ");
    Serial.println(i);
    #endif
    
    sensors[i].low = shiftIn();
    sensors[i].high = shiftIn();
    
  }
  
}

void publishData(){
  Serial.write('i');
  Serial.write(num_sensors); // number of modules
  for(int i=0; i<num_sensors; i++){
    Serial.write(sensors[i].address); // module number
    Serial.write(sensors[i].high); // high byte
    Serial.write(sensors[i].low); // low byte
  }
  Serial.write('\r');
}

uint16_t last;

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    uint8_t buffer[PAYLOAD_BUFFER_SIZE];
    uint8_t current;
    for(int i = 0; i < PAYLOAD_BUFFER_SIZE; i++){
      current = Serial.read();
      if(current == '\r')
        break;
    }
    
    if(cmd == 'v'){
      Serial.write("hsi88-arduino ", HEX);
      Serial.write('\0');
      Serial.write('\r');
    }

    // return number of connected modules
    if(cmd == 's'){
      Serial.write('s');
      Serial.write(num_sensors);
      Serial.write('\r');

      readSensors();
      publishData();
    }
    
  }

  uint16_t current = millis();
  if((current - last) > 1000){
    readSensors();
    publishData();
  }
  
}
