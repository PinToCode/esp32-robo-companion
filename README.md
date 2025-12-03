# ESP32 Smart Display with Robot Eyes

An ESP32-based smart display featuring animated robot eyes, person detection, weather updates, and time display.

## Features

- 🤖 **Animated Robot Eyes** with different moods (happy, sad, angry, tired)
- 👋 **Person Detection** using VL53L0X distance sensor with greeting messages
- 🌤️ **Weather Display** with real-time data from OpenMeteo API
- ⏰ **Time & Date** display with NTP synchronization
- 🎯 **Servo Control** with automatic movement patterns
- 📱 **WiFi Connectivity** with status indicators

## Hardware Components

- ESP32 Development Board
- SH1106 OLED Display (128x64, I2C)
- VL53L0X Distance Sensor (I2C)
- Servo Motor
- Jumper wires

## Quick Setup

1. **Install Libraries** (Arduino IDE → Library Manager):
   ```
   - ArduinoJson
   - ESP32Servo
   - Adafruit GFX Library
   - Adafruit SH110X
   - VL53L0X (Pololu)
   - FluxGarage_RoboEyes
   ```

2. **Wiring**:
   ```
   OLED & VL53L0X: SDA→GPIO21, SCL→GPIO22
   Servo: Signal→GPIO13
   Power: 3.3V and GND to all components
   ```

3. **Configure WiFi** in code:
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_PASSWORD";
   ```

4. **Upload** to ESP32 and enjoy!

## How It Works

- **Normal Mode**: Cycles through robot eyes → time → quotes → weather
- **Person Detected**: Shows random greeting message
- **WiFi Status**: Built-in LED indicates connection
- **Auto Updates**: Weather refreshes every 10 minutes

## Configuration

```cpp
// Person detection distance (in mm)
#define DETECTION_THRESHOLD 250

// Display I2C address (try 0x3d if 0x3c doesn't work)
#define i2c_Address 0x3c

// Location for weather (change coordinates)
const char* weatherUrl = "http://api.open-meteo.com/v1/forecast?latitude=12.87&longitude=74.88&current_weather=true&temperature_unit=celsius";
```

## Troubleshooting

- **Blank screen**: Try changing `i2c_Address` to `0x3d`
- **No weather**: Check WiFi connection and coordinates
- **No person detection**: Verify VL53L0X wiring
- **Servo issues**: Check power supply and GPIO13 connection

## Demo

The device automatically:
1. Shows startup animation
2. Connects to WiFi
3. Displays robot eyes with random moods
4. Shows current time and date
5. Displays contextual quotes
6. Shows weather information
7. Greets people when they approach

---
*Built with ❤️ for interactive displays*
