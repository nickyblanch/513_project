/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/nicky/OneDrive/Documents/GitHub/513_project/Particle/PW_Project/src/main.ino"
// ECE 513
// Author: Nicolas Blanchard

void setup();
void loop();
#line 4 "c:/Users/nicky/OneDrive/Documents/GitHub/513_project/Particle/PW_Project/src/main.ino"
int led = D7;  // The on-board LED

void setup() {
  pinMode(led, OUTPUT);
}

void loop() {
  digitalWrite(led, HIGH);   // Turn ON the LED

  String temp = String(random(60, 80));
  Particle.publish("temp", temp, PRIVATE);
  delay(30000);               // Wait for 30 seconds

  digitalWrite(led, LOW);    // Turn OFF the LED
  delay(3000);               // Wait for 30 seconds
}