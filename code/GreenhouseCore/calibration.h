/*
   calibration.h - Fixed Math Layer
*/

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>

// DHT11 Temperature Calibration
const float DHT_TEMP_M =  1.1155;  
const float DHT_TEMP_C = -3.6764;  

const float DHT_HUM_M  = 1.00;  
const float DHT_HUM_C  = 0.00;  

const float SOIL_M     = -0.2478;  
const float SOIL_C     = 249.32;  

const float TANK_FORCE_M = 1.00;  
const float TANK_FORCE_C = 0.00;  

// HX711 Scale
const double SCALE_M   = 0.0026; // Replace with your exact chart slope
const double SCALE_C   = 861.47-0.9;    //+k, Adjusted Y-intercept for clean 0.0g base

// YF-S201 Flow Factor (pulses per liter)
// Calibrated: 3 trials of 0.4L each, motor running, debounced ISR.
// Total pulses 459 / Total volume 1.2L = 382.5 pulses/L
const float FLOW_FACTOR = 382.5;


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

// pulseCount is the number of pulses counted in the most recent 1-second window.
// FLOW_FACTOR is in pulses per liter (from calibration).
// (pulses/sec) / (pulses/L) = L/sec; multiply by 60 for L/min.
inline float getCalibratedFlowRate(float pulseCount) {
    return (pulseCount / FLOW_FACTOR) * 60.0;
}

// FIX: Linear mapping for LDR to avoid log dropouts in normal lighting
inline float getCalibratedLux(int rawADC) {
    // If raw numbers are working, let's map the 0-1023 analog range 
    // to a reliable, clean 0-1000 Lux estimation directly.
    float lux = map(rawADC, 0, 1023, 0, 1000);
    return lux;
}

#endif // CALIBRATION_H