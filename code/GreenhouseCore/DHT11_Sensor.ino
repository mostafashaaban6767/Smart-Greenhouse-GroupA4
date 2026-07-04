void initDHT11() {
  dht.begin();
}

void readDHT11() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (!isnan(t) && !isnan(h)) {
    currentTemp = getCalibratedTemperature(t);
    currentHum = getCalibratedHumidity(h);
  } else {
    currentTemp = NAN;
    currentHum = NAN;
  }
}