#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "HX711.h"
#include "calibration.h"

#define FLOW_PIN 2
#define DHTPIN 4
#define PWM_3_PIN 3 
#define PWM_6_PIN 6 
#define SCALE_DOUT_PIN 8
#define SCALE_SCK_PIN 9
#define LDR_PIN A0
#define SOIL_PIN A1
#define SOIL_PWR_PIN 5 

#define DHTTYPE DHT11

#define SCALE_SAMPLE_COUNT 7
#define SCALE_SAMPLE_TIMEOUT_MS 90

DHT dht(DHTPIN, DHTTYPE);
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2); 

volatile unsigned long flowPulseCount = 0;
volatile unsigned long lastPulseMicros = 0;
const unsigned long minPulseGapMicros = 4000;

unsigned long previousMillis = 0;
const unsigned long loopTickInterval = 1000;
bool alternateStateToggle = false;
unsigned long logCount = 0;

unsigned long previousRampMillis = 0;
const unsigned long rampStepInterval = 15;
int currentPwm3 = 0;
int targetPwm3  = 0;
int pwm6Value   = 0;

float currentTemp = NAN;
float currentHum = NAN;
float currentWeight = NAN;
float currentFlowRate = 0.0;
float currentLux = 0.0;
float currentSoil = NAN;

#define FLOW_MEDIAN_SIZE 3   
float flowRateHistory[FLOW_MEDIAN_SIZE];
int flowHistoryIndex = 0;
bool flowHistoryFilled = false;

unsigned long lastSoilReadMillis = 0;
const unsigned long soilReadInterval = 900000; 
const unsigned long soilReadWindow = 2500;     
bool isSoilReadingWindow = false;

unsigned long pumpRunStartMillis = 0;
bool isPumpRunningAutomated = false;
bool isPumpManualOverride = false; 

bool isFanManualOverride = false;

unsigned long lastDisplaySwitchMillis = 0;
int activeDisplayScreen = 1;
bool isLcdManualOverride = false;  

void flowPulseISR() {
  unsigned long now = micros();
  if (now - lastPulseMicros > minPulseGapMicros) {
    flowPulseCount++;
    lastPulseMicros = now;
  }
}

float medianOf(float *source, int size) {
  float buf[size];
  memcpy(buf, source, size * sizeof(float));
  for (int i = 1; i < size; i++) {
    float key = buf[i];
    int j = i - 1;
    while (j >= 0 && buf[j] > key) {
      buf[j + 1] = buf[j];
      j--;
    }
    buf[j + 1] = key;
  }
  return buf[size / 2];
}

float logSafe(float value) {
  return isnan(value) ? -999.0 : value;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(PWM_3_PIN, OUTPUT);
  pinMode(PWM_6_PIN, OUTPUT);
  
  analogWrite(PWM_3_PIN, currentPwm3);
  analogWrite(PWM_6_PIN, pwm6Value);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Greenhouse Core");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  initDHT11();
  initScaleHX711();
  initFlowSensor();
  initSoilSensor();
  
  Serial.println("--- Full Greenhouse Management System Online ---");
  Serial.println("index,time_s,temp_C,hum_pct,weight_g,flow_Lmin,lux,soil_pct,pwm3,pwm6");
}

void loop() {
  unsigned long currentMillis = millis();

  if (Serial.available() > 0) {
    String commandString = Serial.readStringUntil('\n');
    commandString.trim();

    if (commandString.equalsIgnoreCase("P3:AUTO")) {
      isPumpManualOverride = false;
      isPumpRunningAutomated = false;
      targetPwm3 = 0;
      Serial.println(">> Pump control handed back to AUTOMATION.");
    }
    else if (commandString.startsWith("P3:")) {
      int parsedVal = commandString.substring(3).toInt();
      if (parsedVal >= 0 && parsedVal <= 255) {
        isPumpManualOverride = true;
        targetPwm3 = parsedVal;
        Serial.print(">> MANUAL OVERRIDE: Target Pump P3 set to: "); Serial.println(targetPwm3);
      }
    } 
    else if (commandString.equalsIgnoreCase("P6:AUTO")) {
      isFanManualOverride = false;
      Serial.println(">> Fan control handed back to AUTOMATION.");
    }
    else if (commandString.startsWith("P6:")) {
      int parsedVal = commandString.substring(3).toInt();
      if (parsedVal >= 0 && parsedVal <= 255) {
        isFanManualOverride = true;
        pwm6Value = parsedVal;
        analogWrite(PWM_6_PIN, pwm6Value);
        Serial.print(">> MANUAL OVERRIDE: Channel P6 updated to duty: "); Serial.println(pwm6Value);
      }
    }
    else if (commandString.equalsIgnoreCase("LCD:AUTO")) {
      isLcdManualOverride = false;
      Serial.println(">> LCD control handed back to AUTOMATIC ALTERNATION.");
    }
    else if (commandString.startsWith("LCD:FIX:")) {
      int parsedPage = commandString.substring(8).toInt();
      if (parsedPage == 0 || parsedPage == 1) {
        isLcdManualOverride = true;
        activeDisplayScreen = parsedPage;
        lcd.clear();
        Serial.print(">> LCD OVERRIDE: Display pinned cleanly to Screen "); Serial.println(activeDisplayScreen);
      }
    }
  }

  handleSoilDutyCycle(currentMillis);

  if (currentPwm3 != targetPwm3 && (currentMillis - previousRampMillis >= rampStepInterval)) {
    previousRampMillis = currentMillis;
    if (currentPwm3 < targetPwm3) {
      currentPwm3 += 5;
      if (currentPwm3 > targetPwm3) currentPwm3 = targetPwm3;
    } else {
      currentPwm3 -= 5;
      if (currentPwm3 < targetPwm3) currentPwm3 = targetPwm3;
    }
    analogWrite(PWM_3_PIN, currentPwm3);
  }

  if (currentMillis - previousMillis >= loopTickInterval) {
    previousMillis = currentMillis;
    
    noInterrupts();
    unsigned long snapshotsPulses = flowPulseCount;
    flowPulseCount = 0; 
    interrupts();
    processFlowTick(snapshotsPulses);

    updateAmbientLux();

    readSoilMoisture();

    if (alternateStateToggle == false) {
      readDHT11();
      alternateStateToggle = true;
    } 
    else {
      updateScaleWeight();
      alternateStateToggle = false;
    }
    
    if (!isFanManualOverride && !isnan(currentTemp)) {
      if (currentTemp > 30.0) {
        pwm6Value = 255;
      } else {
        pwm6Value = 0;
      }
      analogWrite(PWM_6_PIN, pwm6Value);
    }

    if (!isPumpManualOverride) {
      if (isPumpRunningAutomated) {
        if ((currentMillis - pumpRunStartMillis >= 12000) || (!isnan(currentWeight) && currentWeight < 278.3)) {
          targetPwm3 = 0;
          isPumpRunningAutomated = false;
          Serial.println(">> Automated Irrigation Completed.");
        }
      } else {
        if (isSoilReadingWindow && !isnan(currentSoil)) {
          if (currentSoil < 25.0) {
            if (!isnan(currentWeight) && currentWeight >= 278.3) {
              targetPwm3 = 180;
              pumpRunStartMillis = currentMillis;
              isPumpRunningAutomated = true;
              Serial.println(">> Automated Irrigation Initiated.");
            }
          }
        }
      }
    }

    if (!isLcdManualOverride) {
      if (currentMillis - lastDisplaySwitchMillis >= 3000) {
        lastDisplaySwitchMillis = currentMillis;
        activeDisplayScreen = (activeDisplayScreen + 1) % 2; 
        lcd.clear();
      }
    }

    if (activeDisplayScreen == 0) {
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      if (!isnan(currentTemp)) { 
        lcd.print(currentTemp, 1); 
        lcd.print("C   ");
      } else { 
        lcd.print("--C   ");
      }
      
      lcd.setCursor(11, 0);
      lcd.print("H:");
      if (!isnan(currentHum)) { 
        lcd.print((int)currentHum); 
        lcd.print("%");
      } else { 
        lcd.print("--%");
      }
      
      lcd.setCursor(0, 1);
      lcd.print("F:");
      lcd.print(pwm6Value > 0 ? "ON " : "OFF");
      
      lcd.setCursor(9, 1); 
      lcd.print("Lx:"); 
      
      int luxInt = (int)currentLux;
      if (luxInt < 10) {
        lcd.print("  ");
      } else if (luxInt < 100) {
        lcd.print(" ");
      }
      lcd.print(luxInt);
    } 
    else {
      lcd.setCursor(0, 0);
      lcd.print("Soil:");
      if (!isnan(currentSoil)) { lcd.print(currentSoil, 0); lcd.print("%"); } else { lcd.print("STBY");
      } 
      
      lcd.setCursor(11, 0);
      lcd.print("P:");
      lcd.print(currentPwm3 > 0 ? "ON " : "OFF");

      lcd.setCursor(0, 1);
      lcd.print("W:");
      if (!isnan(currentWeight)) { 
        lcd.print(currentWeight, 1); 
        lcd.print("g  ");
      } else { 
        lcd.print("--   ");
      }
      
      lcd.print("F:"); 
      lcd.print(currentFlowRate, 1);
    }

    logCount++;
    Serial.print(logCount); Serial.print(",");
    Serial.print(currentMillis / 1000.0, 1); Serial.print(",");
    Serial.print(logSafe(currentTemp), 1); Serial.print(",");
    Serial.print(logSafe(currentHum), 1);  Serial.print(",");
    Serial.print(logSafe(currentWeight), 1); Serial.print(",");
    Serial.print(currentFlowRate, 3); Serial.print(",");
    Serial.print(currentLux, 1);  Serial.print(",");
    Serial.print(logSafe(currentSoil), 1); Serial.print(",");
    Serial.print(currentPwm3); Serial.print(",");
    Serial.println(pwm6Value);
  }
}