/*
 * Mariunder Eyes - ESP32-C6 Animated Eyes Project
 *
 * Hardware:
 * - ESP32-C6 Supermini
 * - GME 12865-50 (SSD1306 128x64 OLED) - I2C (SDA=GPIO1, SCL=GPIO0)
 * - MPU6050 Motion Sensor - I2C (SDA=GPIO1, SCL=GPIO0)
 * - TTP223 Touch Sensor - Digital (GPIO7)
 * - MP3-TF-16P Audio Module - Serial (RX=GPIO20, TX=GPIO19)
 *
 * Features:
 * - 18 different eye emotions with smooth transitions
 * - Random emotion changes every 15 seconds
 * - Inertia eye effect based on device rotation (gyroscope)
 * - Shake detection triggers angry emotions
 * - Single/double tap detection via TTP223 touch sensor
 * - Synchronized MP3 playback for each emotion
 * - Power management: 30s idle -> sleepy, 60s -> deep sleep
 * - Wake-on-motion from deep sleep
 */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// Project headers
#include "Common.h"
#include "Face.h"
#include "MotionManager.h"
#include "TouchSensorManager.h"
#include "AudioManager.h"
#include "PowerManager.h"
#include "EmotionScheduler.h"

// ==================== Hardware Configuration ====================

// U8G2 display instance for ESP32-C6
// Using hardware I2C with custom pins
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_instance(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* scl=*/ I2C_SCL_PIN, /* sda=*/ I2C_SDA_PIN);

// Hardware serial for MP3 module (UART1)
HardwareSerial mp3Serial(1);

// ==================== Component Instances ====================

Face* face = nullptr;
MotionManager motionManager;
TouchSensorManager touchSensor;
AudioManager audioManager;
PowerManager powerManager;
EmotionScheduler scheduler;

// ==================== Setup ====================

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);  // Wait for serial to initialize

    Serial.println("\n\n=================================");
    Serial.println("  Mariunder Two - Starting Up");
    Serial.println("=================================\n");

    // Check if waking from deep sleep
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        Serial.println("Woke up from deep sleep!");
        Serial.print("Wake caused by GPIO: ");
        uint64_t wakeup_pin = esp_sleep_get_ext1_wakeup_status();
        if (wakeup_pin & (1ULL << MPU_INT_PIN)) {
            Serial.println("MPU6050 (motion/shake)");
        }
        if (wakeup_pin & (1ULL << TOUCH_SENSOR_PIN)) {
            Serial.println("TTP223 (touch sensor)");
        }
    }

    // ===== I2C Initialization =====
    Serial.println("Initializing I2C bus...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);  // 100kHz for compatibility

    // Scan I2C bus
    Serial.println("Scanning I2C bus:");
    int devicesFound = 0;
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("  Device found at 0x");
            if (addr < 16) Serial.print("0");
            Serial.println(addr, HEX);
            devicesFound++;
        }
    }
    Serial.print("Total devices found: ");
    Serial.println(devicesFound);
    Serial.println();

    // ===== Display Initialization =====
    Serial.println("Initializing OLED display...");
    u8g2_instance.begin();
    u8g2_instance.setPowerSave(0);  // Ensure display is on after wake from sleep
    u8g2_instance.setContrast(128);
    u8g2_instance.clearBuffer();
    u8g2_instance.setFont(u8g2_font_ncenB08_tr);
    u8g2_instance.drawStr(0, 20, "Mariunder Two");
    u8g2_instance.sendBuffer();
    Serial.println("Display OK");

    // ===== Motion Sensor Initialization =====
    Serial.println("Initializing MPU6050 (gyroscope/accelerometer)...");
    if (!motionManager.begin()) {
        Serial.println("ERROR: MPU6050 initialization failed!");
        u8g2_instance.clearBuffer();
        u8g2_instance.drawStr(0, 20, "ERROR:");
        u8g2_instance.drawStr(0, 35, "MPU6050 Failed");
        u8g2_instance.sendBuffer();
        while (1) delay(1000);  // Halt
    }
    Serial.println("MPU6050 OK");

    // ===== Touch Sensor Initialization =====
    Serial.println("Initializing TTP223 touch sensor...");
    if (!touchSensor.begin()) {
        Serial.println("WARNING: Touch sensor initialization failed!");
        // Don't halt - continue without touch
    } else {
        Serial.println("TTP223 touch sensor OK");
    }

    // ===== Audio Module Initialization =====
    Serial.println("Initializing MP3 player...");
    if (!audioManager.begin(&mp3Serial)) {
        Serial.println("WARNING: MP3 player initialization failed!");
        Serial.println("Audio features will not work.");
        // Don't halt - continue without audio
    } else {
        Serial.println("MP3 player OK");
    }

    // ===== Face Initialization =====
    Serial.println("Initializing face system...");
    face = new Face(&u8g2_instance);
    face->Expression.SetTransitionTime(500);  // 500ms transitions
    face->RandomBehavior = true;
    face->RandomBlink = true;
    face->RandomLook = true;

    // Configure inertia effect
    face->Inertia.SetMotionManager(&motionManager);
    face->Inertia.Enabled = true;           // Enable inertia effect
    face->Inertia.Sensitivity = 1.0f;       // Default sensitivity (was 0.2f - this was the issue!)
    face->Inertia.Damping = 0.65f;          // Default damping (higher = more sluggish)
    face->Inertia.MaxDisplacement = 8.0f;   // Maximum eye movement
    face->Inertia.ReturnSpeed = 0.05f;      // Speed of return to center

    Serial.println("Face system OK");

    // ===== Power Manager Initialization =====
    Serial.println("Initializing power manager...");
    powerManager.begin(&motionManager);
    Serial.println("Power manager OK");

    // ===== Emotion Scheduler Initialization =====
    Serial.println("Initializing emotion scheduler...");
    scheduler.begin(face, &audioManager, &powerManager);
    Serial.println("Emotion scheduler OK");

    // ===== Set Initial Emotion =====
    Serial.println("Setting initial emotion to NORMAL");
    face->Expression.GoTo_Normal();

    Serial.println("\n=================================");
    Serial.println("  Initialization Complete!");
    Serial.println("=================================\n");
    Serial.println("Controls:");
    Serial.println("  - Rotate device: Inertia eye movement");
    Serial.println("  - Shake: Trigger angry emotion");
    Serial.println("  - Single tap (touch sensor): Enter rotate mode 1");
    Serial.println("    - In rotate mode 1: rotate 15 degrees to play/change MP3 file from folder 02");
    Serial.println("    - Single tap again to exit rotate mode");
    Serial.println("    - Auto-exit after 5s of no rotation");
    Serial.println("  - Double tap (touch sensor): Enter rotate mode 2");
    Serial.println("    - In rotate mode 2: rotate 15 degrees right=volume up, left=volume down");
    Serial.println("    - Single tap to exit rotate mode");
    Serial.println("    - Auto-exit after 45s of no rotation");
    Serial.println("  - Shake during rotate mode: Exit rotate mode");
    Serial.println("  - 30s idle: Enter sleepy mode");
    Serial.println("  - 60s idle: Deep sleep (wake on motion)");
    Serial.println("\nInertia Configuration:");
    Serial.print("  - Enabled: "); Serial.println(face->Inertia.Enabled ? "Yes" : "No");
    Serial.print("  - Sensitivity: "); Serial.println(face->Inertia.Sensitivity);
    Serial.print("  - Damping: "); Serial.println(face->Inertia.Damping);
    Serial.print("  - MaxDisplacement: "); Serial.println(face->Inertia.MaxDisplacement);
    Serial.print("  - ReturnSpeed: "); Serial.println(face->Inertia.ReturnSpeed);
    Serial.println("\nTouch Sensor: GPIO7 (TTP223)");
    Serial.println("Motion Sensor: MPU6050 (rotation & shake only)");
    Serial.println("\nStarting main loop...\n");

    delay(1000);
}

// ==================== Main Loop ====================

void loop() {
    // Update all managers
    motionManager.update();
    touchSensor.update();
    audioManager.update();
    powerManager.update();

    // Check if entering deep sleep
    if (powerManager.getState() == POWER_DEEP_SLEEP) {
        // Check if sleepy eyes animation is visible (give it time to render)
        static uint32_t deepSleepStartTime = 0;
        static bool displayTurnedOff = false;

        if (deepSleepStartTime == 0) {
            deepSleepStartTime = millis();
            displayTurnedOff = false;
        }

        uint32_t timeSinceSleepyStart = millis() - deepSleepStartTime;

        // Turn off display after 2 seconds (after sleepy eyes visible)
        if (timeSinceSleepyStart > 2000 && !displayTurnedOff) {
            Serial.println("Turning off display...");
            u8g2_instance.setPowerSave(1);  // Turn off display
            displayTurnedOff = true;
        }

        // Wait total of 7 seconds (2s eyes + 5s display off), then execute deep sleep
        if (timeSinceSleepyStart > 7000) {
            powerManager.executeDeepSleep();
            // Will not return from executeDeepSleep()
        }

        // Continue rendering sleepy eyes while waiting (before display off)
        if (!displayTurnedOff) {
            // Rendering continues normally below
        } else {
            // Display is off, skip rendering
            return;
        }
    } else {
        // Reset deep sleep start time if not in deep sleep mode
        static uint32_t deepSleepStartTime = 0;
        static bool displayTurnedOff = false;
        deepSleepStartTime = 0;
        displayTurnedOff = false;
    }

    // ===== Handle Shake Events =====
    if (motionManager.isShaking()) {
        if (scheduler.getRotateMode() != EmotionScheduler::ROTATE_MODE_NONE) {
            // Shake in rotate mode = stop audio & exit rotate mode
            Serial.println("Shake during rotate mode - exiting");
            audioManager.stopPlayback();
            // Single tap to exit will be handled by next tap event
            // For now, we'll just stop audio
        } else {
            // Normal shake = trigger angry emotion
            scheduler.handleShake();
        }

        // Reset power manager activity timer
        powerManager.resetActivity();
    }

    // ===== Handle Touch/Tap Events =====
    TouchSensorManager::TapType tap = touchSensor.getTapEvent();
    if (tap != TouchSensorManager::NONE) {
        scheduler.handleTap(tap);

        // Reset power manager activity timer
        powerManager.resetActivity();
    }

    // ===== Handle Rotation in Rotate Modes =====
    if (scheduler.getRotateMode() != EmotionScheduler::ROTATE_MODE_NONE) {
        scheduler.handleRotation(&motionManager);
    }

    // ===== Handle Motion After Idle =====
    // Check for motion-triggered emotion change (after 5s idle)
    scheduler.handleMotion(&motionManager);

    // ===== Update Emotion Scheduler =====
    scheduler.update();

    // ===== Update and Render Face =====
    face->Update();

    // Small delay to prevent CPU overload
    delay(10);
}
