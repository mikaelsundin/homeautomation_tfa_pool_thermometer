#include "tfa433.h"

TFA433 tfa = TFA433(); //Input pin where 433 receiver is connected.

void setup() {
  Serial.begin(9600);
  tfa.start(2);
}

void loop() {
  if (tfa.isDataAvailable()) {
	char txt[100];

    
	tfaResult result = tfa.getData();

    //output as json
	sprintf(txt, "{\"type\":\"TFA_30.3240.10\", \"id\": %d, \"channel\": %d, \"temperature\": %d.%d}\n", result.id, result.channel, result.temperature / 10, result.temperature % 10);
	Serial.print(txt);
  }
}
