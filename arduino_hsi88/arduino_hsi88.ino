#define PAYLOAD_BUFFER_SIZE 32
#define CHECK_PERIOD 1000

#define S88_LOAD 8
#define S88_DATA 9
#define S88_CLK 7

//#define DEBUG


// each module has up to 16 sensor inputs
typedef struct feedback_module {
  uint8_t address;
  uint8_t high;   // upper byte
  uint8_t low;    // lower byte
} FeedbackModule;

const uint8_t num_modules = 3;
FeedbackModule modules[num_modules];

void setup() {

  pinMode(S88_LOAD, OUTPUT);
  pinMode(S88_CLK, OUTPUT);

  //enable pullup on data pin
  digitalWrite(S88_DATA, HIGH);
  pinMode(S88_DATA, INPUT);
  
  Serial.begin(9600);
  while(!Serial){};

  for(uint8_t i = 0; i < num_modules; i++){
    modules[i].address = i+1;
    modules[i].high = 0xff;
    modules[i].low = 0xff;
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
  byte myDataIn = 0;

  pinMode(S88_CLK, OUTPUT);
  
  for (i=7; i>=0; i--)
  {
    digitalWrite(S88_CLK, HIGH);
    delayMicroseconds(200);
    if (digitalRead(S88_DATA)) {
      myDataIn = myDataIn | (1 << i); // set corresponding bit to 1
    }
    digitalWrite(S88_CLK, LOW);
    delayMicroseconds(200);
  }
  
  return myDataIn;
}

void readModules() {

  #ifdef DEBUG
  Serial.println("Reading S88");
  #endif
  
  s88_load();
  for(uint8_t i = 0; i < num_modules; i++){
    #ifdef DEBUG
    Serial.print("Reading sensor: ");
    Serial.println(i);
    #endif
    
    modules[i].low = shiftIn();
    modules[i].high = shiftIn();
    
  }
  
}

void publishData(){
  Serial.write('i');
  Serial.write(num_modules); // number of modules
  for(int i=0; i<num_modules; i++){
    Serial.write(modules[i].address); // module number
    Serial.write(modules[i].high); // high byte
    Serial.write(modules[i].low); // low byte
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
    
    if(cmd == 'v'){ // version command
      Serial.write("hsi88-arduino ", HEX);
      Serial.write('\0');
      Serial.write('\r');
    }

    
    if(cmd == 's'){ // init and register, return number of connected modules, collect data and return states
      Serial.write('s');
      Serial.write(num_modules);
      Serial.write('\r');

      readModules();
      publishData();
    }
    
  }

  uint16_t current = millis();
  if((current - last) > 1000){
    readModules();
    publishData();
  }
  
}
