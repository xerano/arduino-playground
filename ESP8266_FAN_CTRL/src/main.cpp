#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define PWM_30  1024 * 0.3
#define PWM_50  1024 * 0.5
#define PWM_75  1024 * 0.75
#define PWM_100 1024

#define PWM_PIN_0 5
#define PWM_PIN_1 4
#define TACHO_0 0
#define TACHO_1 2
#define ON_OFF 15

const char* PARAM_FAN = "fan";
const char* ssid     = "******";
const char* password = "******";

AsyncWebServer server(80);

volatile int tacho_0_pulses = 0;
volatile int tacho_1_pulses = 0;

int last = 0;
int pin;
int level;
int mode;

void tacho_0_Pulse();
void tacho_1_Pulse();
void setupWifi();

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(57600);

  pinMode(PWM_PIN_0, OUTPUT);
  pinMode(PWM_PIN_1, OUTPUT);

  analogWrite(PWM_PIN_0, PWM_30);
  analogWrite(PWM_PIN_1, PWM_30);

  pinMode(ON_OFF, OUTPUT);
  
  pinMode(TACHO_0, INPUT_PULLUP);
  pinMode(TACHO_1, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(TACHO_0), tacho_0_Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(TACHO_1), tacho_1_Pulse, FALLING);

  // 25 kHz PWM for fan control
  // TODO check fans data sheet
  analogWriteFreq(25000);

  setupWifi();
}

int pulses[2];

void loop() {
  int now = millis();

  if(now - last > 2000){
    last = now;
    pulses[0] = tacho_0_pulses;
    pulses[1] = tacho_1_pulses;
    tacho_0_pulses = 0;
    tacho_1_pulses = 0;
    Serial.println("--------------");
    Serial.print("Pulses: ");
    Serial.print(pulses[0]);
    Serial.print("\t");
    Serial.println(pulses[1]);
    
  }

  if(Serial.available() > 0){
    while (Serial.peek() == 'S') { // Start-Tag
      Serial.read();               // S (Start) vom Serial-Buffer löschen
      pin = Serial.parseInt();     // Pin-Information lesen
      mode = Serial.read();        // D bzw. A vom Serial-Buffer löschen
      level = Serial.parseInt();   // Level lesen

      Serial.println("Setting PIN ");
      Serial.print(pin);
      Serial.print(" to ");
      Serial.print(mode);
      Serial.println(level);

      if(mode == 'D')
        digitalWrite(pin, level);  // LED-Status setzen
      if(mode == 'A')
        analogWrite(pin, level);   // LED-Status setzen
    }
    while (Serial.available() > 0){ // Buffer löschen
      Serial.read();
    }
  }

}


// ISRs for pulse counting
void tacho_0_Pulse() {
  tacho_0_pulses++;
}

void tacho_1_Pulse() {
  tacho_1_pulses++;
}

void setupWifi(){
  WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(WiFi.hostname());

    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false);
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/pulses", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String fan;
        if (request->hasParam(PARAM_FAN)) {
            fan = request->getParam(PARAM_FAN)->value();
            if(fan == "0"){
              request->send(200, "text/plain", String(pulses[0]));
            }
            if(fan == "1"){
              request->send(200, "text/plain", String(pulses[1]));
            }
        }
        request->send(200, "text/plain", "param fan not specified");
    });

    server.on("/pulses", HTTP_POST, [] (AsyncWebServerRequest *request) {
        String fan;
        String value;
        if (request->hasParam("value")) {
                value = request->getParam("value")->value();
        }
        if (request->hasParam(PARAM_FAN)) {
            fan = request->getParam(PARAM_FAN)->value();
            if(fan == "0"){     
              analogWrite(PWM_PIN_0, value.toInt());
            }
            if(fan == "1"){
              analogWrite(PWM_PIN_0, value.toInt());
            }
        }
        request->send(200, "text/plain");
    });

    server.onNotFound(notFound);

    server.begin();
}
