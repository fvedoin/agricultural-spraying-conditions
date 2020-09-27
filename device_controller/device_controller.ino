#include "DHT.h"
#include <iostream>
#include <Ticker.h>
#include <ESP8266WiFi.h>

double rain = 0.0;

#include "mqttFunctions.h"
#include "utils.h"

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

//DHT configs
#define DHTPIN 12
#define DHTTYPE DHT22

//Led pins
#define redLed 0
#define blueLed 5
#define yellowLed 4

//Buttons pins
#define decrease D7
#define increase D8

int flow = 0;

DHT dht(DHTPIN, DHTTYPE);

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
  float wind = analogRead(A0)/50;
  // Read humidity
  float humidity = dht.readHumidity();
  // Read temperature as Celsius
  float temperature = dht.readTemperature();

  // Check if any reads failed and exit early.
  if (isnan(humidity) || isnan(temperature) || isnan(wind) || isnan(digitalRead(decrease)) || isnan(digitalRead(increase))) {
    Serial.println("Failed to read data from sensors!");
    return;
  }
  
  if(digitalRead(decrease) == 1){
    flow = flow-1;
  }

  if(digitalRead(increase) == 1){
    flow = flow+1;
  }

  if (rain > 60.0){
    digitalWrite(blueLed, LOW);
    digitalWrite(redLed, HIGH);
    digitalWrite(yellowLed, LOW);
  }else {
    
    float condition = setCondition(wind, humidity, temperature, rain);
  
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

  printConditions(wind, humidity, temperature, rain, flow);
  
  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temperature).c_str());  
  uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(humidity).c_str());
  uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_WIND, 1, true, String(wind).c_str());
  uint16_t packetIdPub4 = mqttClient.publish(MQTT_PUB_FLOW, 1, true, String(flow).c_str());                            
                
  Serial.printf("Publishing on topics %s, %s, %s, %s at QoS 1: ", MQTT_PUB_WIND, MQTT_PUB_HUM, MQTT_PUB_TEMP, MQTT_PUB_FLOW);
  Serial.printf("Messages: %.2f, %.2f, %.2f, %d \n", wind, humidity, temperature, flow);
  
}
