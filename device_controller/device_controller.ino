#include "DHT.h"
#include <iostream>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Wifi configs
#define WIFI_SSID "YOUR WIFI SSID"
#define WIFI_PASSWORD "YOUR WIFI PASSWORD"

//MQTT configs
//You can get your IP address by running the "hostname -I" command on terminal
#define MQTT_HOST IPAddress(192, 168, 0, 0)
#define MQTT_PORT 1883

//topics
#define MQTT_PUB_TEMP "esp/dht/temperature"
#define MQTT_PUB_HUM "esp/dht/humidity"
#define MQTT_PUB_WIND "esp/wind"
#define MQTT_PUB_FLOW "esp/flow"

#define DHTPIN 12     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define redLed 0
#define blueLed 5
#define yellowLed 4
#define decrease D7
#define increase D8

int flow = 0;
double rain = 0.0;

DHT dht(DHTPIN, DHTTYPE);

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("esp/rain", 1);
  Serial.print("Subscribing at QoS 1, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.println(payload[0]);

  rain = atof(payload);
}

void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(decrease, INPUT);
  pinMode(increase, INPUT);
  Serial.begin(115200); 
  dht.begin();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  
  connectToWifi();
}

void loop() {
  delay(5000);

  //Read wind speed
  float w = analogRead(A0)/50;
  // Read humidity
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  
  if(digitalRead(decrease) == 1){
    flow = flow-1;
  }

  if(digitalRead(increase) == 1){
    flow = flow+1;
  }
  
  // Check if any reads failed and exit early.
  if (isnan(h) || isnan(t) || isnan(w)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (rain > 60.0){
    digitalWrite(blueLed, LOW);
    digitalWrite(redLed, HIGH);
    digitalWrite(yellowLed, LOW);
  }else {
    int cWind, cHumidity, cTemperature, cRain;
    
    if(t > 15 && t < 32) {
      cTemperature = 1;
    }else{
      cTemperature = 2;
    }
  
    if(h > 55 && h < 95) {
      cHumidity = 1;
    } else {
      cHumidity = 2;
    }
  
    if(w > 2 && w < 10) {
      cWind = 1;
    } else {
      cWind = 2;
    }

    if(rain > 40.0) {
      cRain = 2;  
    }else{
      cRain = 1;
    }
  
    float condition = (cTemperature + cWind + cHumidity + cRain) / 4.0;
  
    if(condition == 1.00){
      digitalWrite(blueLed, HIGH);
      digitalWrite(redLed, LOW);
      digitalWrite(yellowLed, LOW);
    } else if (condition > 1 && condition <= 1.25){
      digitalWrite(blueLed, LOW);
      digitalWrite(redLed, LOW);
      digitalWrite(yellowLed, HIGH);
    } else {
      digitalWrite(blueLed, LOW);
      digitalWrite(redLed, HIGH);
      digitalWrite(yellowLed, LOW);
    }
  }
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" % \t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" ºC \t");
  Serial.print("Wind: ");
  Serial.print(w);
  Serial.print(" Km/h \t");
  Serial.print("Rain chances: ");
  Serial.print(rain);
  Serial.println(" %");
  
  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(t).c_str());
    
  uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(h).c_str());
    
  uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_WIND, 1, true, String(w).c_str());                            
    Serial.printf("Publishing on topics %s, %s, %s at QoS 1: ", MQTT_PUB_WIND, MQTT_PUB_HUM, MQTT_PUB_TEMP);
    Serial.printf("Messages: %.2f, %.2f, %.2f \n", w, h, t);
  
  if(flow > 0){
    Serial.print("*Spraying* \t Flow: ");
    Serial.print(flow);
    Serial.println(" L/s");
    uint16_t packetIdPub4 = mqttClient.publish(MQTT_PUB_FLOW, 1, true, String(flow).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1: ", MQTT_PUB_FLOW);
    Serial.printf("Message: %.2f \n", flow);
  }
}
