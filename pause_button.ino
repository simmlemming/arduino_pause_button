#include <SPI.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Button.h"

#define PIN_RED_LED 2
#define PIN_GREEN_LED 3
#define PIN_BTN_PAUSE 6

const char* CMD_PAUSE = "pause";
const char* NAME_ALL = "all";

const char* deviceName = "pause_button";

// MQTT
const char* mqtt_server = "192.168.0.110";
const char* outTopic = "home/out";
const char* inTopic = "home/in";

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x04 };
byte ip[] = { 192, 168, 0, 123 };

EthernetClient ethClient;
PubSubClient client(ethClient);

boolean mqtt_connecting = false, mqtt_connected = false, mqtt_error = false;

const int STATE_OK = 1;
const int STATE_INIT = 2;
const int STATE_ERROR = 3;
const int STATE_PAUSED = 6;

const int PAUSE_TIMEOUT_SEC = 30;

int state = STATE_INIT;

const char* name01 = "motion_sensor_01";
const char* name02 = "motion_sensor_02";

bool is01Paused = false;
bool is02Paused = false;


void onPauseClick() {
//  Serial.println("****");
  StaticJsonBuffer<64> jsonBuffer;
  char jsonMessageBuffer[64];
  
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = NAME_ALL;
  root["cmd"] = CMD_PAUSE;  
  root["value"] = PAUSE_TIMEOUT_SEC;
  
  root.printTo(jsonMessageBuffer, sizeof(jsonMessageBuffer));
  client.publish(inTopic, jsonMessageBuffer);
}

Button buttonPause = Button(PIN_BTN_PAUSE, onPauseClick);

void setup() {
  Serial.begin(9600);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_GREEN_LED, OUTPUT);
  
  Ethernet.begin(mac, ip);

  client.setServer(mqtt_server, 1883);
  client.setCallback(onNewMessage);
}

void loop() {  
  setup_mqtt();
  
  client.loop();
  buttonPause.loop();

  if (!ethClient.connected()) {
    state = STATE_INIT;
  } else {
    state = STATE_OK;
  }

  updateLed();
}

void onNewMessage(char* topic, byte* payload, unsigned int length) {
  StaticJsonBuffer<64> jsonBuffer;
  JsonObject& message = jsonBuffer.parseObject(payload);

  if (!message.success()) {
    return;
  }

//  char jsonMessageBuffer[128];
//  message.printTo(jsonMessageBuffer, sizeof(jsonMessageBuffer));  
//  Serial.print("<-- ");
//  Serial.println(jsonMessageBuffer);

  const char* messageName = message.get<const char*>("name");
  const int messageState = message.get<int>("state");
  
  if (eq(messageName, name01)) {
    is01Paused = messageState == STATE_PAUSED;
  }
  
  if (eq(messageName, name02)) {
    is02Paused = messageState == STATE_PAUSED;
  }
}

void updateLed() {
  if (state == STATE_INIT) {
    greenOn();
    delay(250);
    greenOff();
    delay(250);
    return;
  }

  int btn = digitalRead(PIN_BTN_PAUSE);
  if (btn == LOW) {
    redOn();
    greenOff();
  } else if (state == STATE_OK) {
    greenOn();
    redOff();
  } else { // unknown state
    redOn();
    greenOff();
  }
}

void setup_mqtt() {
  mqtt_connected = client.connected();

  if (mqtt_connected) {
    return;
  }

  if (!mqtt_connecting) {
    mqtt_connecting = true;

    if (client.connect(deviceName)) {
      mqtt_error = false;
      mqtt_connecting = false;
      mqtt_connected = true;
      client.subscribe(outTopic);
    } else {
      mqtt_error = true;
      mqtt_connecting = false;
      mqtt_connected = false;
    }
  }
}

boolean eq(const char* a1, const char* a2) {
  return strcmp(a1, a2) == 0;
}

void greenOn() {
  digitalWrite(PIN_GREEN_LED, HIGH);
}

void greenOff() {
  digitalWrite(PIN_GREEN_LED, LOW);
}

void redOn() {
  digitalWrite(PIN_RED_LED, HIGH);
}

void redOff() {
  digitalWrite(PIN_RED_LED, LOW);
}


void debugPrint() {
  Serial.println(" ");

  Serial.print("eth connected = ");
  Serial.println(ethClient.connected());

  Serial.print("mqtt connecting = ");
  Serial.println(mqtt_connecting);

  Serial.print("mqtt connected = ");
  Serial.println(mqtt_connected);
  
  Serial.print("mqtt error = ");
  Serial.println(mqtt_error);
}
