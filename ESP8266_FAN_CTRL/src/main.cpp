#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"

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

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

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

  analogWrite(PWM_PIN_0, 0);
  analogWrite(PWM_PIN_1, 0);

  pinMode(ON_OFF, OUTPUT);
  
  pinMode(TACHO_0, INPUT_PULLUP);
  pinMode(TACHO_1, INPUT_PULLUP);

  // TODO check fans data sheet
  analogWriteFreq(20000);

  attachInterrupt(digitalPinToInterrupt(TACHO_0), tacho_0_Pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(TACHO_1), tacho_1_Pulse, FALLING);

  setupWifi();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client){
    client->send("hello!",NULL,millis(),1000);
  });
  server.addHandler(&events);
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
        String resp = "{\"fan0\":" + String(pulses[0]) + ", \"fan1\": " + String(pulses[1]) + "}";
        request->send(200, "text/plain", resp);
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
              analogWrite(PWM_PIN_1, value.toInt());
            }
        }
        request->send(200, "text/plain");
    });

    server.onNotFound(notFound);

    server.begin();
}
