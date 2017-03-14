#include <avr/sleep.h>
#include <avr/interrupt.h>

#define PIR_PIN PB2
#define LED_PIN PA0

volatile int value = 0; 

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void sleep() {
  // enable pin change on pins of port b
  GIMSK |= (1<<PCIE1);
  // listen for pin change on pcint10
  PCMSK1 |= (1<<PCINT10);

  // disable adc and goto sleep
  ADCSRA &= ~(1<<ADEN);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // enable sleel
  sleep_enable();
  
  sei();
  sleep_cpu(); // goto sleep

  cli(); // woke up! disable IR here
  PCMSK1 &= ~(1<<PCINT10);

  sleep_disable();
  ADCSRA |= (1<<ADEN);
  
  sei();
  
}

void loop() {
  sleep();
  
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
}

ISR(PCINT1_vect){
  
}

