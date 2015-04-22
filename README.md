# ESP8266Energia

## Remote humidity/temperature sensor on ESP8266 link

Uses MSP430G2 Launchpad as main MCU, DHT22 as sensor and simple stupid buzzer
for funny init/error tones.

## Software requirements

* Energia 0013+ (LPM API introduced in this version: sleep, sleepSeconds, etc.).

## Hardware requirements

* MSP430G2 Launchpad.
* ESP8266 board with connected XPD_DCDC <-> EXT_RSTB pins. See hack for ESP-01: http://goo.gl/J7OYMq.
* DHT22 sensor (DHT11 should be fine and cheaper replacement).
* [Optional] Piezo buzzer.
