#ifndef CALIBRATION_H
#define CALIBRATION_H
 
#include <Arduino.h>
#include <math.h>
 
const float DHT_TEMP_M =  1.1155;  
const float DHT_TEMP_C = -3.6764;  
 
const float DHT_HUM_M  = 1.00;  
const float DHT_HUM_C  = 0.00;  
 
const float SOIL_M     = -0.2478;  
const float SOIL_C     = 249.32;  
 
const float TANK_FORCE_M = 1.00;  
const float TANK_FORCE_C = 0.00;  
 
const double SCALE_M   = 0.0026;
const double SCALE_C   = 861.47-0.9;    //+k, Adjusted Y-intercept for clean 0.0g base
 
const float FLOW_FACTOR = 382.5;
 
const float LDR_COEFF_A = 0.3274;
const float LDR_COEFF_B = 0.0098;
 
 
inline float getCalibratedTemperature(float rawTemp) {
    return (DHT_TEMP_M * rawTemp) + DHT_TEMP_C;
}
 
inline float getCalibratedHumidity(float rawHum) {
    return (DHT_HUM_M * rawHum) + DHT_HUM_C;
}
 
inline float getCalibratedSoilMoisture(float rawSoil) {
    return (SOIL_M * rawSoil) + SOIL_C;
}
 
inline float getCalibratedTankForce(float rawForce) {
    return (TANK_FORCE_M * rawForce) + TANK_FORCE_C;
}
 
inline float getCalibratedScaleWeight(double rawAvg) {
    double weight = (SCALE_M * rawAvg) + SCALE_C;
    if (abs(weight) < 0.5) { weight = 0.0; }
    return (float)weight;
}
 
inline float getCalibratedFlowRate(float pulseCount) {
    return (pulseCount / FLOW_FACTOR) * 60.0;
}
 
inline float getCalibratedLux(int rawADC) {
    return LDR_COEFF_A * exp(LDR_COEFF_B * (float)rawADC);
}
 
#endif