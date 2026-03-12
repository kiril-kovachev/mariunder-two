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
