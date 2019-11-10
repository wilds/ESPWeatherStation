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

#ifndef __FORECAST_DISPLAY_H__
#define __FORECAST_DISPLAY_H__

#include <time.h>
#include <OLEDDisplayUi.h>
#include <Ticker.h>

#include <OpenWeatherMapForecast.h>

#include "fonts/WeatherStationFonts.h"

const uint8_t MAX_FORECASTS = 6;

class ForecastDisplay {
public:

    ForecastDisplay(String appId, String locationId, bool metric = true, String language = "it");

    void init(float seconds);

    void draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    void draw2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);

    bool isReadyForUpdate();
    void triggerUpdate();
    void update();
protected:

    void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);

    void setReadyForUpdate();
    boolean isValidNumber(String str);

    Ticker tickerUpdate;
    bool readyForUpdate = true; // flag changed in the ticker function every 10 minutes

    OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
    OpenWeatherMapForecast forecastClient;

    boolean metric;
    String language;
    String appId;
    String locationId;
    String location;
};

#endif
