# 🌡️ Micro Weather Station — ESP32-C3

**A pocket weather station built from bare silicon up.** An ESP32-C3
reads live temperature and humidity from an SHTC3 sensor and paints
them onto a 16×2 RGB LCD — two I2C devices, one shared bus, zero
libraries doing the heavy lifting. Every LCD command is written at the
register level.

## What it does

- Reads the SHTC3 sensor (I2C `0x70`) once per second, waking it for
  each measurement and putting it back to sleep afterwards
- Drives a DFRobot 16x2 RGB LCD (AiP31068 controller, I2C `0x3E`) at
  the register level — command/data writes, cursor positioning, and
  line updates padded so stale digits never linger
- Prints the same readings to the serial monitor

```
Temp: 24C  Hum: 41%
```

## Hardware

- ESP32-C3 board
- SHTC3 temperature/humidity sensor
- DFRobot 16x2 RGB LCD (I2C)
- Shared I2C bus: SDA on GPIO 7, SCL on GPIO 8, 100 kHz

## Build and flash

Requires ESP-IDF 6.1.

```bash
idf.py set-target esp32c3
idf.py build flash monitor
```

## Files

All of the code lives in one source file:

- `main/weather_station.cpp` — sensor reads, LCD driver, main loop
- `sdkconfig.defaults` — target selection (ESP32-C3)
- The two `CMakeLists.txt` files are the standard ESP-IDF build glue
