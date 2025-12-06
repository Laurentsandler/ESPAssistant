# Wiring Diagram

This document describes the hardware connections for the ESP32 AI Assistant.

## Component Connections

### INMP441 I2S Microphone
Connect the INMP441 microphone to the ESP32 as follows:

```
INMP441 Pin  →  ESP32 Pin
--------------------------
VDD          →  3.3V
GND          →  GND
WS (Word Select) → GPIO 4
SD (Serial Data) → GPIO 2
SCK (Serial Clock) → GPIO 3
L/R          →  GND (for left channel)
```

### SH1106/SSD1306 OLED Display (I2C)
Connect the 128x64 OLED display to the ESP32:

```
OLED Pin  →  ESP32 Pin
-----------------------
VCC       →  3.3V
GND       →  GND
SDA       →  GPIO 5
SCL       →  GPIO 6
```

### Buttons
Connect momentary push buttons with internal pull-up resistors:

```
Button          →  ESP32 Pin  →  Function
-------------------------------------------
Button 1        →  GPIO 7     →  Main button (menu navigation, recording)
Button 2        →  GPIO 8     →  Scroll button (menu navigation, scrolling)
Button 3        →  GPIO 9     →  Assignment button (voice assignments)
Button 4        →  GPIO 1     →  Extra button (reserved for future use)
```

Each button should connect:
- One side to the GPIO pin
- Other side to GND
- The ESP32 will use INPUT_PULLUP mode (no external resistor needed)

## Circuit Diagram (ASCII Art)

```
                    ┌─────────────────────┐
                    │                     │
    INMP441         │     ESP32-C3        │        SH1106 OLED
    ┌──────┐        │   Seed Studio       │         ┌────────┐
    │ VDD  ├────────┤ 3.3V                │         │  VCC   │
    │ GND  ├────────┤ GND         GPIO 5  ├─────────┤  SDA   │
    │ WS   ├────────┤ GPIO 4      GPIO 6  ├─────────┤  SCL   │
    │ SD   ├────────┤ GPIO 2              │         │  GND   │
    │ SCK  ├────────┤ GPIO 3              │         └────────┘
    │ L/R  ├────┐   │                     │
    └──────┘    │   │                     │
                │   │                     │
               GND  │                     │
                    │                     │
                    │  GPIO 7 ├───[BTN1]──┤ GND
                    │  GPIO 8 ├───[BTN2]──┤ GND
                    │  GPIO 9 ├───[BTN3]──┤ GND
                    │  GPIO 1 ├───[BTN4]──┤ GND
                    │                     │
                    └─────────────────────┘
```

## Power Supply

- The ESP32 can be powered via USB-C (if your board has USB-C) or Micro-USB
- All components (INMP441, OLED) are powered from the ESP32's 3.3V regulator
- Ensure your power supply can provide at least 500mA for stable operation

## Notes

- The INMP441 L/R pin is connected to GND to select the left channel
- All buttons use the ESP32's internal pull-up resistors (INPUT_PULLUP mode)
- I2C address for the OLED is typically 0x3C (defined in the code)
- Ensure proper grounding - connect all GND pins together

## Testing the Hardware

After wiring, you can test each component:

1. **OLED Display**: Upload the sketch and check if the initialization screen appears
2. **Buttons**: Press each button and check the Serial Monitor for debug output
3. **Microphone**: Try recording in AI mode and check the serial output for sample count
4. **WiFi**: Verify connection on the OLED and check the IP address on Serial Monitor

## Troubleshooting

| Issue | Possible Cause | Solution |
|-------|----------------|----------|
| OLED not displaying | Wrong I2C address or wiring | Check I2C scanner, verify SDA/SCL pins |
| No audio recording | Incorrect I2S pins | Verify WS, SD, SCK connections |
| Buttons not responding | Pull-up not enabled | Check pinMode(pin, INPUT_PULLUP) in code |
| WiFi not connecting | Wrong credentials | Update WIFI_SSID and WIFI_PASS |
