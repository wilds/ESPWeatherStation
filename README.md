# ESPWeatherStation

Daylight Saving Time and other customizations of the original ESP8266 OLED Weather Station.
Uses SSD1306 128x64 OLED display with with either SPI or I2C interface

Based on:
* https://github.com/neptune2/esp8266-weather-station-oled-DST/
* https://github.com/ThingPulse/esp8266-weather-station

## Features
* clock
* weather
* forecast
* sun and moon phase
* read data from:
  * DHT sensor
  * BME280 sensor
  * NTC resistor
* configuration through webserver
* send temperature data to:
  * ThingSpeak channel
  * MQTT brocker
* AWS labda function for Alexa integration

## Hardware Requirements

This code is made for an 128x64 SSD1603 OLED display with code running on an ESP8266.
Either the SPI or I2C version can be used.

## Software Requirements/Libraries

* [PlatformIO] (https://platformio.org/)

You also need to get an API key for the OpenWeatherMap data: https://docs.thingpulse.com/how-tos/openweathermap-key/

## Wemos D1R2 Wiring

See code for pin configurations

| SSD1306 SPI | Wemos D1R2 |
| ----------- |:----------:|
| CS          | D8         |
| DC          | D2         |
| RST         | D0         |
| D1          | D7         |
| D0          | D5         |
| GND         | GND        |
| VCC         | 3V3        |

| DHT22 | Wemos D1R2 |
| ----- |:----------:|
| DATA  | D4         |
| GND   | GND        |
| VCC   | 3V3        |
