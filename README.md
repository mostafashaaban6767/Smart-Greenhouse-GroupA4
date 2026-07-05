# Smart Greenhouse — Group A4

**Course:** Sensors \& Instrumentation — Final Project
**Instructor:** Dr. Mohamad Aoude
**Institution:** Lebanese University — Faculty of Engineering III
**Academic Year:** 2025–2026

## Overview

An Arduino Uno–based smart greenhouse system that monitors ambient conditions and reservoir status, and automatically drives irrigation and cooling in response to sensor readings. The system integrates five sensor modalities (optical, force, flow, temperature, humidity) plus soil moisture as a bonus sensor, and logs all measurements as CSV over serial for offline analysis.

### Automated behavior

* **Irrigation:** the pump activates when soil moisture drops below a defined threshold *and* the reservoir has sufficient water (by weight), and shuts off on a runtime timeout or once reservoir weight drops below a minimum — whichever occurs first.
* **Cooling:** the fan switches on above a defined ambient temperature threshold and off below it.
* **Manual override:** both the pump and fan (and the LCD display page) can be manually controlled over serial, independent of automatic control, for testing and calibration.

## Team Members

|Name|ID|
|-|-|
|Mostafa Shaaban|*6767*|
|Ghassan Iskandarani|*6928*|
|Ali Daher|*6927*|
|Hassan Hareb|*6777*|

## Hardware

|Component|Function|
|-|-|
|Arduino Uno|Main controller|
|DHT11|Temperature and humidity|
|HX711 + load cell|Reservoir weight (force)|
|YF-S201|Flow rate|
|LDR (voltage divider)|Ambient light (optical)|
|HW-080 + probe|Soil moisture (bonus)|
|IRLZ44N MOSFET ×2|Pump and fan switching|
|LM2596 buck converter|19V → regulated 5V actuator rail|
|1N4007 flyback diodes ×2|Motor-Pump inductive-kickback protection|
|16×2 I2C LCD|Local status display|

Full schematic: [`docs/Greenhouse\_Schematic.pdf`](docs/Greenhouse_Schematic.pdf)
Bill of materials: [`docs/BOM.md`](docs/BOM.md)
Component datasheets: [`docs/Datasheets`](docs/datasheets)

## Repository Structure

```
README.md
/docs           Schematic, datasheets, and supporting reference material
/code           Arduino firmware (modular, one file per sensor driver)
/data           Raw calibration data and logged test-run CSVs
/video          Demonstration video
/report         Final technical report
```

### `/code`

Sketch folder: `GreenhouseCore/`

|File|Purpose|
|-|-|
|`GreenhouseCore.ino`|Main sketch — actuator control, LCD, serial commands, CSV logging|
|`DHT11\_Sensor.ino`|Temperature and humidity driver|
|`ScaleHX711.ino`|Load cell / weight driver|
|`FlowSensor.ino`|Flow rate driver (interrupt + debounce + median filter)|
|`LDR\_Sensor.ino`|Ambient light driver|
|`SoilSensor.ino`|Soil moisture driver (power-gated to limit probe corrosion)|
|`calibration.h`|Centralized calibration constants and conversion functions for all sensors|

### `/data`

|Folder|Contents|
|-|-|
|`1 hour run greenhouse data`|Full CSV log from the 1-hour continuous test run (Section 8 of the report)|
|`values used for calibration`|Raw reference-vs-sensor calibration data per sensor, underlying the calibration curves presented in Section 7|

## Serial Command Interface

Sent over the serial monitor at 9600 baud:

|Command|Effect|
|-|-|
|`P3:<0-255>`|Manually set pump PWM duty; disables automatic pump control|
|`P3:AUTO`|Return pump to automatic control|
|`P6:<0-255>`|Manually set fan PWM duty; disables automatic fan control|
|`P6:AUTO`|Return fan to automatic control|
|`LCD:FIX:<0\|1>`|Pin the LCD to a specific display page|
|`LCD:AUTO`|Return LCD to automatic page alternation|

## Serial Output Format (CSV)

```
index,time\_s,temp\_C,hum\_pct,weight\_g,flow\_Lmin,lux,soil\_pct,pwm3,pwm6
```

A value of `-999` indicates the sensor had no valid reading at that tick (e.g. a failed DHT11 checksum, or the soil sensor outside its measurement window).

## Building and Uploading

1. Open `code/GreenhouseCore/GreenhouseCore.ino` in the Arduino IDE (all other `.ino`/`.h` files in the folder load automatically as tabs).
2. Install the required libraries (see below) via **Sketch → Include Library → Manage Libraries**.
3. Select **Arduino Uno** as the board and the correct COM port.
4. Upload.
5. Open the Serial Monitor/Plotter at **9600 baud** to view live CSV output.

## Third-Party Libraries

|Library|Purpose|Source|License|
|-|-|-|-|
|`LiquidCrystal\_I2C`|I2C LCD driver|*markub3327/LiquidCrystal\_I2C (fork of johnrickman/LiquidCrystal\_I2C)*|*MIT*|
|`DHT sensor library`|DHT11 temperature/humidity|*adafruit/DHT-sensor-library*|*MIT*|
|`HX711`|Load cell amplifier interface|*bogde/HX711*|*MIT*|

## Report and Demonstration

* Final technical report: [`report/`](report) *(to be added)*
* Demonstration video link: [`video/demonstration_video_link.txt`](video/demonstration_video_link.txt)

