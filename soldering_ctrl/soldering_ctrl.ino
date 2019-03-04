#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <EEPROM.h>

//#define DEBUG

#include "PID.h"

#define VOLTS_PER_UNIT 5.0 / 1023.0

#define SENSE_PIN A2
#define PWM_PIN 9
#define INPUT_PIN A0



class Thermocouple {
  public:
    Thermocouple(int value, int pin){
      _value = value;
      _pin = pin;  
    }
    int getTemperature();
  private:
    int _value;
    int _pin;
};

int Thermocouple::getTemperature() {
  int read = analogRead(_pin);
  #ifdef DEBUG
  Serial.print("Read: ");
  Serial.println(read);
  #endif
  float voltage = VOLTS_PER_UNIT * read;
  #ifdef DEBUG
  Serial.print("U: ");
  Serial.println(voltage);
  #endif
  return voltage / (300 * _value/1000000.0);
}

LiquidCrystal_PCF8574 lcd(0x3F);  // set the LCD address to 0x27 for a 16 chars and 2 line display

PID pid(10,0.1,4);
Thermocouple thermocouple(41, SENSE_PIN);

void setup()
{
  pinMode(PWM_PIN, OUTPUT);
  int error;
  lcd.begin(16, 2); // initialize the lcd
  lcd.setBacklight(255);

#ifdef DEBUG
  Serial.begin(57600);
#endif

}

int last = 0;
int setpoint = 0;
char buffer[10];
void loop()
{
  int input_val = analogRead(INPUT_PIN);
  int new_setpoint = map(analogRead(INPUT_PIN), 0, 1023, 0, 406);

  if(new_setpoint + 2 > setpoint || new_setpoint - 2 < setpoint){
    setpoint = new_setpoint;
  }
  
  int now = millis();
  if(now - last > 100){
    int input = thermocouple.getTemperature();
    int output = pid.calculate(setpoint, input);
    analogWrite(PWM_PIN, output);
    //lcd.clear();
    sprintf(buffer, "%3d/%3d", input, setpoint);
    lcd.setCursor(0,0);
    lcd.print(buffer);
    lcd.setCursor(0,1);
    sprintf(buffer, "%3d", output);
    lcd.print(buffer);
    last=now;

  #ifdef DEBUG
    Serial.print("Input: ");
    Serial.print(input);
    Serial.print(" Output: ");
    Serial.println(output);
  #endif
    
  }
} // loop()
