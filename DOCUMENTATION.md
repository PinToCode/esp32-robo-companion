# ESP32 Smart Display - Technical Documentation

## Project Overview

This project creates an interactive smart display using ESP32 that combines entertainment, information, and person interaction through animated robot eyes and sensor-based detection.

## System Architecture

### Core Components
- **ESP32**: Main microcontroller handling WiFi, processing, and sensor management
- **SH1106 OLED**: 128x64 pixel display for visual output
- **VL53L0X**: Time-of-flight distance sensor for person detection
- **Servo Motor**: Provides physical movement for enhanced interactivity
- **FluxGarage RoboEyes**: Library for animated eye expressions

### Communication Protocols
- **I2C**: OLED display and distance sensor (shared bus)
- **PWM**: Servo motor control
- **HTTP**: Weather data retrieval
- **NTP**: Time synchronization

## Pin Configuration

| Function | GPIO | Component | Notes |
|----------|------|-----------|--------|
| I2C SDA | 21 | OLED + VL53L0X | Shared bus |
| I2C SCL | 22 | OLED + VL53L0X | Shared bus |
| Servo PWM | 13 | Servo Motor | 50Hz PWM signal |
| Status LED | 2 | Built-in LED | WiFi indicator |

## Software Features

### Display Modes
1. **Robot Eyes**: Animated expressions with mood changes
2. **Time Display**: HH:MM:SS format with date
3. **Weather Info**: Temperature and conditions
4. **Quote Display**: Time-contextual messages
5. **Greeting Mode**: Triggered by person detection

### State Management
```cpp
// Main operational states
- Normal cycling mode
- Person detected mode
- WiFi connecting mode
- Error/offline mode
```

### Memory Optimization
- Static JSON document allocation
- Efficient string handling
- Periodic memory monitoring
- Servo rest periods to prevent overheating

## Configuration Parameters

### Distance Detection
```cpp
#define DETECTION_THRESHOLD 250  // mm - adjust for sensitivity
const unsigned long greetingDisplayTime = 5000;  // ms
```

### Network Settings
```cpp
const unsigned long weatherUpdateInterval = 600000;  // 10 minutes
const long gmtOffset_sec = 19800;  // IST timezone
```

### Display Settings
```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define i2c_Address 0x3c  // or 0x3d for some displays
```

## Data Flow

1. **Initialization**: WiFi → Time sync → Weather fetch → Sensor setup
2. **Main Loop**: Person check → Display update → Servo control → Data refresh
3. **Interrupt Handling**: Distance readings every 100ms
4. **Background Tasks**: Memory monitoring, time sync, weather updates

## API Integration

### Weather Service (OpenMeteo)
- **Endpoint**: `api.open-meteo.com/v1/forecast`
- **Parameters**: Latitude, longitude, current weather, temperature unit
- **Response**: JSON with temperature and weather code
- **Rate Limit**: Free tier, reasonable usage

### Time Service (NTP)
- **Servers**: pool.ntp.org, time.nist.gov, time.google.com
- **Sync Frequency**: Every hour
- **Timezone**: IST (GMT+5:30)

## Error Handling

### Sensor Failures
- VL53L0X timeout detection
- Graceful degradation without distance sensing
- Serial output for debugging

### Network Issues
- WiFi reconnection attempts
- Offline mode operation
- Visual status indicators

### Memory Management
- Heap monitoring
- JSON document reuse
- String length validation

## Performance Characteristics

### Power Consumption
- Active mode: ~200-300mA (with servo)
- Sleep periods during servo rest
- LED indicates high-power WiFi usage

### Response Times
- Person detection: <100ms
- Display updates: ~50ms
- Weather refresh: 2-5 seconds
- Servo movement: 1ms intervals

### Memory Usage
- Program storage: ~1MB
- RAM usage: ~100KB typical
- Free heap monitoring: >200KB maintained

## Customization Options

### Adding New Features
1. **New Display Pages**: Add to main loop cycle
2. **Custom Greetings**: Modify `greetingMessages[]` array
3. **Different Sensors**: Extend I2C bus
4. **Additional APIs**: Integrate in weather update cycle

### Behavior Modifications
- Adjust detection sensitivity
- Change display timing
- Modify servo movement patterns
- Add new robot eye moods

## Debugging Guide

### Serial Monitor (115200 baud)
- WiFi connection status
- Distance sensor readings
- Memory usage reports
- Weather update confirmations
- Error messages

### Common Issues
1. **Display not working**: Check I2C address and connections
2. **Distance sensor timeout**: Verify wiring and power
3. **WiFi connection failed**: Check credentials and signal
4. **Servo not moving**: Verify power supply capacity

## Future Enhancement Ideas

- **Voice commands** using microphone
- **RGB lighting** synchronized with moods
- **Web interface** for remote configuration
- **Multiple location** weather support
- **Bluetooth connectivity** for mobile app
- **External sensors** (temperature, humidity)
- **Time-based automation** (morning routines, sleep mode)

---
*Technical documentation for ESP32 Smart Display project*
