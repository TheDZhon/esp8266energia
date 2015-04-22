# ESP8266Energia

## Remote humidity/temperature sensor on ESP8266 link

Uses MSP430G2 Launchpad as main MCU, DHT22 as sensor and piezo buzzer for init and error tones.
Power consumtion issues adressed with MSP430 LPM3 and ESP8266 LPM.
Sends humidity and temperature to remote TCP server over WiFi as ASCII strings.

## Hardware requirements

* MSP430G2 Launchpad.
* ESP8266 board with connected XPD_DCDC <-> EXT_RSTB pins. See hack for ESP-01: http://goo.gl/J7OYMq.
* DHT22 sensor (DHT11 should be fine and cheaper replacement).
* [Optional] Piezo buzzer.

## Software requirements

* Energia 0013+ (LPM API introduced in this version: sleep, sleepSeconds, etc.).
* ESP8266 AT 0.22+ firmware.
* ESP8266 should be preconfigured with AT+*_DEF commands for automatic WiFi link operation.
