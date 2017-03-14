
//Pin connected to ST_CP of 74HC595
#define LATCH 8
//Pin connected to SH_CP of 74HC595
#define CLK 12
////Pin connected to DS of 74HC595
#define DATA 11

byte data;
byte dataArray[10];

void setup() {
  //set pins to output because they are addressed in the main loop
  pinMode(LATCH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  Serial.begin(9600);

  //
  // e 0 1
  // d 1 2
  // c 2 4
  // b 3 8
  // a 4 16
  // g 5 32
  // f 6 64
  // dp 7 128
  dataArray[0] = 0b10100000;
  dataArray[1] = 0b11110011;
  dataArray[2] = 0b11000100;
  dataArray[3] = 0b11000001;
  dataArray[4] = 0b10010011;
  dataArray[5] = 0b10001001;
  dataArray[6] = 0b10011000;
  dataArray[7] = 0b11100011;
  dataArray[8] = 0b10000000;
  dataArray[9] = 0b10000011;
  
}

void loop() {

  for (int j = 0; j < 10; j++) {
    //load the light sequence you want from array
    data = dataArray[j];
    //ground latchPin and hold low for as long as you are transmitting
    digitalWrite(LATCH, LOW);
    //move 'em out
    shiftOut(DATA, CLK, MSBFIRST, data);
    //return the latch pin high to signal chip that it
    //no longer needs to listen for information
    digitalWrite(LATCH, HIGH);
    delay(300);
  }
}


