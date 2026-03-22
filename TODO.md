# Define tap and 2 seconds hold modes
## Single tap - mp3 select

## Hold 2 secs - enter volume mode

# DFRobotDFPlayerMini to ADC link
## Sinewave to show real played data
## Diagram

           3.3V
            |
           100k
            |
DAC_L ----||----- ESP32 ADC
         1uF
            |
           100k
            |
           GND

# Move sinewave down not to go over the eyes

# Bugfix - exit tap mode goes to angry

# ADC - battery divider

Battery + ---- R1 (100k) ----+---- ESP32 ADC
                             |
                            R2 (100k)
                             |
                            GND

# GPIO 5v detect
USB 5V (VBUS)
      |
     R1(100k)
      |
      +---- GPIO (input)
      |
     R2(100k)
      |
     GND
     
# Play sound on 5v usb
# battery indicator on low battery

#Bug: fixed? Log -> Manual sleep requested via long tap - entering POWER_DEEP_SLEEP
is shown, but device does not enter sleep

#Bug: fixed? After playing mp3 (single tap mode), exit to eyes, instead to mp3 mode
#Move sinewave 4 pixels down

# Bug: fixed? - sinewave on whole screen
# Bug: fixed? - After shake mode -> not returning to normal eyes
# Shake mode buz effect stays active after shake completion
# Bug: fixed? - power saving state:
22:23:37.056 -> Manual sleep requested via long tap - entering POWER_DEEP_SLEEP
22:23:37.056 -> PowerManager in POWER_SLEEPY/POWER_DEEP_SLEEP state - forcing sleepy emotion
22:23:37.056 -> Changing emotion to: Sleepy
22:23:37.056 -> Playing sleep transition sound
22:23:37.165 -> Activity detected - returning to POWER_ACTIVE state