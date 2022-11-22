// ECE 513
// Author: Nicolas Blanchard

// Libraries:
#include "max30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include <Wire.h>

// Definitions
#define MAX_BRIGHTNESS 255

// Global variable delcarations
MAX30105 max30105;                    // Sensor

// Heart Rate & SPO2
int32_t bufferLength;       //data length
int32_t spo2;               //SPO2 value
int8_t validSPO2;           //indicator to show if the SPO2 calculation is valid
int32_t heartRate;          //heart rate value
int8_t validHeartRate;      //indicator to show if the heart rate calculation is valid
String data = String(10);
uint16_t irBuffer[100];     //infrared LED sensor data
uint16_t redBuffer[100];    //red LED sensor data
uint32_t loop_counter = 0;  //loop counter


void setup() {
  Serial.begin(9600);
  max30105.begin();
  max30105.setup();
  max30105.setPulseAmplitudeRed(0x0A);
  max30105.setPulseAmplitudeGreen(0);
}

void loop() {

  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
 
  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++) {
    while (max30105.available() == false) //do we have new data?
      max30105.check(); //Check the sensor for new data

      redBuffer[i] = max30105.getRed();
      irBuffer[i] = max30105.getIR();
      max30105.nextSample(); //We're finished with this sample so move to next sample

      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.println(irBuffer[i], DEC);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (max30105.available() == false) //do we have new data?
        max30105.check(); //Check the sensor for new data

      redBuffer[i] = max30105.getRed();
      irBuffer[i] = max30105.getIR();
      max30105.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      // Serial.print(F("red="));
      // Serial.print(redBuffer[i], DEC);
      // Serial.print(F(", ir="));
      // Serial.print(irBuffer[i], DEC);

      // Serial.print(F(", HR="));
      // Serial.print(heartRate, DEC);

      // Serial.print(F(", HRvalid="));
      // Serial.print(validHeartRate, DEC);

      // Serial.print(F(", SPO2="));
      // Serial.print(spo2, DEC);

      // Serial.print(F(", SPO2Valid="));
      // Serial.println(validSPO2, DEC);
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    if (loop_counter % 3000 == 0) {
      String send_data = String("{ \"beat\": \"") + String(heartRate) + "\"" + ", \"ox\": " + String(spo2) + "}";
      Particle.publish("Reading", String(send_data), PRIVATE);
    }
    Serial.println(loop_counter);
    Serial.println(String("HR:") + heartRate + ", O2: " + spo2);
  }
}



/*
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
*/