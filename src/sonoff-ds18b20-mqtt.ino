#include <DallasTemperature.h>
#include <OneWire.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
// #include "Streaming.h"

#define RELAY 12
#define LED 13
#define BUTTON 0
#define TEMP_SENSOR 14
#define CONNECTION_WAIT_TIME 300 // How long ESP8266 should wait before next attempt to connect to WiFi or MQTT Broker (msec)

/* Configuration parameters */
const int   ID              = ESP.getChipId();        // Device ID
const char* WIFI_SSID       = "<SSID>";               // WiFi Name
const char* WIFI_PASSWORD   = "<password>";           // WiFi Password
const char* MQTT_HOST       = "<host>";               // MQTT Broker Host
const int   MQTT_PORT       = 1883;                   // MQTT Port
const char* MQTT_USER       = "<user>";               // MQTT User
const char* MQTT_PASSWORD   = "<password?";           // MQTT Password
const char* MQTT_TOPIC      = "/sonoff/switch/";      // MQTT Topic
const float TEMP_CORRECTION = 0;                      // Temperature correction
const int   TEMP_INTERVAL   = 600;                    // How often temperature sensor should be read

/* Variables */
char  mqttTopic[26];  // it stories topic which is MQTT_TOPIC/ID/
float tempCorrection = TEMP_CORRECTION;
float previousTemperature = 0;
/* Timers */
Ticker buttonTimer;
unsigned long pressedCount = 0;

Ticker temperatureTimer;

/* Libraries init */
WiFiClient esp;
PubSubClient client(esp);
OneWire wireProtocol(TEMP_SENSOR);
DallasTemperature DS18B20(&wireProtocol);

/* Setup */
void setup() {
  Serial.begin(115200);
  delay(10);

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callbackMQTT);

  Serial.println();
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(BUTTON, INPUT_PULLUP);

 // Serial << " Device ID: " << ID << endl;

  sprintf(mqttTopic, "%s%i", MQTT_TOPIC, ID);

  connectToWiFi();
  DS18B20.begin();
  setSensorReadInterval(TEMP_INTERVAL);
  buttonTimer.attach(0.1, button);
}


/* Blink LED, t defines for how long LED should be ON */
void blinkLED(int t=50) {
  if (digitalRead(LED)==HIGH) {
    digitalWrite(LED, LOW);
  }
  delay(t);
  digitalWrite(LED, HIGH);
}

/* Publishing state of the Relay to MQTT Broker */
void publishRelayStateMessage() {
  char  mqttString[50];
  sprintf(mqttString,"%s/state", mqttTopic);
  if (digitalRead(RELAY)==LOW) {
    client.publish(mqttString, "OFF");
  } else {
      client.publish(mqttString, "ON");
  }
}

/* Pubish temperature to MQTT broker */
void publishTemperature() {
  char  temperatureString[6];
  char  mqttString[50];
  float temperature = getTemperature();
  dtostrf(temperature, 2, 1, temperatureString);
  if (previousTemperature!=temperature) {
    sprintf(mqttString,"%s/temperature", mqttTopic);
    client.publish(mqttString, temperatureString);
    previousTemperature=temperature;
  }
}


/* Gets default configuration. Relay state and temp sensor config. It has to be implemented in the service and published over MQTT */
void getConfiguration() {
  char  mqttString[50];
  sprintf(mqttString,"%s/get", mqttTopic);
  client.publish(mqttString, "defaultState");
  blinkLED();
}

/* Set relay to ON */
void setON() {
  digitalWrite(RELAY, HIGH);
  publishRelayStateMessage();
}

/* Set relay to OFF */
void setOFF() {
  digitalWrite(RELAY, LOW);
  publishRelayStateMessage();
}

/* Connect to WiFI */
void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    blinkLED(CONNECTION_WAIT_TIME/2);
    delay(CONNECTION_WAIT_TIME/2);
  }
}

/* Connected to MQTT Broker */
void connectToMQTT() {
  char  mqttString[50];
  sprintf(mqttString,"Sonoff (ID: %i)",ID);
  while (!client.connected()) {
    if (client.connect(mqttString, MQTT_USER, MQTT_PASSWORD)) {
        sprintf(mqttString,"%s/cmd", mqttTopic);
        client.subscribe(mqttString);
        getConfiguration();
    } else {
      blinkLED(CONNECTION_WAIT_TIME/2);
      delay(CONNECTION_WAIT_TIME/2);
    }
  }
}

/* Set how often sensor should be read */
void setSensorReadInterval(int interval) {
  temperatureTimer.detach();
  temperatureTimer.attach(interval, publishTemperature);
}

/* Callback of MQTT Broker, it listens for messages */
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  char  mqttString[50];
  blinkLED();
  if (length>=6) { // command arrived
    if((char)payload[5] == 'N') { // turnON
      setON();
    } else if((char)payload[5] == 'F') { // turnOFF
      setOFF();
    } else if((char)payload[2] == 'p') { // reportState
      publishRelayStateMessage();
    } else if((char)payload[2] == 's') { // reset
      ESP.restart();
    } else if((char)payload[4] == 'I') { // tempInterval
        String inString = "";
        for (int i=13;i<length;i++) {
            inString += (char)payload[i];
        }
        setSensorReadInterval(inString.toInt());
    } else if((char)payload[4] == 'C') { // tempCorrection
        String inString = "";
        for (int i=15;i<length;i++) {
            inString += (char)payload[i];
        }
        tempCorrection =  inString.toFloat();
    }
  }
}

/* Button pressed method. Short changes relay state, long reboot device */
void button() {
  if (!digitalRead(BUTTON)) {
    pressedCount++;
  }
  else {
    if (pressedCount > 1 && pressedCount <= 50) {
      if (digitalRead(RELAY)==LOW) {
          setON();
      } else {
          setOFF();
      }
    }
    else if (pressedCount >50){
      blinkLED(5000);
    }
  pressedCount = 0;
  }
}

/* Get temperature */
float getTemperature() {
  float temperature;
  do {
    DS18B20.requestTemperatures();
    temperature = DS18B20.getTempCByIndex(0);
  } while (temperature == 85.0 || temperature == (-127.0));
  return temperature + tempCorrection;
}

void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();
}