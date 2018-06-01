#include <Arduino.h>
#include <U8g2lib.h>

#include "DHTesp.h"

DHTesp dht;

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


#define BL D8

U8G2_PCD8544_84X48_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ D2, /* dc=*/ D3, /* reset=*/ D1);						// Nokia 5110 Display


void setup(void) {
  pinMode(BL, OUTPUT);
  u8g2.begin();
  
  dht.setup(D0, DHTesp::DHT11);
  digitalWrite(BL, HIGH);
}

void loop(void) {
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(10, 15);
    u8g2.print(temperature);
    u8g2.setCursor(10, 30);
    u8g2.print(humidity);
  } while ( u8g2.nextPage() );
  //delay(1000);
  
}

