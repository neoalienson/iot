# Respository of my IoTs

## pi-home
Raspberry Pi in my living room with below feature
* Hub of home automation with Home Assistant
* Pi Hole 
* CO<sub>2</sub> level sensor

### Hardware
1. Raspberry Pi 3 B+
2. MHZ14A CO<sub>2</sub> sensor

### Setup
To setup on Raspberry PI
* Install Raspberry Pi OS
* Copy config.ini.sample to config.ini and update the credentails
* Run `raspi-config` to enable serial port and disable console for serial port
* Install https://www.home-assistant.io/ manually
* Install pi-hole
