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

#include "ForecastDisplay.h"

const char* WDAY_NAMES[] = {"DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB"};
const char* MONTH_NAMES[] = {"GEN", "FEB", "MAR", "APR", "MAG", "GIU", "LUG", "AGO", "SET", "OTT", "NOV", "DIC"};

ForecastDisplay::ForecastDisplay(String appId, String location, bool metric, String language)  {
  this->appId = appId;
  if (isValidNumber(location)) {
    this->locationId = location;
  } else {
    this->location = location;
  }
  this->metric = metric;
  this->language = language;
}

void ForecastDisplay::init(float seconds) {
  tickerUpdate.attach(seconds, std::bind(&ForecastDisplay::setReadyForUpdate, this));
}

void ForecastDisplay::draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void ForecastDisplay::draw2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  drawForecastDetails(display, x, y, 3);
  drawForecastDetails(display, x + 44, y, 4);
  drawForecastDetails(display, x + 88, y, 5);
}


void ForecastDisplay::drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  if (forecasts[dayIndex].observationTime == 0)
    return;

  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm* timeInfo;
  timeInfo = localtime(&observationTimestamp);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, WDAY_NAMES[timeInfo->tm_wday]);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, forecasts[dayIndex].iconMeteoCon);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, String(forecasts[dayIndex].temp, 0) + (metric ? "°C" : "°F"));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void ForecastDisplay::triggerUpdate() {
  if (isReadyForUpdate()) {
    update();
  }
}

void ForecastDisplay::update() {
  forecastClient.setMetric(metric);
  forecastClient.setLanguage(language);
  uint8_t allowedHours[] = {12};
  forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
  if (locationId.length() > 0)
    forecastClient.updateForecastsById(forecasts, appId, locationId, MAX_FORECASTS);
  else
    forecastClient.updateForecasts(forecasts, appId, location, MAX_FORECASTS);

  readyForUpdate = false;
}

bool ForecastDisplay::isReadyForUpdate() {
  return readyForUpdate;
}

void ForecastDisplay::setReadyForUpdate() {
  //Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

boolean ForecastDisplay::isValidNumber(String str)
{
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i)))
      return true;
  }
  return false;
}