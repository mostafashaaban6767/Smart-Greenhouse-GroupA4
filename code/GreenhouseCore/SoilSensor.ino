void initSoilSensor() {
  pinMode(SOIL_PWR_PIN, OUTPUT);
  isSoilReadingWindow = true;
  digitalWrite(SOIL_PWR_PIN, HIGH); 
  lastSoilReadMillis = millis();
}

void handleSoilDutyCycle(unsigned long currentMillis) {
  if (!isSoilReadingWindow && (currentMillis - lastSoilReadMillis >= soilReadInterval)) {
    lastSoilReadMillis = currentMillis;
    isSoilReadingWindow = true;
    digitalWrite(SOIL_PWR_PIN, HIGH); 
  }
  
  if (isSoilReadingWindow && (currentMillis - lastSoilReadMillis >= soilReadWindow)) {
    isSoilReadingWindow = false;
    digitalWrite(SOIL_PWR_PIN, LOW);  
    currentSoil = NAN;                 
  }
}

void readSoilMoisture() {
  if (isSoilReadingWindow) {
    float calculatedSoil = getCalibratedSoilMoisture(analogRead(SOIL_PIN));
    currentSoil = constrain(calculatedSoil, 0.0, 100.0);
  }
}