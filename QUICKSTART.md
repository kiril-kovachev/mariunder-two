# Quick Start Guide

Get your Mariunder Eyes up and running in 5 steps!

## Step 1: Gather Hardware (5 minutes)

You need:
- [ ] ESP32-C6 Supermini board
- [ ] SSD1306 OLED display (128x64)
- [ ] MPU6050 motion sensor
- [ ] TTP223 capacitive touch sensor
- [ ] MP3-TF-16P audio module
- [ ] 8Ω speaker
- [ ] MicroSD card (any size, FAT32)
- [ ] Jumper wires
- [ ] USB cable for programming

## Step 2: Wire Everything (10 minutes)

**I2C Bus (shared):**
```
ESP32-C6 GPIO0 (SCL) → OLED SCL + MPU6050 SCL
ESP32-C6 GPIO1 (SDA) → OLED SDA + MPU6050 SDA
ESP32-C6 3.3V        → OLED VCC + MPU6050 VCC
ESP32-C6 GND         → OLED GND + MPU6050 GND
```

**Motion Interrupt:**
```
ESP32-C6 GPIO2 → MPU6050 INT pin
```

**Touch Sensor:**
```
ESP32-C6 GPIO3  → TTP223 OUT
ESP32-C6 3.3V   → TTP223 VCC
ESP32-C6 GND    → TTP223 GND
```

**Audio Module:**
```
ESP32-C6 GPIO7 (TX) → MP3 Module RX
ESP32-C6 GPIO6 (RX) → MP3 Module TX
ESP32-C6 5V          → MP3 Module VCC
ESP32-C6 GND         → MP3 Module GND
MP3 Module SPK+/SPK- → Speaker
```

## Step 3: Prepare SD Card (5 minutes)

1. Format microSD card as FAT32
2. Create folders: `01`, `02`, `03`
3. Add MP3 files:
   - `01/001.mp3` – `01/018.mp3` — emotion sounds (Normal → Frustrated)
   - `02/001.mp3` – `02/010.mp3` — your own playlist (browsed in MP3 mode)
   - `03/001.mp3` — sleep transition sound
   - `03/002.mp3` — enter MP3 mode sound
   - `03/003.mp3` — enter volume mode sound
   - `03/004.mp3` — volume test tone
   - `03/005.mp3` — shake reaction sound
   - `03/006.mp3` — exit mode sound
4. Insert SD card into MP3 module

**Quick tip**: Folder 02 can have 1–10 tracks; start with one to test!

## Step 4: Install Software (10 minutes)

### Arduino IDE:
1. Install Arduino IDE 2.0+
2. Add ESP32 board support:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Tools → Board → Boards Manager → Install "esp32"

3. Install libraries (Tools → Manage Libraries):
   - `U8g2`
   - `Adafruit MPU6050`
   - `Adafruit Unified Sensor`
   - `DFRobotDFPlayerMini`

4. Select board:
   - Tools → Board → ESP32C6 Dev Module
   - Tools → Port → (your ESP32-C6 port)

## Step 5: Upload and Test (5 minutes)

1. Open `mariunder_eyes/mariunder_eyes.ino`
2. Click Upload (→) button
3. Open Serial Monitor (115200 baud)
4. Watch initialization messages

**Expected output:**
```
=================================
  Mariunder Eyes - Starting Up
=================================

Initializing I2C bus...
Scanning I2C bus:
  Device found at 0x3C  ← OLED display
  Device found at 0x68  ← MPU6050 sensor
Total devices found: 2

Initializing OLED display...
Display OK

Initializing MPU6050...
MPU6050 OK

Initializing MP3 player...
MP3 player OK

[... more initialization messages ...]

Starting main loop...
```

## Troubleshooting

### ❌ "MPU6050 initialization failed"
- Check I2C wiring (SDA, SCL, VCC, GND)
- Verify 3.3V power
- Check Serial Monitor for I2C scan results

### ❌ "MP3 player initialization failed"
- Check serial wiring (TX↔RX crossed)
- Verify 5V power (not 3.3V!)
- Check SD card is inserted and FAT32 formatted

### ❌ No display output
- Check I2C connections
- Run I2C scan (should show 0x3C)
- Try powering display with 5V instead of 3.3V

### ❌ Upload fails
- Hold BOOT button during upload
- Check USB cable supports data
- Try lower upload speed (Tools → Upload Speed → 115200)

## First Test

Once running, try these:
1. **Watch**: Eyes should blink and look around automatically
2. **Wait 15s**: Emotion should change randomly with a sound
3. **Shake**: Should trigger angry/furious emotion — eyes buzz and dedicated shake sound plays
4. **Single tap**: Enters MP3 mode (pulsing circle) — rotate 15° to select and play a track from folder 02; eyes show with wave animation during playback
5. **Single tap during playback**: Cancels track, returns to track selection
6. **Single tap (idle in MP3 mode)**: Exits MP3 mode
7. **Long press 2s**: Enters volume mode — rotate 15° right/left to adjust volume; each step plays a test tone
8. **Wait 30s idle**: Should show sleepy eyes with sleep sound
9. **Wait 60s idle**: Should enter deep sleep
10. **Shake to wake**: Should wake up from deep sleep!

## Next Steps

✅ Working? Great! Now:
1. Add more emotion sound files
2. Adjust thresholds in code if needed
3. Customize timings for your preference
4. See README.md for full documentation

## Support

If stuck:
1. Check wiring carefully (most common issue)
2. Verify all libraries installed correctly
3. Read full README.md for detailed troubleshooting
4. Check Serial Monitor output for clues

---

**Estimated total time**: 35 minutes from zero to working!

Enjoy your Mariunder Eyes! 👀
