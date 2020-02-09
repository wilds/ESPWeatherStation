/**The MIT License (MIT)

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

*/

#include "WeatherDisplay.h"

WeatherDisplay::WeatherDisplay(String appId, String location, bool metric, String language)  {
  this->appId = appId;
  if (isValidNumber(location)) {
    this->locationId = location;
  } else {
    this->location = location;
  }
  this->metric = metric;
  this->language = language;
}

void WeatherDisplay::init(float seconds) {
  tickerUpdate.attach(seconds, std::bind(&WeatherDisplay::setReadyForUpdate, this));
}

void WeatherDisplay::draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 36 + y, currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(55 + x, 5 + y,  String(currentWeather.temp, 1) + (metric ? "°C" : "°F"));

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(30 + x, 0 + y, currentWeather.iconMeteoCon);
}

void WeatherDisplay::triggerUpdate() {
  if (isReadyForUpdate()) {
    update();
  }
}

void WeatherDisplay::update() {
  currentWeatherClient.setMetric(metric);
  currentWeatherClient.setLanguage(language);
  if (locationId.length() > 0)
    currentWeatherClient.updateCurrentById(&currentWeather, appId, locationId);
  else
    currentWeatherClient.updateCurrent(&currentWeather, appId, location);

  readyForUpdate = false;
}

bool WeatherDisplay::isReadyForUpdate() {
  return readyForUpdate;
}

void WeatherDisplay::setReadyForUpdate() {
  //Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

boolean WeatherDisplay::isValidNumber(String str)
{
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i)))
      return true;
  }
  return false;
}