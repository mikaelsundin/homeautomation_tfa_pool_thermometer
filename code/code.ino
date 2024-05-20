#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "tfa433.h"

#define PIN_RFINPUT  2

TFA433 tfa = TFA433(); //Input pin where 433 receiver is connected.

StaticJsonDocument<200> doc; 


void setup() {
  Serial.begin(115200);
  tfa.start(PIN_RFINPUT);
}

void loop() {
  if (tfa.isDataAvailable()) {    
    char id[100];

    tfaResult result = tfa.getData();

    //generate json data.
    doc.clear();
    doc["manufacture"] = F("tfa");
    doc["model"] = F("30.3240.10");

    doc["id"] = (result.id * 10) + result.channel; 
    doc["temperature"] = ((float)result.temperature / 10.0f) ;

    //output json via serial
    serializeJson(doc, Serial);
    Serial.println();
  }
}

