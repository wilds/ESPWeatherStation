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

#include "NTCDisplay.h"

NTCDisplay::NTCDisplay(uint8_t pin, bool metric) {
  this->pin = pin;
  this->metric = metric;
}

void NTCDisplay::init(float seconds) {
  tickerUpdate.attach(seconds, std::bind(&NTCDisplay::setReadyForUpdate, this));
}

void NTCDisplay::draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  //FrameParameter* parameter = (FrameParameter*) state->userData;

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, "NTC Indoor Sensor"); // + String((int)parameter->id)
/*
  display->setFont(Meteocons_Plain_21);
  display->drawString(30 + x, 12, "'");

  display->setFont(ArialMT_Plain_16);
  display->drawString(64 + x, 12, " " + (!isnan(ntcTemperature) ? String(ntcTemperature, 1) + (IS_METRIC ? "°C" : "°F") : "N/A"));

*/

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(55 + x, 17 + y, (!isnan(ntcTemperature) ? String(ntcTemperature, 1) + (metric ? "°C" : "°F") : "N/A"));

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(30 + x, 12 + y, "'");

}

void NTCDisplay::triggerUpdate() {
  if (isReadyForUpdate()) {
    update();
  }
}

void NTCDisplay::update() {
float vThermistor = 0; //analogRead(A0);  // multipex.readValue(1);

  for (int i=0; i < NUMSAMPLES; ++i) {
   ntcSamples[currentPos + i] = analogRead(A0);
   //ntcSamples[currentPos +i ] = multipex.readValue(1);
   delay(10);
  }
  currentPos = (currentPos + NUMSAMPLES);

  if (currentPos > currentBufferSize) {
    currentBufferSize = currentPos;
  }
  currentPos = currentPos % NUMSAMPLES_BUFFERSIZE;
  for (int i = 0; i < currentBufferSize; ++i)
  {
    vThermistor += ntcSamples[i];
  }
  vThermistor /= currentBufferSize;

  float rThermistor = SERIESRESISTOR / (1023 / vThermistor - 1);
  float tKelvin = (BCOEFFICIENT * (TEMPERATURENOMINAL + 273.15)) /  (BCOEFFICIENT + ((TEMPERATURENOMINAL + 273.15) * log(rThermistor / THERMISTORNOMINAL)));
  float tCelsius = tKelvin - 273.15;  // convert kelvin to celsius
  ntcTemperature = tCelsius;

  Serial.print("NTC v: "); Serial.print(vThermistor);
  Serial.print(" r: "); Serial.print(rThermistor);
  Serial.print(" t: "); Serial.println(tCelsius);

  readyForUpdate = false;
}

bool NTCDisplay::isReadyForUpdate() {
  return readyForUpdate;
}

void NTCDisplay::setReadyForUpdate() {
  //Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

boolean NTCDisplay::isValidNumber(String str)
{
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i)))
      return true;
  }
  return false;
}

void NTCDisplay::setUpdateCallback( void (*func)(NTCDisplay* myNTCDisplay) ) {
  _updateCallback = func;
}