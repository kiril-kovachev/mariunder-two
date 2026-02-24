# Mariunder Eyes - ESP32-C6 Animated Eyes Project

An interactive animated eye display featuring 18 emotions, motion detection, audio feedback, and power management for ESP32-C6.

## Features

- **18 Different Emotions**: Normal, Happy, Angry, Sad, Surprised, Sleepy, Scared, Furious, Excited, Disappointed, Confused, Curious, Bored, Worried, Annoyed, Suspicious, Skeptical, Frustrated
- **Smooth Transitions**: Parametric eye animations with easing functions
- **Motion Detection**: Shake detection triggers angry emotions
- **Tap Detection**: Single and double tap recognition with audio feedback
- **Audio Synchronization**: MP3 playback for each emotion and event
- **Power Management**: Automatic sleep after 30s idle, deep sleep after 60s
- **Wake-on-Motion**: ESP32-C6 wakes from deep sleep on motion detection
- **Random Behavior**: Automatic emotion changes, blinking, and eye movement

## Hardware Requirements

### Components

1. **ESP32-C6 Supermini**
   - Microcontroller with I2C, UART, and deep sleep support

2. **GME 12865-50 (SSD1306 128x64 OLED Display)**
   - I2C interface (0x3C address)
   - VCC: 3.3V or 5V
   - GND: Ground
   - SCL: I2C clock
   - SDA: I2C data

3. **MPU6050 Accelerometer/Gyroscope**
   - I2C interface (0x68 address)
   - VCC: 3.3V or 5V
   - GND: Ground
   - SCL: I2C clock
   - SDA: I2C data
   - INT: Interrupt output

4. **MP3-TF-16P Audio Module**
   - Serial interface (9600 baud)
   - VCC: 5V
   - GND: Ground
   - RX: Receive data
   - TX: Transmit data
   - SPK+/SPK-: Speaker output
   - MicroSD card slot

5. **Speaker** (8Ω, 0.5-3W)

6. **MicroSD Card** (formatted FAT32)

### Wiring Diagram

```
ESP32-C6 Pin Connections:
┌─────────────────┬──────────────────┬─────────────────┐
│ ESP32-C6 Pin    │ Component        │ Notes           │
├─────────────────┼──────────────────┼─────────────────┤
│ GPIO0 (SCL)     │ OLED SCL         │ I2C Clock       │
│ GPIO0 (SCL)     │ MPU6050 SCL      │ I2C Clock       │
│ GPIO1 (SDA)     │ OLED SDA         │ I2C Data        │
│ GPIO1 (SDA)     │ MPU6050 SDA      │ I2C Data        │
│ GPIO2           │ MPU6050 INT      │ Motion Int      │
│ GPIO19 (TX)     │ MP3 Module RX    │ UART TX         │
│ GPIO20 (RX)     │ MP3 Module TX    │ UART RX         │
│ 3.3V            │ OLED VCC         │ Power           │
│ 3.3V            │ MPU6050 VCC      │ Power           │
│ GND             │ All GND pins     │ Common Ground   │
│ 5V              │ MP3 Module VCC   │ 5V Power        │
└─────────────────┴──────────────────┴─────────────────┘

I2C Bus (Shared):
  - Pull-up resistors: 4.7kΩ on SCL and SDA (may be built-in)
  - Clock speed: 100kHz for compatibility
  - Two devices: OLED (0x3C) and MPU6050 (0x68)

Speaker Connection:
  - Connect 8Ω speaker to MP3 module SPK+ and SPK- terminals
  - Optional: Add 100μF capacitor in series for better audio quality
```

## Software Requirements

### Arduino IDE Setup

1. **Install Arduino IDE** (version 2.0 or later)
   - Download from: https://www.arduino.cc/en/software

2. **Install ESP32 Board Support**
   - In Arduino IDE, go to File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://espressif.github.io/arduino-esp32/package_esp32_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "esp32" and install "esp32" by Espressif (version 2.0.x or later)
   - Select Board: "ESP32C6 Dev Module"

3. **Install Required Libraries**
   - Go to Tools → Manage Libraries
   - Install the following:
     - **U8g2** by olikraus (for OLED display)
     - **Adafruit MPU6050** by Adafruit (for motion sensor)
     - **Adafruit Unified Sensor** by Adafruit (dependency)
     - **DFRobotDFPlayerMini** by DFRobot (for MP3 module)

### Board Configuration

In Arduino IDE, configure these settings:
```
Board: "ESP32C6 Dev Module"
Upload Speed: "921600"
USB CDC On Boot: "Enabled"
USB Mode: "Hardware CDC and JTAG"
Flash Mode: "QIO 80MHz"
Flash Size: "4MB (32Mb)"
Partition Scheme: "Default 4MB with spiffs"
PSRAM: "Disabled"
```

## MicroSD Card Setup

### Folder Structure

Prepare a MicroSD card (FAT32 format) with this structure:

```
SD Card Root:
├── 01/                    # Emotion sounds
│   ├── 001.mp3           # Normal
│   ├── 002.mp3           # Happy
│   ├── 003.mp3           # Angry
│   ├── 004.mp3           # Sad
│   ├── 005.mp3           # Surprised
│   ├── 006.mp3           # Sleepy
│   ├── 007.mp3           # Scared
│   ├── 008.mp3           # Furious
│   ├── 009.mp3           # Excited
│   ├── 010.mp3           # Disappointed
│   ├── 011.mp3           # Confused
│   ├── 012.mp3           # Curious
│   ├── 013.mp3           # Bored
│   ├── 014.mp3           # Worried
│   ├── 015.mp3           # Annoyed
│   ├── 016.mp3           # Suspicious
│   ├── 017.mp3           # Skeptical
│   └── 018.mp3           # Frustrated
├── 02/                    # Tap sounds
│   ├── 001.mp3           # Single tap
│   └── 002.mp3           # Double tap
└── 03/                    # System sounds
    └── 001.mp3           # Sleep transition
```

### MP3 File Requirements

- **Format**: MP3 (CBR recommended, avoid VBR)
- **Bitrate**: 128kbps or lower
- **Sample Rate**: 44.1kHz or 22.05kHz
- **File Naming**: Exactly 3 digits (001, 002, etc.)
- **Folder Naming**: Exactly 2 digits (01, 02, etc.)
- **Duration**: Keep emotion sounds under 5 seconds for best experience

### Creating Audio Files

You can use any audio editor (Audacity, Adobe Audition, etc.) to create short sound effects:
- Emotion sounds: Match the emotion (happy laugh, sad sigh, angry grunt, etc.)
- Tap sounds: Short beeps or clicks
- Sleep sound: Gentle fade-out or yawn sound

## Installation

1. **Clone or Download** this repository

2. **Wire the hardware** according to the wiring diagram above

3. **Prepare the MicroSD card** with MP3 files as described

4. **Insert MicroSD card** into the MP3-TF-16P module

5. **Open** `mariunder_eyes.ino` in Arduino IDE

6. **Select** the correct board and port:
   - Tools → Board → ESP32C6 Dev Module
   - Tools → Port → (select your ESP32-C6 port)

7. **Upload** the sketch to your ESP32-C6

8. **Open Serial Monitor** (115200 baud) to see debug output

## Usage

### Basic Operation

Once powered on, the eyes will:
1. Initialize all components (watch Serial Monitor for status)
2. Display normal eyes with random blinking and looking around
3. Change emotions randomly every 15 seconds
4. Play corresponding MP3 files for each emotion

### Interactions

**Shake Detection:**
- Shake the device to trigger angry/furious emotion
- Plays angry sound effect
- Resets power idle timer

**Tap Detection:**
- Single tap: Plays tap sound 1, enters tap mode
- Double tap: Plays tap sound 2, enters tap mode
- During tap mode: No random emotion changes for 5 seconds
- Shake during tap playback: Stops audio immediately

**Power Management:**
- After 30 seconds of no motion: Switches to sleepy eyes
- After 60 seconds total: Enters deep sleep mode
- Wake from sleep: Shake the device (MPU6050 motion interrupt)

### Serial Monitor Commands

The Serial Monitor (115200 baud) displays:
- Component initialization status
- I2C device scan results
- Emotion changes
- Motion events (shake, tap)
- Power state transitions
- Wake-up source on restart

## Troubleshooting

### Display Issues

**Problem:** Display not working or showing garbage
- Check I2C connections (SDA=GPIO1, SCL=GPIO0)
- Verify 3.3V power supply
- Run I2C scan in Serial Monitor (should show 0x3C)
- Try lowering I2C clock speed in code: `Wire.setClock(50000);`

**Problem:** Display flickers
- Add 100nF ceramic capacitors near VCC/GND pins
- Shorten I2C wires
- Add 4.7kΩ pull-up resistors on SDA and SCL

### Motion Sensor Issues

**Problem:** MPU6050 not detected
- Check I2C connections (same bus as display)
- Verify power supply (3.3V or 5V)
- Run I2C scan (should show 0x68)
- Check INT pin connection to GPIO2

**Problem:** Too sensitive or not sensitive enough
- Adjust `_shakeThreshold` in `MotionManager.h` (default: 1.5g)
- Adjust `_motionThreshold` in `MotionManager.h` (default: 0.3g)
- Modify tap detection thresholds in `detectTap()` function

**Problem:** False tap detections
- Increase `TAP_THRESHOLD` in `MotionManager.h` (default: 12.0)
- Increase `TAP_DURATION` for longer tap window
- Check for vibrations from speaker affecting sensor

### Audio Issues

**Problem:** MP3 module not responding
- Check serial connections (TX→RX, RX→TX crossed correctly)
- Verify 5V power supply (not 3.3V!)
- Check baud rate (9600)
- Try different TX/RX pins if needed

**Problem:** No sound from speaker
- Check speaker connection to SPK+ and SPK-
- Verify MicroSD card is inserted and formatted as FAT32
- Check file naming: must be exactly 001.mp3, 002.mp3, etc.
- Check folder naming: must be exactly 01, 02, 03
- Test with known-good MP3 files
- Adjust volume: modify `_dfPlayer.volume(25)` in `AudioManager.h` (range: 0-30)

**Problem:** Audio playback skips or stutters
- Use lower bitrate MP3s (128kbps or less)
- Use CBR (Constant Bit Rate) instead of VBR
- Use smaller file sizes
- Check power supply (inadequate power can cause issues)

### Power Management Issues

**Problem:** Not entering sleep mode
- Ensure no continuous motion near sensor
- Check idle timers in `PowerManager.h`:
  - `_sleepyTimeout` (default: 30000ms)
  - `_deepSleepTimeout` (default: 60000ms)

**Problem:** Won't wake from deep sleep
- Verify MPU6050 INT pin connected to GPIO2
- Check `enableWakeInterrupt()` configuration
- Ensure motion detection is sensitive enough
- Try increasing motion by shaking harder

### Compilation Errors

**Problem:** "esp_sleep.h not found"
- Install latest ESP32 board support (2.0.0 or later)
- Select correct board: "ESP32C6 Dev Module"

**Problem:** "U8g2lib.h not found" or similar
- Install missing libraries via Library Manager
- Verify library versions are compatible

**Problem:** Upload fails
- Check USB cable (must support data, not just charging)
- Hold BOOT button during upload if needed
- Select correct port in Tools menu
- Try lower upload speed (460800 or 115200)

## Customization

### Adjusting Timings

Edit these values in the respective files:

**Random Emotion Changes** (`EmotionScheduler.h`):
```cpp
_randomChangeInterval(15000)  // Change from 15000ms (15s) to desired value
```

**Blink Timing** (`mariunder_eyes.ino` in setup()):
```cpp
face->Blink.MinBlinkInterval = 2000;  // Minimum time between blinks (ms)
face->Blink.MaxBlinkInterval = 5000;  // Maximum time between blinks (ms)
```

**Look Timing** (`mariunder_eyes.ino` in setup()):
```cpp
face->Look.MinLookInterval = 1000;  // Minimum time before look change (ms)
face->Look.MaxLookInterval = 4000;  // Maximum time before look change (ms)
```

**Power Timeouts** (`PowerManager.h`):
```cpp
_sleepyTimeout(30000)      // Time until sleepy mode (ms)
_deepSleepTimeout(60000)   // Time until deep sleep (ms)
```

**Tap Mode Duration** (`EmotionScheduler.h`):
```cpp
if (_inTapMode && (millis() - _tapModeStartTime) > 5000)  // Change 5000ms to desired
```

### Modifying Emotions

Edit eye configurations in `EyePresets.h`:
- Adjust `upperLidY`, `lowerLidY` for eye openness
- Adjust `upperLidBend`, `lowerLidBend` for expression curves
- Adjust `irisRadius`, `pupilRadius` for eye appearance

### Adding New Emotions

1. Add new emotion to `FaceEmotions.hpp` enum
2. Create preset in `EyePresets.h` `getPreset()` function
3. Add MP3 file to SD card folder 01/
4. Update emotion count and documentation

## Project Structure

```
mariunder_eyes/
├── mariunder_eyes.ino          # Main Arduino sketch
├── Common.h                    # Common definitions and pin config
├── FaceEmotions.hpp            # Emotion enum definitions
├── AsyncTimer.h/cpp            # Non-blocking timer utility
├── Animations.h                # Animation primitives (Ramp, Triangle, etc.)
├── EyeConfig.h                 # Eye parameter structure
├── EyePresets.h                # Predefined emotion configurations
├── Eye.h                       # Individual eye class
├── EyeDrawer.h                 # OLED rendering engine
├── BlinkAssistant.h            # Automatic blinking
├── LookAssistant.h             # Eye movement/gaze control
├── FaceExpression.h            # Smooth emotion transitions
├── FaceBehavior.h              # Random behavior/emotion selection
├── Face.h                      # Main face controller
├── MotionManager.h             # MPU6050 integration
├── AudioManager.h              # MP3-TF-16P integration
├── PowerManager.h              # Power state management
├── EmotionScheduler.h          # Emotion scheduling and audio sync
└── README.md                   # This file
```

## Credits

This project is based on and adapted from:
- **esp32-eyes** GitHub project - Parametric eye animation system
- **U8g2** library by olikraus - Monochrome OLED graphics
- **Adafruit MPU6050** library - Motion sensor support
- **DFRobotDFPlayerMini** library - MP3 playback

## License

This project is provided as-is for educational and personal use.

## Contributing

Feel free to submit issues, suggestions, or improvements. This is an open-ended project with many possibilities for enhancement.

## Version History

- **v1.0** (2026-02-18): Initial implementation
  - 18 emotions with smooth transitions
  - Motion detection (shake, tap)
  - Audio synchronization
  - Power management with deep sleep
  - Wake-on-motion

## Future Enhancement Ideas

- BLE control for remote emotion triggering
- Microphone input for sound-reactive emotions
- Multiple emotion playlists (happy playlist, scary playlist, etc.)
- Battery voltage monitoring and low-power optimization
- NeoPixel LED integration for colored lighting effects
- Web interface for configuration
- Servo-controlled physical eyelids
- Multiple display support (both eyes on separate OLEDs)

---

**Enjoy your Mariunder Eyes project!** 👀
# mariunder-two
