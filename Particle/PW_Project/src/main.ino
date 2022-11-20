// ECE 513
// Author: Nicolas Blanchard
//#include <Arduino.h>
//MAX30105 max30105;

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