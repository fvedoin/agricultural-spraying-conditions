#include "DHT.h"

#define DHTPIN 12     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define redLed 0
#define blueLed 5
#define yellowLed 4
#define decrease D7
#define increase D8

int flow = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(decrease, INPUT);
  pinMode(increase, INPUT);
  Serial.begin(9600); 
  dht.begin();
}

void loop() {
  delay(2000);

  //Read wind speed
  float w = analogRead(A0);
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
  int cWind, cHumidity, cTemperature;
  
  if(t < 10 || t > 35) {
    cTemperature = 3;
  } else if((t >= 10 && t < 20) || (t > 30 && t <= 35)){
    cTemperature = 2;
  } else if(t >= 20 && t <= 30){
    cTemperature = 1;
  }

  if(h < 60 || h > 95) {
    cHumidity = 3;
  } else if((h >= 60 && h < 70) || (h > 90 && h <= 95)){
    cHumidity = 2;
  } else if(h >= 70 && h <= 90){
    cHumidity = 1;
  }

  if(w <= 10) {
    cWind = 1;
  } else {
    cWind = 3;
  }

  float condition = (cTemperature + cWind + cHumidity) / 3.0;

  if(condition == 1.00){
    digitalWrite(blueLed, HIGH);
    digitalWrite(redLed, LOW);
    digitalWrite(yellowLed, LOW);
  } else if (condition > 1 && condition <= 2){
    digitalWrite(blueLed, LOW);
    digitalWrite(redLed, LOW);
    digitalWrite(yellowLed, HIGH);
  } else {
    digitalWrite(blueLed, LOW);
    digitalWrite(redLed, HIGH);
    digitalWrite(yellowLed, LOW);
  }
  
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("\t");
  Serial.print("Wind: ");
  Serial.print(w);
  Serial.println(" Km/h ");

  if(flow > 0){
    Serial.print("*Spraying* \t Flow: ");
    Serial.print(flow);
    Serial.println(" L/s");
  }
}
