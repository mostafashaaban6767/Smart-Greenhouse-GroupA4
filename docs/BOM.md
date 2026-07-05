# Bill of Materials (BOM)

Full parts list for the Smart Greenhouse system, based on the final schematic (`docs/Greenhouse_Schematic.pdf`). For sensor selection rationale, see Section 4 of the report; for wiring and design rationale, see Section 5.

## Microcontroller

| Ref | Component | Notes |
|---|---|---|
| — | Arduino Uno (Rev3) | Main controller |

## Sensors

| Ref | Component | Modality | Interface |
|---|---|---|---|
| — | DHT11 | Temperature / Humidity | Digital (1-wire), D4 |
| — | Load Cell + HX711 module | Force (weight) | 2-wire digital, D8 (DOUT) / D9 (SCK) |
| YF-S201 Flow Meter1 | YF-S201 | Flow | Digital interrupt, D2 |
| LDR | Photoresistor | Optical (light) | Analog, A0 |
| HW-080 Soil Hygrometer + Probe | HW-080 | Soil moisture (bonus) | Analog, A1; power-gated via D5 |

## Actuators

| Ref | Component | Function |
|---|---|---|
| Fan | DC fan | Cooling |
| Pump | DC water pump | Irrigation |

## Actuator Driver Stage

| Ref | Component | Value / Part No. | Function |
|---|---|---|---|
| Q1 | IRLZ44N | Logic-level N-MOSFET | Fan switching |
| Q2 | IRLZ44N | Logic-level N-MOSFET | Pump switching |
| R2 | Resistor | 1 kΩ | Q1 gate series resistor |
| R3 | Resistor | 10 kΩ | Q1 gate pulldown |
| R6 | Resistor | 1 kΩ | Q2 gate series resistor |
| R7 | Resistor | 10 kΩ | Q2 gate pulldown |
| D1 | Diode | 1N4007 | Fan flyback protection |
| D2 | Diode | 1N4007 | Pump flyback protection |
| C1 | Capacitor | 220 µF, 25 V | Fan rail supply smoothing |
| C2 | Capacitor | 220 µF, 25 V | Fan rail supply smoothing |

## Signal Conditioning

| Ref | Component | Value | Function |
|---|---|---|---|
| R1 | Resistor | 10 kΩ | DHT11 data line pull-up |
| R4 | Resistor | 1 kΩ | YF-S201 signal line RC filter (series) |
| C3 | Capacitor | 100 nF, 35 V | YF-S201 signal line RC filter (shunt) |
| R5 | Resistor | 10 kΩ | LDR voltage divider |

## Power

| Ref | Component | Value / Part No. | Function |
|---|---|---|---|
| V1 | DC power source | 19 V | Main system input |
| M1 | LM2596 adjustable buck converter | — | Steps 19 V down to the regulated logic/actuator rail |

## Display

| Ref | Component | Notes |
|---|---|---|
| Display1 | 16×2 LCD with I2C backpack (PCF8574, addr 0x27) | I2C, A4 (SDA) / A5 (SCL) |

## Notes

- All resistor/capacitor reference designators (R1–R7, C1–C3, D1–D2, Q1–Q2) correspond exactly to the labels in `docs/Greenhouse_Schematic.pdf` — cross-reference there for wiring.
- IRLZ44N was chosen specifically because it is a **logic-level** MOSFET, switching fully on with a direct 5 V Arduino gate signal (no gate driver IC required).
- 1N4007 was chosen over the more common 1N4001 for additional reverse-voltage margin on the flyback protection.
