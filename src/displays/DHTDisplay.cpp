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

#include "DHTDisplay.h"

DHTDisplay::DHTDisplay(uint8_t pin, uint8_t type, bool metric) : dht(pin, type) {

  this->pin = pin;
  this->metric = metric;

  if (type == DHT22)
    DHTTEXT=  "DHT22";
  else if (type == DHT21)
    DHTTEXT = "DHT21";
  else if (type == DHT11)
    DHTTEXT = "DHT11";

}

void DHTDisplay::init(float seconds) {

  dht.begin(55);
  _init = true;

  tickerUpdate.attach(seconds, std::bind(&DHTDisplay::setReadyForUpdate, this));
}

void DHTDisplay::draw(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  //FrameParameter* parameter = (FrameParameter*) state->userData;

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, DHTTEXT + " Indoor Sensor"); // + String((int)parameter->id)

  display->setFont(Meteocons_Plain_21);
  display->drawString(30 + x, 12, "'");
  display->drawString(30 + x, 30, "M");

  //String formattedTemp = String(temperature, 1);
  //int weatherIconWidth = display->getStringWidth(formattedTemp);

  display->setFont(ArialMT_Plain_16);
  display->drawString(64 + x, 12, " " + (!isnan(data.temperature) ? String(data.temperature, 1) + (metric ? "°C" : "°F") : "N/A"));
  display->drawString(64 + x, 30, " " + (!isnan(data.humidity) ? String(data.humidity, 1) + "%" : "N/A"));

}

void DHTDisplay::triggerUpdate() {
  if (isReadyForUpdate()) {
    update();
  }
}

void DHTDisplay::update() {
  if (!_init)
    return;

  // fix for stablity
  digitalWrite(pin, HIGH);
  delayMicroseconds(55);
/*
  sensors_event_t eventT;
  dht.temperature().getEvent(&eventT);

  sensors_event_t eventH;
  dht.humidity().getEvent(&eventH);

  Serial.print("DHT t: "); Serial.print(eventH.temperature);
  Serial.print(" h: "); Serial.println(eventH.relative_humidity);

  if (!isnan(eventT.temperature) && !isnan(eventH.relative_humidity)) {
    humidity = eventH.relative_humidity;
    temperature = eventH.temperature;

    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setStatus("OK");
    ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_WRITE_KEY);
  }
*/

  float h = dht.readHumidity();
  float t = dht.readTemperature(!metric);

  Serial.print("DHT t: "); Serial.print(t);
  Serial.print(" h: "); Serial.println(h);

  if (!isnan(h) && !isnan(t) && h>=0 && h<=100) {
    data.humidity = h;
    data.temperature = t;

    if ( _updateCallback != NULL) {
      _updateCallback(this);
    }

  }
  readyForUpdate = false;
}

bool DHTDisplay::isReadyForUpdate() {
  return readyForUpdate;
}

void DHTDisplay::setReadyForUpdate() {
  //Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

boolean DHTDisplay::isValidNumber(String str)
{
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i)))
      return true;
  }
  return false;
}

void DHTDisplay::setUpdateCallback( void (*func)(DHTDisplay* myDHTDisplay) ) {
  _updateCallback = func;
}