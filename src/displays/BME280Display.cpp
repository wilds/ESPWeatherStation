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

#include "BME280Display.h"

BME280Display::BME280Display(bool metric) : bme() {

  this->pin = pin;
  this->metric = metric;

}

void BME280Display::init(float seconds) {

  unsigned status = bme.begin();

  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      return;
  }

  Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);

  _init = true;
  tickerUpdate.attach(seconds, std::bind(&BME280Display::setReadyForUpdate, this));
}

void BME280Display::draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  //FrameParameter* parameter = (FrameParameter*) state->userData;

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, BMETEXT + " Indoor Sensor"); // + String((int)parameter->id)

  display->setFont(Meteocons_Plain_21);
  display->drawString(30 + x, 12, "'");
  display->drawString(30 + x, 30, "M");

  //String formattedTemp = String(temperature, 1);
  //int weatherIconWidth = display->getStringWidth(formattedTemp);

  display->setFont(ArialMT_Plain_16);
  display->drawString(64 + x, 12, " " + (!isnan(data.temperature) ? String(data.temperature, 1) + (metric ? "°C" : "°F") : "N/A"));
  display->drawString(64 + x, 30, " " + (!isnan(data.humidity) ? String(data.humidity, 1) + "%" : "N/A"));

}

void BME280Display::draw2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  //FrameParameter* parameter = (FrameParameter*) state->userData;

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, BMETEXT + " Indoor Sensor"); // + String((int)parameter->id)

  display->setFont(Meteocons_Plain_21);
  display->drawString(14 + x, 12, "(");
  display->drawString(14 + x, 30, ")");

  //String formattedTemp = String(temperature, 1);
  //int weatherIconWidth = display->getStringWidth(formattedTemp);

  display->setFont(ArialMT_Plain_16);
  display->drawString(64 + x, 12, " " + (!isnan(data.pressure) ? String(data.pressure, 1) + "hPa" : "N/A"));
  display->drawString(64 + x, 30, " " + (!isnan(data.altitude) ? String(data.altitude, 1) + "m" : "N/A"));

}

void BME280Display::triggerUpdate() {
  if (isReadyForUpdate()) {
    update();
  }
}

void BME280Display::update() {
  if (!_init)
    return;

  float t = bme.readTemperature(/*!metric*/);
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0F;  //hPa
  float a = bme.readAltitude(SEALEVELPRESSURE_HPA); //m

  Serial.print("BME280 t: "); Serial.print(t);
  Serial.print(" h: "); Serial.print(h);
  Serial.print(" p: "); Serial.print(p);
  Serial.print(" a: "); Serial.println(a);

  if (!isnan(h) && !isnan(t) && h>=0 && h<=100) {
    data.temperature = t;
    data.humidity = h;
    data.pressure = p;
    data.altitude = a;

    if ( _updateCallback != NULL) {
      _updateCallback(this);
    }

  }
  readyForUpdate = false;
}

bool BME280Display::isReadyForUpdate() {
  return readyForUpdate;
}

void BME280Display::setReadyForUpdate() {
  //Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

void BME280Display::setUpdateCallback( void (*func)(BME280Display* myBME280Display) ) {
  _updateCallback = func;
}