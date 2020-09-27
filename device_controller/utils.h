float setCondition(float wind, float humidity, float temperature, double rain){
  int cWind, cHumidity, cTemperature, cRain;
    
  if(temperature > 15 && temperature < 32) {
    cTemperature = 1;
  }else{
    cTemperature = 2;
  }
  
  if(humidity > 55 && humidity < 95) {
    cHumidity = 1;
  } else {
    cHumidity = 2;
  }
  
  if(wind > 2 && wind < 10) {
    cWind = 1;
  } else {
    cWind = 2;
  }

  if(rain > 40.0) {
    cRain = 2;  
  }else{
    cRain = 1;
  }

  return (cTemperature + cWind + cHumidity + cRain) / 4.0;
}

void printConditions(float wind, float humidity, float temperature, double rain, int flow){
  Serial.print("Humidity: "); 
  Serial.print(humidity);
  Serial.print(" % \t");
  Serial.print("Temperature: "); 
  Serial.print(temperature);
  Serial.print(" ºC \t");
  Serial.print("Wind: ");
  Serial.print(wind);
  Serial.print(" Km/h \t");
  Serial.print("Rain chances: ");
  Serial.print(rain);
  Serial.println(" %");
  if(flow > 0){
    Serial.print("*Spraying* \t Flow: ");
    Serial.print(flow);
    Serial.println(" L/s");
  }
}
