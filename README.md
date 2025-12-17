# ESP32 Robot Companion

A friendly ESP32 robot with animated eyes, person detection, and weather updates that sits on your desk and interacts with you!

##  Features

- **Animated Robot Eyes** with different moods (happy, sad, angry, tired)
- **Person Detection** using VL53L0X distance sensor with greeting messages
- **Weather Display** with real-time data from OpenMeteo API
- **Time & Date** display with NTP synchronization
- **Servo Control** with automatic movement patterns
- **WiFi Connectivity** with status indicators
- **Interactive Greetings** - says hello when you approach!

##  Hardware Components

- ESP32 Development Board
- SH1106 OLED Display (128x64, I2C)
- VL53L0X Distance Sensor (I2C)
- Servo Motor (SG90 or similar)
- Jumper wires and breadboard
- Power supply (5V recommended for servo)

##  Wiring Diagram

| Component | ESP32 Pin | Power | Notes |
|-----------|-----------|--------|-------|
| **OLED Display** | | | |
| SDA | GPIO 21 | 3.3V | I2C Data |
| SCL | GPIO 22 | 3.3V | I2C Clock |
| **VL53L0X Sensor** | | | |
| SDA | GPIO 21 | 3.3V | Shared I2C Bus |
| SCL | GPIO 22 | 3.3V | Shared I2C Bus |
| **Servo Motor** | | | |
| Signal | GPIO 13 | - | PWM Control |
| Power | - | 5V | External power recommended |
| **Common** | | | |
| GND | GND | GND | All components |

## 📚 Required Libraries

### Standard Libraries (Install via Arduino IDE Library Manager)
```
✓ WiFi (ESP32 Core)
✓ HTTPClient (ESP32 Core) 
✓ ArduinoJson by Benoit Blanchon
✓ ESP32Servo by Kevin Harrington
✓ Adafruit GFX Library
✓ Adafruit SH110X
✓ VL53L0X by Pololu
```

### External Library (Manual Installation Required)
**FluxGarage_RoboEyes** - For animated robot eyes
- **Repository**: https://github.com/FluxGarage/RoboEyes
- **Installation Steps**:
  1. Download ZIP from GitHub repository
  2. Arduino IDE → Sketch → Include Library → Add .ZIP Library
  3. Select the downloaded ZIP file
  4. Restart Arduino IDE

**Note**: If you can't find this library, see the "Library Issues" section below for alternatives.

##  Quick Setup

### 1. Hardware Setup
- Connect components according to wiring diagram
- Ensure stable power supply (especially for servo)
- Double-check I2C connections

### 2. Software Installation
```bash
# Install all required libraries listed above
# Don't forget the external FluxGarage_RoboEyes library!
```

### 3. Configure WiFi
```cpp
// In the Arduino code, update these lines:
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 4. Configure Location (Optional)
```cpp
// Update coordinates for your location:
const char* weatherCity = "YourCity";
const char* weatherUrl = "http://api.open-meteo.com/v1/forecast?latitude=YOUR_LAT&longitude=YOUR_LON&current_weather=true&temperature_unit=celsius";
```

### 5. Upload and Test
1. Select ESP32 board in Arduino IDE
2. Choose correct COM port
3. Upload the code
4. Open Serial Monitor (115200 baud) for debugging

##  How It Works

### Normal Operation Cycle
1. **Robot Eyes** - Shows animated eyes with random mood changes (10 seconds)
2. **Time Display** - Current time and date (3 seconds)
3. **Quote Display** - Time-based funny messages (3 seconds)
4. **Weather Info** - Temperature and conditions (2 seconds)
5. **Repeat** - Cycles through all modes continuously

### Interactive Mode
- **Person Detection** - When someone approaches (within 25cm)
- **Greeting Display** - Shows random friendly message for 5 seconds
- **Happy Mood** - Eyes switch to happy expression
- **Resume Normal** - Returns to normal cycle after person leaves

### Status Indicators
- **Built-in LED** - ON = WiFi connected, OFF = No WiFi
- **Serial Monitor** - Shows distance readings, weather updates, memory usage

##  Configuration Options

### Distance Detection Sensitivity
```cpp
#define DETECTION_THRESHOLD 250  // Distance in mm (default: 25cm)
```

### Display Settings
```cpp
#define i2c_Address 0x3c  // Try 0x3d if display shows nothing
```

### Servo Movement
```cpp
const unsigned long servoActiveTime = 300000;  // 5 minutes active
const unsigned long servoRestTime = 60000;     // 1 minute rest
```

### Weather Update Frequency
```cpp
const unsigned long weatherUpdateInterval = 600000;  // 10 minutes
```

##  Troubleshooting

### Display Issues
- **Blank screen**: Try changing `i2c_Address` from `0x3c` to `0x3d`
- **Garbled display**: Check I2C wiring (SDA/SCL connections)
- **Partial display**: Verify power supply and connections

### Library Issues
- **FluxGarage_RoboEyes not found**:
  - Check if library is properly installed in Arduino/libraries folder
  - Try restarting Arduino IDE
  - Verify ZIP file downloaded correctly from: https://github.com/FluxGarage/RoboEyes
  - **Alternative Solution**: Remove robot eyes and use basic circles:
    ```cpp
    // Replace RoboEyes functions with:
    void drawBasicEyes() {
      display.fillCircle(30, 30, 15, SH110X_WHITE);
      display.fillCircle(90, 30, 15, SH110X_WHITE);
      display.fillCircle(30, 30, 8, SH110X_BLACK);  // pupils
      display.fillCircle(90, 30, 8, SH110X_BLACK);
    }
    
    // Comment out all roboEyes.* function calls
    // Replace with drawBasicEyes() function
    ```
  - **Simple Fix**: Comment out lines 21-23 and all roboEyes functions if you want to skip animated eyes entirely

### Sensor Problems
- **No person detection**: 
  - Verify VL53L0X wiring and power
  - Check Serial Monitor for distance readings
  - Sensor will be disabled if initialization fails
- **Distance readings wrong**: Adjust `DETECTION_THRESHOLD` value

### WiFi Connection
- **Won't connect**: 
  - Verify SSID and password
  - Check signal strength
  - Try 2.4GHz network (ESP32 doesn't support 5GHz)
- **Connects then disconnects**: Check power supply stability

### Servo Issues
- **Not moving**: 
  - Verify GPIO 13 connection
  - Check power supply (servo needs adequate current)
  - Servo has automatic rest periods every 5 minutes
- **Jittery movement**: Use external 5V power supply

### Serial Monitor Debug Info
Enable Serial Monitor (115200 baud) to see:
- WiFi connection status
- Distance sensor readings
- Weather API responses
- Memory usage reports
- Error messages

## 🔧 Advanced Configuration

### Adding Custom Greetings
```cpp
const char* greetingMessages[] = {
  "Hi!",
  "Hello there!",
  "Your custom greeting!",
  // Add more messages here...
};
```

### Changing Robot Personality
```cpp
// Available moods: DEFAULT, TIRED, ANGRY, HAPPY
// Modify mood change frequency:
nextMoodInterval = random(3000, 7000);  // 3-7 seconds
```

### Multiple Weather Locations
See DOCUMENTATION.md for advanced weather configuration.

## 📖 Documentation

For detailed technical information, architecture details, and advanced configuration, see [DOCUMENTATION.md](DOCUMENTATION.md).

##  Contributing

1. Fork the repository
2. Create your feature branch
3. Test thoroughly with real hardware
4. Submit a pull request with detailed description

##  License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

##  Acknowledgments

- **FluxGarage** - For the excellent RoboEyes library
- **OpenMeteo** - For free weather API service
- **Adafruit** - For reliable display libraries
- **Pololu** - For VL53L0X sensor library
- **ESP32 Community** - For extensive documentation and support

##  Future Enhancements

- [ ] Voice command recognition
- [ ] Mobile app for remote control
- [ ] RGB LED mood lighting
- [ ] Multiple personality modes
- [ ] Custom animation sequences
- [ ] Temperature/humidity sensors
- [ ] Scheduled messages and reminders

##  Support

- **Issues**: Open an issue in this repository
- **Discussions**: Use GitHub Discussions for questions
- **Hardware Help**: Check component datasheets and ESP32 documentation

---
**Built with ❤️ for interactive robotics and IoT projects**

*Your desk companion is ready to greet you!* 
