
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <FS.h>
#include <ArduinoJson.h>

class Config
{
 public:

    Config(String file) : _file(file) {

    }

    String _file;

    String mqttBrokerURL = "";   // IP or Address (DO NOT include http://)
    int mqttBrokerPort = 80;     // the port you are running (usually 80);
    String mqttUser = "";
    String mqttPassword = "";


    String apiKey = "";

    // Date and Time
    float UtcOffset = -7; // Hour offset from GMT for your timezone
    boolean IS_24HOUR = false;     // 23:00 millitary 24 hour clock

    String themeColor = "light-blue"; // this can be changed later in the web interface.


    void write() {
        // Save decoded message to SPIFFS file for playback on power up.
        File f = SPIFFS.open(_file, "w");
        if (!f) {
            Serial.println("File open failed!");
        } else {
            Serial.println("Saving settings now...");
            StaticJsonDocument<200> doc;
            doc["mqttBrokerURL"] = this->mqttBrokerURL;
            doc["mqttBrokerPort"] = this->mqttBrokerPort;
            doc["mqttUser"] = this->mqttUser;
            doc["mqttPassword"] = this->mqttPassword;
            doc["apiKey"] = this->apiKey;
            doc["UtcOffset"] = this->UtcOffset;
            doc["is24hour"] = this->IS_24HOUR;
            serializeJson(doc, f);
        }
        f.close();
    }

    void read() {
        if (SPIFFS.exists(_file) == false) {
            Serial.println("Settings File does not yet exists.");
            return;
        }
        File fr = SPIFFS.open(_file, "r");

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, fr);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }

        this->mqttBrokerURL = doc["mqttBrokerURL"].as<String>();
        this->mqttBrokerPort = doc["mqttBrokerPort"].as<int>();
        this->mqttUser = doc["mqttUser"].as<String>();
        this->mqttPassword = doc["mqttPassword"].as<String>();
        this->apiKey = doc["apiKey"].as<String>();
        this->UtcOffset = doc["UtcOffset"].as<float>();
        this->IS_24HOUR = doc["is24hour"].as<int>();

        fr.close();
    }

    bool remove() {
        return SPIFFS.remove(_file);
    }
};

#endif