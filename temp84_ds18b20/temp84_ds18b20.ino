#include <OneWire.h>
#include <DallasTemperature.h>
#include "RF24.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#define CE_PIN 7
#define CSN_PIN 3
#define ONE_WIRE_BUS 10
#define SENSOR_POWER 9

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC
#define SLEEP_CYCLES 16

volatile short sleep_cycles_remaining = SLEEP_CYCLES;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

RF24 radio(CE_PIN, CSN_PIN);

byte address[6] = "2Node";

struct SensorData {
    float temp;
    int vcc;
} sensorData;

long readVcc() {
   adc_enable();
   long result;
   // Read 1.1V reference against Vcc
   #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
   #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
   #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
   #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
   #endif 
   delay(2); // Wait for Vref to settle
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   adc_disable();
   result = 1126400L / result; // Back-calculate Vcc in mV
   return result;
} 

void setup_watchdog(uint8_t prescalar)
{
  prescalar = min(9,prescalar);
  uint8_t wdtcsr = prescalar & 7;
  if ( prescalar & 8 )
    wdtcsr |= _BV(WDP3);

  MCUSR &= ~_BV(WDRF);
  WDTCSR = _BV(WDCE) | _BV(WDE);
  WDTCSR = _BV(WDCE) | wdtcsr | _BV(WDIE);
}

ISR(WDT_vect)
{
  --sleep_cycles_remaining;
}

void do_sleep(void)
{
  sleep_enable();
  sleep_mode();                        // System sleeps here
  sleep_disable();
}

void setup(void)
{
  // Start up the library
  pinMode(SENSOR_POWER, OUTPUT);
  digitalWrite(SENSOR_POWER, LOW);
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  setup_watchdog(WDTO_8S);
  
  radio.begin();
  radio.setAutoAck(1);
  radio.setRetries(15,15);
  radio.openWritingPipe(address);
  radio.setDataRate(RF24_250KBPS);
  radio.powerDown();
}

void loop(void)
{ 
  sensorData.vcc = readVcc();
  digitalWrite(SENSOR_POWER, HIGH);
  delay(5);
  sensors.begin();
  sensors.requestTemperatures(); // Send the command to get temperatures
  sensorData.temp = sensors.getTempCByIndex(0);
  digitalWrite(SENSOR_POWER, LOW);

  radio.powerUp();
  radio.write( &sensorData, sizeof(sensorData) );
  radio.powerDown();

  while( sleep_cycles_remaining ){
    do_sleep();
  }
  sleep_cycles_remaining = SLEEP_CYCLES;
}
