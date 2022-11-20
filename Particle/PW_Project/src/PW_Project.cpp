/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/nicky/OneDrive/Documents/GitHub/513_project/Particle/PW_Project/src/PW_Project.ino"
// ECE 513
// Author: Nicolas Blanchard

// Libraries:
#include "max30105.h"
#include "heartRate.h"
#include <Wire.h>

// Global variable delcarations
void setup();
void loop();
#line 10 "c:/Users/nicky/OneDrive/Documents/GitHub/513_project/Particle/PW_Project/src/PW_Project.ino"
MAX30105 max30105;                    // Sensor
const byte RATE_SIZE = 4;             // Samples to average
byte rates[RATE_SIZE];                // Store samples
byte rateSpot = 0;long lastBeat = 0;  // Time at last beat
int led = D7;                         // The on-board LED
float beatsPerMinute;
int beatAvg;
String data = String(10);


void setup() {
  pinMode(led, OUTPUT);
  max30105.begin();
  max30105.setup();
  max30105.setPulseAmplitudeRed(0x0A);
  max30105.setPulseAmplitudeGreen(0);

}

void loop() {

  // Get some data
  long irValue = max30105.getIR();

  if(checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta * 1000.00);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for(byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

    data = String(beatsPerMinute);
  }

  // Trigger the integration
  Particle.publish("Reading", "{'beat:'" + String(80) + ",'ox':" + String(50) + "}", PRIVATE); // REPLACE "80" with data

  // Wait 30 seconds
  delay(30000);
}