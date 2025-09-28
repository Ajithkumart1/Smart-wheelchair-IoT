# Blynk Mobile App Configuration Guide

## Overview
This guide helps you set up the Blynk mobile app to control and monitor your IoT Smart Wheelchair project.

## Prerequisites
- Blynk mobile app installed (iOS/Android)
- ESP8266 programmed with Blynk library
- WiFi network credentials
- Blynk authentication token

## Step 1: Create New Blynk Project

### 1.1 Project Setup
1. Open Blynk app and tap "New Project"
2. **Project Name**: "Smart Wheelchair"
3. **Device**: ESP8266
4. **Connection**: WiFi
5. **Theme**: Dark (recommended for better visibility)
6. Tap "Create Project"
7. **Save the Auth Token** - you'll need this for ESP8266 programming

### 1.2 Project Settings
- **Project Name**: Smart Wheelchair Control
- **Device**: Generic ESP8266 Board
- **Connection Type**: WiFi
- **Template**: Custom (create your own layout)

## Step 2: Widget Configuration

### 2.1 Control Widgets

#### Joystick Widget (Virtual Pin V1)
- **Widget**: Joystick
- **Virtual Pin**: V1 
- **Label**: "Direction Control"
- **Position**: Top center of screen
- **Size**: 2x2
- **Settings**:
  - Mode: PUSH (not SWITCH)
  - Auto-return: ON
  - Rotate on tilt: OFF

#### Speed Slider (Virtual Pin V2)
- **Widget**: Slider  
- **Virtual Pin**: V2
- **Label**: "Motor Speed"
- **Position**: Below joystick
- **Size**: 2x1
- **Settings**:
  - Min Value: 0
  - Max Value: 9
  - Send on Release: OFF
  - Write Interval: 100ms

#### Control Buttons (Virtual Pins V3-V7)
Create separate button widgets:

**Forward Button (V3)**
- Virtual Pin: V3
- Label: "Forward"
- Mode: PUSH
- Size: 1x1

**Backward Button (V4)**  
- Virtual Pin: V4
- Label: "Backward"
- Mode: PUSH
- Size: 1x1

**Left Button (V5)**
- Virtual Pin: V5  
- Label: "Left"
- Mode: PUSH
- Size: 1x1

**Right Button (V6)**
- Virtual Pin: V6
- Label: "Right"  
- Mode: PUSH
- Size: 1x1

**Stop Button (V7)**
- Virtual Pin: V7
- Label: "STOP"
- Mode: PUSH
- Size: 1x1
- Color: Red (for emergency identification)

### 2.2 Monitoring Widgets

#### Distance Display (Virtual Pin V10)
- **Widget**: Value Display
- **Virtual Pin**: V10
- **Label**: "Distance (cm)"
- **Position**: Right side, top
- **Size**: 1x1
- **Settings**:
  - Reading Frequency: 1 sec
  - Color: Blue
  - Text Size: Large

#### Heart Rate Gauge (Virtual Pin V11)
- **Widget**: Gauge
- **Virtual Pin**: V11
- **Label**: "Heart Rate (BPM)"
- **Position**: Right side, middle
- **Size**: 2x2
- **Settings**:
  - Min Value: 50
  - Max Value: 180
  - Reading Frequency: 1 sec
  - Color: Red

#### Direction Status (Virtual Pin V12)
- **Widget**: LCD
- **Virtual Pin**: V12
- **Label**: "Current Direction"
- **Position**: Bottom center
- **Size**: 2x1
- **Settings**:
  - Advanced: ON
  - Text Size: Medium

#### System Status LED (Virtual Pin V13)
- **Widget**: LED
- **Virtual Pin**: V13
- **Label**: "System Online"
- **Position**: Top right corner
- **Size**: 1x1
- **Color**: Green when active

### 2.3 History & Data Widgets

#### Heart Rate History (Virtual Pin V11)
- **Widget**: History Graph
- **Virtual Pin**: V11 (same as heart rate)
- **Label**: "BPM Trends"
- **Position**: Bottom section
- **Size**: 4x2
- **Settings**:
  - Time Range: 1 hour
  - Y-axis: 50-180
  - Update: 1 sec

## Step 3: Virtual Pin Mapping

### 3.1 Control Pins (App → ESP8266 → PIC)
| Virtual Pin | Function | Data Type | Range | PIC Command |
|-------------|----------|-----------|-------|-------------|
| V1 | Joystick | x,y coordinates | 0-255 each | F/B/L/R |
| V2 | Speed Control | Integer | 0-9 | Speed Level |
| V3 | Forward Button | Digital | 0/1 | 'F' |
| V4 | Backward Button | Digital | 0/1 | 'B' |
| V5 | Left Button | Digital | 0/1 | 'L' |
| V6 | Right Button | Digital | 0/1 | 'R' |
| V7 | Stop Button | Digital | 0/1 | 'S' |

### 3.2 Monitoring Pins (PIC → ESP8266 → App)
| Virtual Pin | Function | Data Type | Update Rate | Source |
|-------------|----------|-----------|-------------|---------|
| V10 | Distance | Integer (cm) | 1 second | HC-SR04 |
| V11 | Heart Rate | Integer (BPM) | 1 second | Pulse Sensor |
| V12 | Direction | String | 1 second | Motor State |
| V13 | System Status | Digital | 1 second | System Health |

## Step 4: ESP8266 Code Configuration

### 4.1 Required Libraries
```cpp
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>
```

### 4.2 Blynk Credentials
```cpp
// Your Blynk auth token
char auth[] = "YourAuthTokenHere";

// WiFi credentials  
char ssid[] = "YourWiFiName";
char pass[] = "YourWiFiPassword";
```

### 4.3 ESP8266 to PIC Communication
```cpp
// Serial communication with PIC16F877A
SoftwareSerial picSerial(D2, D3); // RX, TX pins
```

### 4.4 Virtual Pin Handlers
```cpp
// Control handlers (App to PIC)
BLYNK_WRITE(V3) { // Forward
  if(param.asInt()) picSerial.write('F');
}

BLYNK_WRITE(V4) { // Backward  
  if(param.asInt()) picSerial.write('B');
}

BLYNK_WRITE(V5) { // Left
  if(param.asInt()) picSerial.write('L');
}

BLYNK_WRITE(V6) { // Right
  if(param.asInt()) picSerial.write('R');
}

BLYNK_WRITE(V7) { // Stop
  if(param.asInt()) picSerial.write('S');
}

BLYNK_WRITE(V2) { // Speed control
  int speed = param.asInt();
  picSerial.write('0' + speed);
}
```

## Step 5: Data Parsing & Display

### 5.1 Data Format from PIC
```
Format: "DIST:XXcm,BPM:XXX,DIR:XXXXXXX,SPD:XXX"
Example: "DIST:45cm,BPM:72,DIR:Forward,SPD:150"
```

### 5.2 ESP8266 Data Processing
```cpp
void processPICData(String data) {
  // Parse distance
  int distStart = data.indexOf("DIST:") + 5;
  int distEnd = data.indexOf("cm");
  int distance = data.substring(distStart, distEnd).toInt();
  Blynk.virtualWrite(V10, distance);
  
  // Parse heart rate
  int bpmStart = data.indexOf("BPM:") + 4;
  int bpmEnd = data.indexOf(",", bpmStart);
  int heartRate = data.substring(bpmStart, bpmEnd).toInt();
  Blynk.virtualWrite(V11, heartRate);
  
  // Parse direction
  int dirStart = data.indexOf("DIR:") + 4;
  int dirEnd = data.indexOf(",", dirStart);
  String direction = data.substring(dirStart, dirEnd);
  Blynk.virtualWrite(V12, direction);
  
  // System status (online)
  Blynk.virtualWrite(V13, 255); // LED ON
}
```

## Step 6: App Layout Design

### 6.1 Recommended Screen Layout
```
┌─────────────────────────────────────┐
│ [System LED]              [Distance]│
│                                     │
│        [Joystick]         [Heart    │
│                           Rate      │
│                           Gauge]    │
│     [Speed Slider]                  │
│                                     │
│ [F] [B] [L] [R] [STOP]             │
│                                     │
│        [Direction Status]           │
│                                     │
│        [Heart Rate Graph]           │
└─────────────────────────────────────┘
```

### 6.2 Color Scheme
- **Background**: Dark theme
- **Control buttons**: Blue
- **Emergency stop**: Red
- **Status indicators**: Green
- **Heart rate**: Red/Orange
- **Distance**: Blue
- **Text**: White

## Step 7: Testing & Calibration

### 7.1 Connection Test
1. Power on wheelchair system
2. Open Blynk app
3. Check "System Online" LED (should be green)
4. Verify all sensor readings update every second

### 7.2 Control Test  
1. Test each direction button (F/B/L/R)
2. Verify stop button works immediately
3. Test speed slider (0-9 levels)
4. Check joystick control responsiveness

### 7.3 Monitoring Test
1. **Distance**: Move obstacle near sensor
2. **Heart Rate**: Place finger on pulse sensor
3. **Direction**: Verify status updates with movement
4. **History**: Check heart rate graph updates

## Step 8: Troubleshooting

### 8.1 Common Issues

#### App Not Connecting
- Check WiFi credentials in ESP8266 code
- Verify auth token is correct
- Ensure ESP8266 and phone on same network

#### Controls Not Working
- Check virtual pin assignments
- Verify ESP8266 to PIC serial connection
- Test UART communication manually

#### No Sensor Data
- Check PIC to ESP8266 data format
- Verify parsing code in ESP8266
- Test sensors individually

### 8.2 Debug Methods
- Use Blynk terminal widget for debugging
- Monitor ESP8266 serial output
- Check PIC UART transmission
- Test components separately

## Step 9: Advanced Features

### 9.1 Notifications
- Set up heart rate alerts (high/low BPM)
- Obstacle detection notifications
- Low battery warnings (if implemented)

### 9.2 Data Logging
- Enable Blynk cloud data storage
- Export heart rate trends
- Monitor usage patterns

### 9.3 Geolocation (Future Enhancement)
- Add GPS coordinates display
- Location tracking for outdoor use
- Emergency location sharing

## Step 10: Safety Configuration

### 10.1 Emergency Features
- **Emergency Stop**: Always accessible, red color
- **Auto-disconnect**: Stop motors if connection lost
- **Manual Override**: Physical controls take priority

### 10.2 User Interface Safety
- Large buttons for easy access
- High contrast colors
- Clear status indicators
- Audio feedback (if supported)

## Final Checklist

- [ ] Blynk project created with correct device type
- [ ] All widgets added with proper virtual pins
- [ ] ESP8266 programmed with auth token
- [ ] WiFi credentials configured
- [ ] Virtual pin mapping matches code
- [ ] Data parsing functions implemented
- [ ] All controls tested and working
- [ ] Sensor monitoring displays correctly
- [ ] Emergency stop functions properly
- [ ] App layout is user-friendly and accessible

## Support Resources

- **Blynk Documentation**: https://docs.blynk.io/
- **ESP8266 Arduino Core**: https://github.com/esp8266/Arduino
- **Blynk Community**: https://community.blynk.cc/
- **Project GitHub**: [https://github.com/Ajithkumart1/Smart-wheelchair-IoT]

---

**Note**: Save your Blynk auth token securely - you'll need it to program the ESP8266. Test all safety features thoroughly before actual use.
