/**The MIT License (MIT)

Copyright (c) 2016 by Daniel Eichhorn
Copyright (c) 2019 by Wilds

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

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <WiFiManager.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <time.h>

#include "settings.h"

#include "displays/WeatherDisplay.h"
#include "displays/ForecastDisplay.h"
#include "displays/DHTDisplay.h"
#include "displays/BME280Display.h"
#include "displays/NTCDisplay.h"
#include "displays/SunMoonDisplay.h"

//ThingSpeak
#include <ThingSpeak.h>

//MQTT
#include <PubSubClient.h>

#include "fonts/WeatherStationImages.h"
#include "fonts/DSEG7Classic-BoldFont.h"

#include "Config.h"


#define NO_INDICATOR
#define PRINT_DEBUG_MESSAGES
OLEDDisplayUi   ui( &display );

// Initialize OpenWeatherMap client with METRIC setting
WeatherDisplay weatherDisplay(OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, IS_METRIC, OPEN_WEATHER_MAP_LANGUAGE);
ForecastDisplay forecastDisplay(OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, IS_METRIC, OPEN_WEATHER_MAP_LANGUAGE);
DHTDisplay dhtDisplay(DHTPIN, DHTTYPE, IS_METRIC);
BME280Display bmeDisplay(IS_METRIC);
void dhtUpdateCallback(DHTDisplay* myDHTDisplay);
void bmeUpdateCallback(DHTDisplay* myDHTDisplay);
NTCDisplay ntcDisplay(DHTPIN, IS_METRIC);
SunMoonDisplay sunmoonDisplay(0, 0, &dstAdjusted);


WiFiClient client;

WiFiClient mqttClient;
PubSubClient mqtt(mqttClient);
Ticker mqttReconnectTimer;

void initMqtt();
void updateMqtt();
void callback(char* topic, byte* payload, unsigned int length);
void connectToMqtt();


//declaring prototypes
void configModeCallback(WiFiManager *myWiFiManager);
void drawProgress(OLEDDisplay *display, int percentage, String label);
void drawOtaProgress(unsigned int, unsigned int);


void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);

void initData(OLEDDisplay *display);

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);

Config config(String(CONFIG));

int8_t getWifiQuality();

struct FrameParameter {
  FrameParameter() : id(0) {}
  FrameParameter(uint64_t _id) : id(_id) {}
  uint64_t     id                = 0;
  void *extra = nullptr;
};


// Add frames
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
FrameCallback frames[] = {
    drawDateTime,
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      weatherDisplay.draw(display, state, x, y);
    },
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      sunmoonDisplay.draw2(display, state, x, y);
    },
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      sunmoonDisplay.draw(display, state, x, y);
    },
    //std::bind(&WeatherDisplay::draw, &weatherDisplay, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      dhtDisplay.draw(display, state, x, y);
    },
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      bmeDisplay.draw(display, state, x, y);
    },
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      bmeDisplay.draw2(display, state, x, y);
    },
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      ntcDisplay.draw(display, state, x, y);
    },
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      forecastDisplay.draw(display, state, x, y);
    },
    [](OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
      forecastDisplay.draw2(display, state, x, y);
    }
};
int numberOfFrames = 9;

FrameParameter parameters[] = {{}, {}, {}, FrameParameter(1), FrameParameter(2), {}, {}, {}, {}};

OverlayCallback overlays[] = { drawHeaderOverlay };
int numberOfOverlays = 1;

//WiFiManager
WiFiManager wifiManager;
WiFiManagerParameter mqttBrokerURL("mqttBrokerURL", "Broker URL", config.mqttBrokerURL.c_str(), 255);
WiFiManagerParameter mqttBrokerPort("mqttBrokerPort", "Broker Port", String(config.mqttBrokerPort).c_str(), 5, "input='number'");
WiFiManagerParameter mqttUser("mqttUser", "User", config.mqttUser.c_str(), 255);
WiFiManagerParameter mqttPassword("mqttPassword", "Password", config.mqttPassword.c_str(), 255, "input='password'");
WiFiManagerParameter mqttApiKey("mqttApiKey", "Token", config.apiKey.c_str(), 255);
WiFiManagerParameter deviceName("deviceName", "Device Name", config.deviceName.c_str(), 255);

//TODO https://github.com/ThingPulse/esp8266-weather-station/blob/master/examples/SunMoonCalcDemo/SunMoonCalcDemo.ino
//https://www.wunderground.com/hourly/it/cosenza/39.30,16.25

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");

  config.mqttBrokerURL = wifiManager.server->arg("mqttBrokerURL");
  config.mqttBrokerPort = wifiManager.server->arg("mqttBrokerPort").toInt();
  config.mqttUser = wifiManager.server->arg("mqttUser");
  config.mqttPassword = wifiManager.server->arg("mqttPassword");
  config.apiKey = wifiManager.server->arg("mqttApiKey");
  config.deviceName = wifiManager.server->arg("deviceName");

  config.write();
  ESP.restart();

}

void setup() {
  // Turn On VCC
  // pinMode(D4, OUTPUT);
  // digitalWrite(D4, HIGH);
  Serial.begin(115200);

  //Init config
  config.read();

  // initialize display
  display.init();
  display.clear();
  display.display();

  //display.flipScreenVertically();  // Comment out to flip display 180deg
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  // Credit where credit is due
  display.drawXbm(-6, 5, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.drawString(88, 8, "Weather Station\nBy Squix78\nmods by Neptune\nWilds");
  display.display();

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.addParameter(&mqttBrokerURL);
  wifiManager.addParameter(&mqttBrokerPort);
  wifiManager.addParameter(&mqttUser);
  wifiManager.addParameter(&mqttPassword);
  wifiManager.addParameter(&mqttApiKey);
  wifiManager.addParameter(&deviceName);
  wifiManager.setSaveParamsCallback(saveParamCallback);

  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
  wifiManager.setMenu(menu);

  wifiManager.setClass("invert");

  wifiManager.setConfigPortalBlocking(false);
  //wifiManager.setConfigPortalTimeout(120);
  if(wifiManager.autoConnect()){
    Serial.println("connected...yeey :)");
    wifiManager.startWebPortal();
  } else {
    Serial.println("non blocking config portal running");
    //wifiManager.startConfigPortal();
  }

  ui.setTargetFPS(30);
  ui.setTimePerFrame(3 * 1000); // Setup frame display time to 10 sec

#ifdef NO_INDICATOR
  //Hack until disableIndicator works:
  //Set an empty symbol
  ui.setActiveSymbol(emptySymbol);
  ui.setInactiveSymbol(emptySymbol);

  ui.disableIndicator();
#else
  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);
#endif

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, numberOfFrames);

  ui.setOverlays(overlays, numberOfOverlays);

  //Manual Wifi
  // WiFi.begin(SSID, PASSWORD);
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  // Setup OTA
  Serial.println("Hostname: " + hostname);
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.onProgress(drawOtaProgress);
  ArduinoOTA.begin();

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Initiazile MQTT
  initMqtt();

  initData(&display);

  dhtDisplay.setUpdateCallback(dhtUpdateCallback);
  //bmeDisplay.setUpdateCallback(bmeUpdateCallback);

  weatherDisplay.init(UPDATE_INTERVAL_SECS);
  forecastDisplay.init(UPDATE_INTERVAL_SECS);
  sunmoonDisplay.init(UPDATE_INTERVAL_SECS * 4);
  dhtDisplay.init(60);
  bmeDisplay.init(60);
  ntcDisplay.init(5);

  ui.getUiState()->userData = parameters;
}

void loop() {
  if (ui.getUiState()->frameState == FIXED) {
    weatherDisplay.triggerUpdate();
    forecastDisplay.triggerUpdate();
    sunmoonDisplay.triggerUpdate();
    dhtDisplay.triggerUpdate();
    bmeDisplay.triggerUpdate();
    ntcDisplay.triggerUpdate();
  }

  updateMqtt();

  wifiManager.process();

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    ArduinoOTA.handle();
    delay(remainingTimeBudget);
  }

}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "Wifi Manager");
  display.drawString(64, 20, "Please connect to AP");
  display.drawString(64, 48, myWiFiManager->getConfigPortalSSID());
  display.drawString(64, 30, "To setup Wifi Configuration");
  display.display();
}

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 12, percentage);
  display->display();
}

void drawOtaProgress(unsigned int progress, unsigned int total) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "OTA Update");
  display.drawProgressBar(2, 28, 124, 12, progress / (total / 100));
  display.display();
}

void initData(OLEDDisplay *display) {
  drawProgress(display, 10, "Updating time...");
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);

  drawProgress(display, 30, "Updating weather...");
  weatherDisplay.update();
  sunmoonDisplay.setLatLon(weatherDisplay.getData().lat, weatherDisplay.getData().lon);

  drawProgress(display, 40, "Updating forecast...");
  forecastDisplay.update();

  drawProgress(display, 50, "Updating astronomy...");
  sunmoonDisplay.update();

  //Connect to MQTT
  if (config.mqttBrokerURL) {
    drawProgress(display, 60, "Connecting to MQTT...");
    connectToMqtt();
  }

  drawProgress(display, 70, "Updating Sensors");
  dhtDisplay.update();
  bmeDisplay.update();
  ntcDisplay.update();

  drawProgress(display, 100, "Done...");
  //delay(1000);
}


void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  char *dstAbbrev;
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm * timeinfo = localtime(&now);

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String date = ctime(&now);
  date = date.substring(0, 11) + String(1900 + timeinfo->tm_year);
  //int textWidth = display->getStringWidth(date);
  display->drawString(64 + x, 5 + y, date);

  display->setFont(DSEG7_Classic_Bold_21);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);

  char time_str[11];

#ifdef STYLE_24HR
  sprintf(time_str, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  display->drawString(108 + x, 19 + y, time_str);
#else
  int hour = (timeinfo->tm_hour + 11) % 12 + 1; // take care of noon and midnight
  sprintf(time_str, "%2d:%02d:%02d\n", hour, timeinfo->tm_min, timeinfo->tm_sec);
  display->drawString(101 + x, 19 + y, time_str);
#endif

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
#ifdef STYLE_24HR
  sprintf(time_str, "%s", dstAbbrev);
  display->drawString(108 + x, 27 + y, time_str);  // Known bug: Cuts off 4th character of timezone abbreviation
#else
  sprintf(time_str, "%s\n%s", dstAbbrev, timeinfo->tm_hour >= 12?"pm":"am");
  display->drawString(102 + x, 18 + y, time_str);
#endif

}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  char time_str[11];
  time_t now = dstAdjusted.time(nullptr);
  struct tm * timeinfo = localtime(&now);

  display->setFont(ArialMT_Plain_10);
#ifdef NO_INDICATOR
#ifdef STYLE_24HR
  sprintf(time_str, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
  int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
  sprintf(time_str, "%2d:%02d:%02d%s\n", hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_hour >= 12?"pm":"am");
#endif
#else
#ifdef STYLE_24HR
  sprintf(time_str, "%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min);
#else
  int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
  sprintf(time_str, "%2d:%02d%s\n",hour, timeinfo->tm_min, timeinfo->tm_hour >= 12?"pm":"am");
#endif
#endif
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(5, 52, time_str);
#ifdef NO_INDICATOR
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  String temp = String(weatherDisplay.getData().temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(101, 52, temp);
#endif
  int8_t quality = getWifiQuality();
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        display->setPixel(120 + 2 * i, 61 - j);
      }
    }
  }
#ifdef NO_INDICATOR
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Meteocons_Plain_10);
  String weatherIcon = weatherDisplay.getData().iconMeteoCon;
  //int weatherIconWidth = display->getStringWidth(weatherIcon);
  // display->drawString(64, 55, weatherIcon);
  display->drawString(77, 53, weatherIcon);
#endif
  display->drawHorizontalLine(0, 50, 128);
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if(dbm <= -100) {
      return 0;
  } else if(dbm >= -50) {
      return 100;
  } else {
      return 2 * (dbm + 100);
  }
}

void initMqtt() {
  if (config.mqttBrokerURL) {
    mqtt.setServer(config.mqttBrokerURL.c_str(), config.mqttBrokerPort);
    mqtt.setCallback(callback);
  }
}

void updateMqtt() {
  //if (!mqtt.connected()) {
  //  previusConnected = false;
  //  connectToMqtt();
  //} else {
    mqtt.loop();
  //}
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (uint32_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

void connectToMqtt() {
  // Loop until we're reconnected
  if (!mqtt.connected()) {
    // Create client ID
    String clientId = "ESP8266Client-" + String(ESP.getChipId(), HEX);
    Serial.println("clientId: " + clientId);

    Serial.print("Attempting MQTT connection... ");
    // Attempt to connect
    if (mqtt.connect(clientId.c_str(), config.mqttUser.c_str(), config.mqttPassword.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //mqtt.publish("ESP8266Client-AAAAAA", "hello world");
      // ... and resubscribe
      mqtt.subscribe("ESP8266Client-AAAAAB");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //mqttReconnectTimer.once(5, connectToMqtt);
    }
  }
}


void dhtUpdateCallback(DHTDisplay* myDHTDisplay) {
    ThingSpeak.setField(1, myDHTDisplay->getData().temperature);
    ThingSpeak.setField(2, myDHTDisplay->getData().humidity);
    ThingSpeak.setStatus("OK");
    ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_WRITE_KEY);

    String msg = "{\ttemperature: "+ String(myDHTDisplay->getData().temperature, 1) + " \n\thumidity:" + String(myDHTDisplay->getData().humidity, 1) + " }";
    mqtt.publish("ESP8266Client-AAAAAA", msg.c_str());
}

void bmeUpdateCallback(BME280Display* myDHTDisplay) {
    ThingSpeak.setField(1, myDHTDisplay->getData().temperature);
    ThingSpeak.setField(2, myDHTDisplay->getData().humidity);
    ThingSpeak.setStatus("OK");
    ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_WRITE_KEY);

    String msg = "{\ttemperature: "+ String(myDHTDisplay->getData().temperature, 1) + " \n\thumidity:" + String(myDHTDisplay->getData().humidity, 1) + " }";
    mqtt.publish("ESP8266Client-AAAAAA", msg.c_str());
}