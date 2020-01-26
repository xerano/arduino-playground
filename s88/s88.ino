
int latchPin = 8;
int dataPin = 9;
int clockPin = 7;

byte switchVar1 = 72;  //01001000

void setup() {
  Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
}

void loop() {

  //Pulse the latch pin:
  //set it to 1 to collect parallel data
  digitalWrite(latchPin,1);
  digitalWrite(clockPin,1);
  //set it to 1 to collect parallel data, wait
  delayMicroseconds(20);
  digitalWrite(clockPin,1);
  delayMicroseconds(20);
  
  //set it to 0 to transmit data serially  
  digitalWrite(latchPin,0);

  switchVar1 = shiftIn(dataPin, clockPin);

  Serial.println(switchVar1, BIN);

  //white space
  Serial.println("-------------------");
  //delay so all these print satements can keep up.
  delay(500);
}

byte shiftIn(int myDataPin, int myClockPin) {
  int i;
  int temp = 0;
  int pinState;
  byte myDataIn = 0;

  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, INPUT);
  //we will be holding the clock pin high 8 times (0,..,7) at the
  //end of each time through the for loop
  
  //at the begining of each loop when we set the clock low, it will
  //be doing the necessary low to high drop to cause the shift
  //register's DataPin to change state based on the value
  //of the next bit in its serial information flow.
  //The register transmits the information about the pins from pin 7 to pin 0
  //so that is why our function counts down
  for (i=7; i>=0; i--)
  {
    digitalWrite(myClockPin, 1);
    delayMicroseconds(0.2);
    temp = digitalRead(myDataPin);
    if (temp) {
      pinState = 1;
      //set the bit to 0 no matter what
      myDataIn = myDataIn | (1 << i);
    }
    else {
      //turn it off -- only necessary for debuging
     //print statement since myDataIn starts as 0
      pinState = 0;
    }

    //Debuging print statements
    //Serial.print(pinState);
    //Serial.print("     ");
    //Serial.println (myDataIn, BIN);

    digitalWrite(myClockPin, 0);

  }
  //debuging print statements whitespace
  //Serial.println();
  //Serial.println(myDataIn, BIN);
  return myDataIn;
}
