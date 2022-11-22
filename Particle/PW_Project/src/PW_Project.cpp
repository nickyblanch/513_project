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
#include "spo2_algorithm.h"
#include <Wire.h>

// Definitions
void setup();
void loop();
#line 11 "c:/Users/nicky/OneDrive/Documents/GitHub/513_project/Particle/PW_Project/src/PW_Project.ino"
#define MAX_BRIGHTNESS 255

// Global variable delcarations
MAX30105 max30105;                    // Sensor
int32_t bufferLength;                 //data length
int32_t spo2;                         //SPO2 value
int8_t validSPO2;                     //indicator to show if the SPO2 calculation is valid
int32_t heartRate;                    //heart rate value
int8_t validHeartRate;                //indicator to show if the heart rate calculation is valid
String data = String(10);             //data to be sent to server
uint16_t irBuffer[300];               //infrared LED sensor data
uint16_t redBuffer[300];              //red LED sensor data
uint32_t loop_counter = 0;            //loop counter
uint32_t validated_hr = 0;            //validated heart rate data
uint32_t validated_o2 = 0;            //validated o2 data
uint32_t previous_request = 0;        //time of the previous sensor data request
uint16_t delay = 1800000;             //ms delay between readings (default 30 minutes)
uint8_t current_minute = 0;           //current minute
uint8_t current_hour = 0;             //current hour
uint8_t constraint_minute_lower = 0;  //constraint minute
uint8_t constraint_hour_lower = 0;    //constraint hour
uint8_t constraint_minute_upper = 0;  //constraint minute (upper)
uint8_t constraint_hour_upper = 0;    //constraint hour (upper)

// Synchronous State Machine
uint8_t state = 0;
// 0 = Waiting to request measurement, 1 = Request measurement, 2 = Take measurement, 3 = Post measurement



void setup() {
  Serial.begin(9600);
  max30105.begin();
  max30105.setup(60, 4, 2, 100, 411, 4096);
  Serial.println("Setup finished!");
}

void loop() {

  // If we are waiting to take a measurement
  if (state == 0) {

    // Update current time

    if (millis() - previous_request >= delay) {
      if ()
    // Transition to measurement state
    state = 1;
    previous_request = millis();
    }
    else {
      // Check for updates from Particle cloud
    }
  }

  // If we are requesting a measurement
  if (state == 1) {

    // Transition to measurement state
    state = 2;
  }

  // If we are taking a measurement:
  if (state == 2) {

    bufferLength = 300; //buffer length of 100 stores 4 seconds of samples running at 25sps
  
    //read the first 300 samples, and determine the signal range
    for (byte i = 0 ; i < bufferLength ; i++) {
      while (max30105.available() == false) //do we have new data?
        max30105.check(); //Check the sensor for new data

        redBuffer[i] = max30105.getRed();
        irBuffer[i] = max30105.getIR();
        max30105.nextSample(); //We're finished with this sample so move to next sample
    }

    //After gathering 300 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    if(validSPO2) {
      validated_o2 = spo2;
    }
    if(validHeartRate){
      validated_hr = heartRate;
    }

    // Transition to post state
    state = 3;
  }

  // If we are posting data to the server
  if (state == 3) {
      String send_data = String("{ \"beat\": \"") + String(validated_hr) + "\"" + ", \"ox\": " + String(validated_o2) + "}";
      Particle.publish("Reading", String(send_data), PRIVATE);
      Serial.println(send_data);

      // Transition to wait state
      state = 0;
  }
}