/**The MIT License (MIT)

Copyright (c) 2016 by Wilds

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

*/

#include "SunMoonDisplay.h"

SunMoonDisplay::SunMoonDisplay(double lat, double lon, simpleDSTadjust *dstAdjusted) {
  this->dstAdjusted = dstAdjusted;
  this->lat = lat;
  this->lon = lon;
}

void SunMoonDisplay::init(float seconds) {
  tickerUpdate.attach(seconds, std::bind(&SunMoonDisplay::setReadyForUpdate, this));
}

String SunMoonDisplay::formatTime(time_t timestamp) {
  time_t now = dstAdjusted->time(timestamp, nullptr);
  struct tm * timeinfo = localtime(&now);
  char time_str[11];
  if (style24h) {
    sprintf(time_str, "%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min);
  }else {
    int hour = (timeinfo->tm_hour + 11) % 12 + 1; // take care of noon and midnight
    sprintf(time_str, "%2d:%02d%s\n", hour, timeinfo->tm_min, timeinfo->tm_hour >= 12 ? "pm" : "am");
  }
  return String(time_str);
}

void SunMoonDisplay::draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, "Moon"); // + String((int)parameter->id)

  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(55 + x, 12 + y, formatTime(data.moon.rise));

  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(120 + x, 12 + y, formatTime(data.moon.set));

  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 36 + y, data.moon.phase.name + " " + String(data.moon.illumination * 100, 1) + "%");

  display->setFont(Moon_Phases_34);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(30 + x, 0 + y, String(getMoonIcon2(data.moon)));
}

void SunMoonDisplay::draw2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, "Sun"); // + String((int)parameter->id)

  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(55 + x, 12 + y, formatTime(data.sun.rise));

  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(120 + x, 12 + y, formatTime(data.sun.set));

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(30 + x, 0 + y, "B");
}

char SunMoonDisplay::getMoonIcon(SunMoonCalc::Moon moon) {

  /*
    < 1% no moon
    Waxing
    lum:  1.0% - 47.5% -> a - f
    lum: 47.5% - 52.5% -> g
    lum: 52.5% - 99.0% -> h - m
    > 99% full moon -> 0
    Waning
    lum: 99.0% - 52.5% -> n - s
    lum: 52.5% - 47.5% -> t
    lum: 47.5% -  1.0% -> u - z
  */

  float illumination = moon.illumination * 100;

  if (illumination <= 1) {
    return '1';
  } else if (illumination >= 99) {
    return '0';
  } else if (moon.age <= 14.765294 && illumination >= 47.5 && illumination <= 52.5) {
    return 'g';
  } else if (moon.age > 14.765294 && illumination >= 47.5 && illumination <= 52.5) {
    return 't';
  }

  if (moon.age <= 14.765294 && illumination < 47.5) { //New Moon to First Quarter
    int c = (int) ((illumination - 1) / 7.75);
      return 'a' + c;
  } else if (moon.age <= 14.765294 && illumination >= 52.5) { //First Quarter to Full Moon
    int c = (int) ((illumination - 52.5) / 7.75);
      return 'h' + c;
  } else if (moon.age > 14.765294 && illumination >= 52.5) { //Full Moon to Third Quarter
    int c = (int) ((illumination - 52.5) / 7.75);
      return 's' - c;
  } else if (moon.age > 14.765294 && illumination < 47.5) { //Third Quarter to New Moon
    int c = (int) ((illumination - 1) / 7.75);
      return 'z' + c;
  }

  return '1';
}


char SunMoonDisplay::getMoonIcon2(SunMoonCalc::Moon moon) {

  /*
    < 1% no moon
    Waxing
    lum:  1.0% - 47.5% -> N - S
    lum: 47.5% - 52.5% -> T
    lum: 52.5% - 99.0% -> U - Z
    > 99% full moon -> 0
    Waning
    lum: 99.0% - 52.5% -> A - F
    lum: 52.5% - 47.5% -> G
    lum: 47.5% -  1.0% -> H - M
  */

  float illumination = moon.illumination * 100;

  if (illumination <= 1) {
    return '1';
  } else if (illumination >= 99) {
    return '0';
  } else if (moon.age <= 14.765294 && illumination >= 47.5 && illumination <= 52.5) {
    return 'T';
  } else if (moon.age > 14.765294 && illumination >= 47.5 && illumination <= 52.5) {
    return 'G';
  }

  if (moon.age <= 14.765294 && illumination < 47.5) { //New Moon to First Quarter
    int c = (int) ((illumination - 1) / 7.75);
      return 'N' + c;
  } else if (moon.age <= 14.765294 && illumination >= 52.5) { //First Quarter to Full Moon
    int c = (int) ((illumination - 52.5) / 7.75);
      return 'U' + c;
  } else if (moon.age > 14.765294 && illumination >= 52.5) { //Full Moon to Third Quarter
    int c = (int) ((illumination - 52.5) / 7.75);
      return 'F' - c;
  } else if (moon.age > 14.765294 && illumination < 47.5) { //Third Quarter to New Moon
    int c = (int) ((illumination - 1) / 7.75);
      return 'M' + c;
  }

  return '1';
}

void SunMoonDisplay::triggerUpdate() {
  if (isReadyForUpdate()) {
    update();
  }
}

void SunMoonDisplay::update() {
  char *dstAbbrev;
  time_t now = dstAdjusted->time(&dstAbbrev);
  //struct tm *timeinfo = localtime(&now);
  data = SunMoonCalc(now, lat, lon).calculateSunAndMoonData();
  readyForUpdate = false;
}

bool SunMoonDisplay::isReadyForUpdate() {
  return readyForUpdate;
}

void SunMoonDisplay::setReadyForUpdate() {
  //Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

boolean SunMoonDisplay::isValidNumber(String str)
{
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i)))
      return true;
  }
  return false;
}