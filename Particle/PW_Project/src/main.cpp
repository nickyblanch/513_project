/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/nicky/OneDrive/Documents/GitHub/513_project/Particle/PW_Project/src/main.ino"
// ECE 513
// Author: Nicolas Blanchard
//#include <Arduino.h>
//MAX30105 max30105;

void setup();
void loop();
#line 6 "c:/Users/nicky/OneDrive/Documents/GitHub/513_project/Particle/PW_Project/src/main.ino"
int led = D7;  // The on-board LED

void setup() {
  pinMode(led, OUTPUT);
  //max30105.begin();
}

void loop() {
  // Get some data
  String data = "test string";//String(10);
  // Trigger the integration
  Particle.publish("Temp", data, PRIVATE);
  // Wait 60 seconds
  delay(60000);
}             // Wait for 30 seconds