# Smart Wheelchair - Complete Setup Instructions

## Prerequisites

### Required Software
- **MPLAB X IDE** (latest version)
- **XC8 Compiler** (free version)
- **Proteus Design Suite** (for simulation)
- **Blynk Mobile App** (iOS/Android)
- **Arduino IDE** (for ESP8266 programming)

### Required Hardware
- All components listed in `components-used.md`
- PIC programmer (PICkit 3/4)
- Breadboard for prototyping
- Multimeter for testing
- Soldering equipment (for final assembly)

## Step 1: Hardware Assembly

### 1.1 Power Supply Setup
1. **Main Power Supply (5V)**
   ```
   12V Battery → 7805 Regulator → 5V Output
                ↓
   Add 100μF capacitor on input and output
   Add heat sink to 7805 if needed
   ```

2. **Power Distribution**
   - Connect VDD pins (11, 32) to +5V
   - Connect VSS pins (12, 31) to Ground
   - Add 100nF decoupling capacitor near VDD pins

### 1.2 Crystal Oscillator Circuit
1. Connect 20MHz crystal between OSC1 (pin 13) and OSC2 (pin 14)
2. Add 22pF capacitor from OSC1 to ground
3. Add 22pF capacitor from OSC2 to ground
4. Keep crystal leads as short as possible

### 1.3 Reset Circuit  
1. Connect MCLR (pin 1) to +5V through 10kΩ resistor
2. Add push button from MCLR to ground (optional reset button)
3. Add 100nF capacitor from MCLR to ground for noise immunity

## Step 2: Sensor Connections

### 2.1 HC-SR04 Ultrasonic Sensor
```
HC-SR04 Pin → PIC16F877A Pin → Function
VCC         → +5V            → Power supply
GND         → Ground         → Ground reference  
Trig        → RB0            → Trigger pulse
Echo        → RB1            → Echo pulse input
```

### 2.2 Pulse Sensor
```
Pulse Sensor → PIC16F877A Pin → Function
Signal       → RA2            → Analog input (ADC Channel 2)
+            → +5V            → Power supply
-            → Ground         → Ground reference
```

### 2.3 Analog Joystick
```
Joystick Pin → PIC16F877A Pin → Function
VRX          → RA0            → X-axis analog input
VRY          → RA1            → Y-axis analog input
SW           → RB3            → Push button input
+5V          → +5V            → Power supply
GND          → Ground         → Ground reference
```

## Step 3: Display & Feedback Setup

### 3.1 16x2 LCD Display (4-bit mode)
```
LCD Pin → PIC Pin → Function
VSS(1)  → Ground  → Ground
VDD(2)  → +5V     → Power
V0(3)   → Pot     → Contrast (10kΩ pot center tap)
RS(4)   → RD4     → Register Select
EN(6)   → RD5     → Enable
D4(11)  → RD0     → Data bit 4
D5(12)  → RD1     → Data bit 5
D6(13)  → RD2     → Data bit 6
D7(14)  → RD3     → Data bit 7
```

**Contrast Circuit:**
- Connect 10kΩ potentiometer between +5V and Ground
- Connect center tap to LCD V0 (pin 3)

### 3.2 Status LEDs
```
Component     → PIC Pin → Series Resistor
Pulse LED     → RC0     → 330Ω to +5V
Status LEDs   → Various → 330Ω to Ground
```

### 3.3 Buzzer
```
Buzzer Pin → PIC Pin → Function
Positive   → RB7     → Control signal
Negative   → Ground  → Ground reference
```

## Step 4: Motor Control System

### 4.1 L298N Motor Driver Connections
```
L298N Pin → PIC16F877A Pin → Function
IN1       → RC3            → Motor A direction 1
IN2       → RC4            → Motor A direction 2  
IN3       → RC5            → Motor B direction 1
IN4       → RB2            → Motor B direction 2
ENA       → RC1            → Motor A speed (PWM)
ENB       → RC2            → Motor B speed (PWM)
VCC       → +5V            → Logic power supply
GND       → Ground         → Ground reference
VS        → +12V           → Motor power supply
```

### 4.2 Motor Connections
```
Left Motor:
- Positive → L298N OUT1
- Negative → L298N OUT2

Right Motor:  
- Positive → L298N OUT3
- Negative → L298N OUT4
```

**Important**: Connect motor power ground to system ground!

## Step 5: ESP8266 WiFi Module Setup

### 5.1 Hardware Connections
```
ESP8266 Pin → Connection → Function
VCC         → +3.3V      → Power (NOT 5V!)
GND         → Ground     → Ground reference
TX          → RC7 (RX)   → Data to PIC (may need level shifter)
RX          → RC6 (TX)   → Data from PIC (may need level shifter)
EN          → +3.3V      → Enable (pull high)
GPIO0       → +3.3V      → Normal operation (pull high)
```

### 5.2 Level Shifter (5V ↔ 3.3V)
If using a level shifter module:
```
Level Shifter → Connections
HV            → +5V (PIC side)
LV            → +3.3V (ESP8266 side)  
GND           → Ground (both sides)
HV1           → RC6 (PIC TX)
LV1           → ESP8266 RX
HV2           → RC7 (PIC RX)  
LV2           → ESP8266 TX
```

### 5.3 ESP8266 Programming
1. **Install ESP8266 Board Package in Arduino IDE:**
   - File → Preferences → Additional Boards Manager URLs
   - Add: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Tools → Board → Boards Manager → Search "ESP8266" → Install

2. **Install Blynk Library:**
   - Sketch → Include Library → Manage Libraries
   - Search "Blynk" → Install

3. **Basic ESP8266 Code:**
```cpp
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Blynk credentials
char auth[] = "YourBlynkAuthToken";
char ssid[] = "YourWiFiName";  
char pass[] = "YourWiFiPassword";

void setup() {
  Serial.begin(9600);  // Communication with PIC
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();
  
  // Handle PIC data
  if (Serial.available()) {
    String data = Serial.readString();
    // Parse and send to Blynk
    // (See blynk-setup-guide.md for complete code)
  }
}

// Control handlers
BLYNK_WRITE(V3) { if(param.asInt()) Serial.write('F'); }
BLYNK_WRITE(V4) { if(param.asInt()) Serial.write('B'); }
BLYNK_WRITE(V5) { if(param.asInt()) Serial.write('L'); }
BLYNK_WRITE(V6) { if(param.asInt()) Serial.write('R'); }
BLYNK_WRITE(V7) { if(param.asInt()) Serial.write('S'); }
BLYNK_WRITE(V2) { Serial.write('0' + param.asInt()); }
```

## Step 6: PIC16F877A Programming

### 6.1 MPLAB X IDE Setup
1. **Create New Project:**
   - File → New Project → Standalone Project
   - Device: PIC16F877A
   - Compiler: XC8

2. **Add Source File:**
   - Copy your `main.c` file to project
   - Build project (Production → Build Main Project)

### 6.2 Programming the PIC
1. **Connect PICkit Programmer:**
   ```
   PICkit Pin → PIC16F877A Pin → Function
   1 (MCLR)   → Pin 1          → Master Clear
   2 (VCC)    → Pin 11/32      → Power
   3 (VSS)    → Pin 12/31      → Ground
   4 (PGD)    → Pin 40 (RB7)   → Program Data
   5 (PGC)    → Pin 39 (RB6)   → Program Clock
   ```

2. **Programming Steps:**
   - Connect PICkit to computer via USB
   - Power the target circuit
   - Click "Make and Program Device" in MPLAB X
   - Verify successful programming

## Step 7: System Testing

### 7.1 Power-On Test
1. **Visual Inspection:**
   - Check all connections
   - Verify no short circuits
   - Ensure proper power supply voltages

2. **Initial Power-Up:**
   - Connect power supply
   - Check LCD displays "Smart Wheelchair Initializing..."
   - Verify system LED indicators

### 7.2 Sensor Testing
1. **Distance Sensor:**
   - Move object in front of sensor
   - Verify LCD shows changing distance values
   - Test emergency stop at 25cm threshold

2. **Pulse Sensor:**
   - Place finger gently on sensor
   - Wait for stable BPM reading (may take 30 seconds)
   - Verify pulse LED flashes with heartbeat

3. **Joystick Test:**
   - Move joystick in all directions
   - Verify LCD shows direction changes
   - Test emergency stop button

### 7.3 Motor Control Testing
1. **Manual Control:**
   - Test joystick motor control
   - Verify smooth speed changes
   - Check obstacle detection stops motors

2. **Remote Control:**
   - Connect to WiFi network
   - Test Blynk app controls
   - Verify bidirectional communication

## Step 8: Calibration

### 8.1 Sensor Calibration
1. **Distance Sensor:**
   - Measure actual distances
   - Compare with LCD readings
   - Adjust MIN_DISTANCE if needed

2. **Pulse Sensor:**
   - Test with multiple people
   - Verify BPM accuracy with fitness tracker
   - Allow time for adaptive calibration

### 8.2 Motor Calibration  
1. **Speed Settings:**
   - Test all 10 speed levels
   - Ensure motors start reliably at lowest setting
   - Check for smooth acceleration

2. **Direction Control:**
   - Verify forward/backward motion
   - Test left/right turning
   - Check motor synchronization

## Step 9: Final Assembly & Mounting

### 9.1 Permanent Assembly
1. **PCB Assembly (Recommended):**
   - Transfer circuit from breadboard to PCB
   - Use proper soldering techniques
   - Add test points for debugging
   - Include component labels

2. **Wire Management:**
   - Use appropriate wire gauges
   - Bundle related wires together
   - Secure with cable ties
   - Leave service loops for maintenance

3. **Enclosure Installation:**
   - Mount control PCB in weatherproof enclosure
   - Install LCD display on enclosure front
   - Add ventilation holes if needed
   - Secure all cable entries

### 9.2 Wheelchair Integration
1. **Control Box Mounting:**
   - Secure enclosure to wheelchair frame
   - Position for easy access to display/controls
   - Ensure protection from damage
   - Allow for easy removal if needed

2. **Motor Installation:**
   - Mount motors securely to wheelchair frame
   - Ensure proper wheel alignment
   - Check for adequate ground clearance
   - Verify smooth wheel rotation

3. **Sensor Positioning:**
   - Mount ultrasonic sensor facing forward
   - Position at appropriate height (avoid ground reflection)
   - Secure pulse sensor for easy finger access
   - Mount joystick within comfortable reach

## Step 10: Safety Testing

### 10.1 Emergency Stop Testing
1. **Hardware Emergency Stop:**
   - Test joystick button response (<100ms)
   - Verify motors stop immediately
   - Check system can be restarted

2. **Software Emergency Stop:**
   - Test Blynk app stop button
   - Verify WiFi disconnect stops motors
   - Test obstacle detection override

3. **Power Loss Test:**
   - Simulate power interruption
   - Verify motors stop immediately
   - Check system restarts properly

### 10.2 Range & Performance Testing
1. **WiFi Range Test:**
   - Test connection at various distances
   - Verify consistent control response
   - Check for interference issues

2. **Battery Life Test:**
   - Measure current consumption
   - Test under various load conditions
   - Estimate operating time

3. **Reliability Test:**
   - Run system for extended periods
   - Monitor for overheating
   - Check for loose connections

## Step 11: Documentation & Backup

### 11.1 Create System Documentation
1. **As-Built Drawings:**
   - Final schematic with part numbers
   - PCB layout (if custom PCB made)
   - Mechanical drawings/photos
   - Wire identification chart

2. **Configuration Records:**
   - Blynk auth token (keep secure!)
   - WiFi credentials used
   - Calibration values
   - PIN configurations

3. **User Manual:**
   - Operating instructions
   - Safety warnings
   - Troubleshooting guide
   - Maintenance procedures

### 11.2 Code Backup
1. **Source Code:**
   - PIC16F877A firmware (main.c)
   - ESP8266 code
   - Blynk project configuration
   - Hex files for programming

2. **Simulation Files:**
   - Complete Proteus project
   - Component libraries used
   - Test vectors/results

## Step 12: Troubleshooting Guide

### 12.1 Common Issues

#### System Won't Start
**Symptoms:** No LCD display, no response
**Checks:**
- Verify power supply voltages (5V, 12V)
- Check crystal oscillator circuit
- Verify MCLR pull-up resistor
- Test PIC programming

#### LCD Display Issues
**Symptoms:** No display, garbled text, wrong characters
**Solutions:**
- Adjust contrast potentiometer
- Check all LCD connections
- Verify 4-bit mode wiring
- Test LCD with simple code

#### Motor Control Problems
**Symptoms:** Motors don't run, run in wrong direction, inconsistent speed
**Solutions:**
- Check L298N power connections
- Verify motor driver input signals
- Test PWM output with oscilloscope
- Check motor power supply voltage

#### Sensor Reading Issues
**Symptoms:** No distance readings, wrong BPM, erratic values
**Solutions:**
- Check sensor power supply
- Verify ADC reference voltage
- Test sensors individually
- Check for electrical noise

#### WiFi Connection Problems
**Symptoms:** Can't connect to Blynk, intermittent control
**Solutions:**
- Verify WiFi credentials
- Check ESP8266 power supply (3.3V!)
- Test level shifter operation
- Monitor serial communication

### 12.2 Debug Tools

#### Multimeter Testing
- Power supply voltages
- Signal continuity
- Ground connections
- Current consumption

#### Oscilloscope Analysis (if available)
- PWM signals
- UART communication
- Crystal oscillator
- Sensor signal quality

#### Software Debugging
- UART terminal monitoring
- Blynk serial debug
- LED status indicators
- Step-by-step code testing

## Step 13: Maintenance Schedule

### 13.1 Regular Checks (Weekly)
- [ ] Battery voltage/charge level
- [ ] All connections secure
- [ ] Sensor cleaning (especially pulse sensor)
- [ ] Motor operation smoothness
- [ ] WiFi connection reliability

### 13.2 Periodic Maintenance (Monthly)
- [ ] Complete system functional test
- [ ] Calibration verification
- [ ] Enclosure inspection for damage
- [ ] Wire insulation check
- [ ] Update software if needed

### 13.3 Annual Maintenance
- [ ] Complete disassembly and inspection
- [ ] Component replacement if worn
- [ ] Calibration reset and verification
- [ ] Documentation update
- [ ] Safety system verification

## Step 14: Upgrades & Enhancements

### 14.1 Immediate Improvements
- **Battery Monitor:** Add voltage monitoring
- **Speed Display:** Show current speed on LCD
- **Data Logging:** Store sensor data to SD card
- **Audio Feedback:** Add voice announcements

### 14.2 Advanced Features
- **GPS Navigation:** Add location tracking
- **Camera System:** Computer vision obstacle detection
- **Voice Control:** Speech recognition interface
- **Mobile App Enhancement:** Custom app development

## Step 15: Safety Reminders

### 15.1 Before Each Use
- [ ] Check emergency stop function
- [ ] Verify obstacle detection works
- [ ] Test manual controls
- [ ] Check battery level
- [ ] Ensure WiFi connection stable

### 15.2 Safety Guidelines
- **Never disable emergency stop systems**
- **Always test in safe environment first**
- **Keep manual override accessible**
- **Monitor battery level during use**
- **Have backup control method ready**

### 15.3 Emergency Procedures
1. **System Malfunction:**
   - Press emergency stop immediately
   - Switch to manual wheelchair operation
   - Power down electronics if needed

2. **Medical Emergency:**
   - Use heart rate alerts appropriately
   - Don't rely solely on pulse sensor for medical decisions
   - Seek professional medical help when needed

## Final Checklist

### Hardware Assembly
- [ ] All connections verified and secure
- [ ] Power supplies tested and stable
- [ ] Sensors calibrated and responsive
- [ ] Motors operating smoothly
- [ ] Emergency stops functional
- [ ] Enclosure properly sealed

### Software Configuration  
- [ ] PIC16F877A programmed successfully
- [ ] ESP8266 configured for WiFi
- [ ] Blynk app setup and tested
- [ ] All virtual pins mapped correctly
- [ ] Communication protocols working
- [ ] Data transmission verified

### Testing & Validation
- [ ] All safety systems tested
- [ ] Range and performance verified
- [ ] Reliability testing completed
- [ ] User interface validated
- [ ] Documentation complete
- [ ] Backup procedures established

### Ready for Operation
- [ ] User training completed
- [ ] Maintenance schedule established  
- [ ] Emergency procedures understood
- [ ] Support contacts available
- [ ] System is safe and reliable

---

## Support & Resources

### Technical Support
- **GitHub Repository:** [Your project repository]
- **Documentation:** All files included in project
- **Community:** Electronics forums and communities

### Emergency Contact
- **Technical Issues:** [Your contact information]
- **Medical Emergencies:** Contact appropriate emergency services
- **Hardware Problems:** Local electronics technician

**⚠️ IMPORTANT SAFETY NOTE**
This system is intended for controlled environments and experienced users. Always prioritize safety over functionality. Test thoroughly before real-world use. Never bypass safety systems.