# Respository of my IoTs

## pc

### Hardware
1. Any PC hardware

### Setup
1. Setup Docker with WSL2 on Windows
2. Deploy Pi hole on docker
3. Deploy Home Assisstant on docker
4. Deploy Mosiquitto MQTT broker on docker

## iot
A general purpose iot for climax and health monitoring and control in each room 

### Hardware
1. ESP8266 WIFI
2. DHT11 Temperature and Humidity sensor (optional)
3. MH-Z14A CO2 concentration sensor (optional)
4. IR emit LED (optional)
5. LED (optional)

## pi-home
Deprecated. Replaced by Docker on Windows WSL2
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
