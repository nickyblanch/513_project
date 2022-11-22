/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
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
uint32_t delay = 1800000;             //ms delay between readings (default 30 minutes)
float current_time = 0;               //current time
float constraint_time_lower = 0;      //constraint time lower
float constraint_time_upper = 0;      //constraint time upper

// Synchronous State Machine
uint8_t state = 0;
// 0 = Waiting to request measurement, 1 = Request measurement, 2 = Take measurement, 3 = Post measurement



void setup() {
  Serial.begin(9600);
  max30105.begin();
  max30105.setup(60, 4, 2, 100, 411, 4096);
  Serial.println("Setup finished!");

  // LED Pin
  pinMode(D6, OUTPUT);
}

void loop() {

  // If we are waiting to take a measurement
  if (state == 0) {

    // Update current time
    current_time = float(Time.hour()) + Time.minute() / 60.00;

    // If we are in the acceptable time frame
    if (current_time > constraint_time_lower && current_time < constraint_time_upper) {
      // If we've waited enough between measurements
      if (millis() - previous_request >= delay) {
        // Transition to measurement state
        state = 1;
        previous_request = millis();
        }
    }
    else {
      // Check for updates from Particle cloud
    }
  }

  // If we are requesting a measurement
  if (state == 1) {

    // Enable LED
    RGB.control(true);
    RGB.color(0, 0, 255);

    // If the user places their finger on the sensor
    if () {
      // Transition to measurement state
      state = 2;
    }

    // If it's been 5 minutes and we haven't transitioned yet
    if (millis() - previous_request > 300000)
    
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