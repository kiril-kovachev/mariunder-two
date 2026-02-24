#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

// ESP32-C6 specific configurations
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// I2C Configuration for ESP32-C6
#define I2C_SDA_PIN 1
#define I2C_SCL_PIN 0

// Serial configuration for MP3 module
#define MP3_RX_PIN 6
#define MP3_TX_PIN 7

// TTP223 touch sensor pin
#define TOUCH_SENSOR_PIN 3

// MPU6050 interrupt pin
#define MPU_INT_PIN 2 // Define the interrupt pin for MPU6050

// Math helpers
#ifndef PI
#define PI 3.14159265359
#endif

#ifndef TWO_PI
#define TWO_PI 6.28318530718
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.2957795131
#endif

// Constrain and map helpers (Arduino compatibility)
template<typename T>
T constrainValue(T value, T min, T max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

template<typename T>
T mapValue(T value, T fromLow, T fromHigh, T toLow, T toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

#endif // COMMON_H
