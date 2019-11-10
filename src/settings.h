/**The MIT License (MIT)

Copyright (c) 2016 by Daniel Eichhorn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://blog.squix.ch
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define VERSION "1.0"

#include <simpleDSTadjust.h>
#include <DHT.h>

// >>> Uncomment one of the following 2 lines to define which OLED display interface type you are using
//#define spiOLED
#define i2cOLED

#ifdef spiOLED
#include "SSD1306Spi.h"
#endif
#ifdef i2cOLED
#include "SSD1306Wire.h"
#endif
#include "OLEDDisplayUi.h"

// Please read http://blog.squix.org/weatherstation-getting-code-adapting-it
// for setup instructions

#define HOSTNAME "ESP8266-OTA-"

// Setup
const int UPDATE_INTERVAL_SECS = 30 * 60; // Update every 10 minutes

#ifdef spiOLED
// Pin definitions for SPI OLED
#define OLED_CS     D8  // Chip select
#define OLED_DC     D2  // Data/Command
#define OLED_RESET  D0  // RESET  - If you get an error on this line, either change Tools->Board to the board you are using or change "D0" to the appropriate pin number for your board.
#endif

#ifdef i2cOLED
// Pin definitions for I2C OLED
const int I2C_DISPLAY_ADDRESS = 0x3c;
// const int SDA_PIN = 0;
// const int SDC_PIN = 2;
const int SDA_PIN = D2;
const int SDC_PIN = D1;
#endif

// DHT Settings
// Uncomment whatever type you're using!

#define DHTPIN D2 // NodeMCU
//#define DHTPIN D4 // Wemos D1R2 Mini

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)



// -----------------------------------
// Example Locales (uncomment only 1)
//#define Zurich
//#define Boston
//#define Sydney
#define Rome
//------------------------------------


#ifdef Rome
//DST rules for Central European Time Zone
#define UTC_OFFSET +1
struct dstRule StartRule = {"CEST", Last, Sun, Mar, 2, 3600 * 1}; // Central European Summer Time = UTC/GMT +2 hours
struct dstRule EndRule = {"CET", Last, Sun, Oct, 2, 3600 * 0};       // Central European Time = UTC/GMT +1 hour

// Uncomment for 24 Hour style clock
#define STYLE_24HR

#define NTP_SERVERS "0.ch.pool.ntp.org", "1.ch.pool.ntp.org", "2.ch.pool.ntp.org"

// OpenWeatherMap Settings
// Sign up here to get an API key:
// https://docs.thingpulse.com/how-tos/openweathermap-key/
String OPEN_WEATHER_MAP_APP_ID = "1179a7fb31447b316339ddc0d070f4b8";
/*
Go to https://openweathermap.org/find?q= and search for a location. Go through the
result set and select the entry closest to the actual location you want to display 
data for. It'll be a URL like https://openweathermap.org/city/2657896. The number
at the end is what you assign to the constant below.
 */
const char* OPEN_WEATHER_MAP_LOCATION_ID = "2524907";

// Pick a language code from this list:
// Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
// English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
// Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
// Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
// Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
// Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
// Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
const char* OPEN_WEATHER_MAP_LANGUAGE = "it";


const boolean IS_METRIC = true;
// Adjust according to your language
//const char* WDAY_NAMES[] = {"DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB"};
//const char* MONTH_NAMES[] = {"GEN", "FEB", "MAR", "APR", "MAG", "GIU", "LUG", "AGO", "SET", "OTT", "NOV", "DIC"};

#endif


//Thingspeak Settings
const unsigned long THINGSPEAK_CHANNEL_ID = 900047;
const char* THINGSPEAK_API_READ_KEY = "";
const char* THINGSPEAK_API_WRITE_KEY = "5S8TFUEEKERX9IUA";

#ifdef spiOLED
SSD1306Spi display(OLED_RESET, OLED_DC, OLED_CS);  // SPI OLED
#endif
#ifdef i2cOLED
SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);  // I2C OLED
#endif



// Setup simpleDSTadjust Library rules
simpleDSTadjust dstAdjusted(StartRule, EndRule);

/*
const char* MQTT_BROKER = "farmer.cloudmqtt.com";
const uint16_t MQTT_PORT = 13630;
const char* MQTT_USER = "uhxrllbh";
const char* MQTT_PASSWORD = "PUut7doEUIdn";
*/

#define CONFIG "/user.json"


/***************************
 * End Settings
 **************************/

#endif