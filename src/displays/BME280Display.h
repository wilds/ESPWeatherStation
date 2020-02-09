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

#ifndef __BME280_DISPLAY_H__
#define __BME280_DISPLAY_H__

#include <OLEDDisplayUi.h>
#include <Ticker.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "fonts/WeatherStationFonts.h"

#define SEALEVELPRESSURE_HPA (1013.25)

typedef struct BME280Return {
    float humidity = NAN;
    float temperature = NAN;
    float pressure = NAN;
    float altitude = NAN;
} BME280Return;

class BME280Display /*: public AbstractDisplay<DHTReturn>*/ {
public:

    BME280Display(bool metric = true);

    void init(float seconds);

    void draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    void draw2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    bool isReadyForUpdate();
    void triggerUpdate();
    void update();

    BME280Return getData() {
        return data;
    }

    //called when AP mode and config portal is started
    void setUpdateCallback( void (*func)(BME280Display*) );
protected:

    void setReadyForUpdate();
    boolean isValidNumber(String str);

    Ticker tickerUpdate;
    bool readyForUpdate = true; // flag changed in the ticker function every 10 minutes

    // Initialize the sensor
    Adafruit_BME280 bme;
    uint8_t pin;
    boolean metric;

    String BMETEXT;

    BME280Return data;

    boolean _init = false;

    void (*_updateCallback)(BME280Display*) = NULL;
};

#endif