void updateAmbientLux() {
  currentLux = getCalibratedLux(analogRead(LDR_PIN));
}