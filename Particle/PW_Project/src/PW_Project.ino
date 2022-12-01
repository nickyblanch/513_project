
/*******************************************************************/
// Course:        ECE 513 Fall 2022
// Instructor:    Soheil Salehi
// Author:        Nicolas Blanchard
// Team members:  Elliot Zuercher & Gavin Caldwell
// Contact:       nickyblanch@arizona.edu
// Description:
//                Building 'PW_Project.cpp' generates firmware
//                for a Particle Argon IOT device running a
//                max30105 UV sensor to act as a heart rate
//                and blood oxygen level monitor. The device
//                interfaces with a custom web service through
//                Particle's webhook based event API.
/*******************************************************************/

// Libraries:
#include "Particle.h"       // Particle library
#include "max30105.h"       // Downloaded from Sparkfun
#include "heartRate.h"      // Downloaded from Sparkfun
#include "spo2_algorithm.h" // Downloaded from Sparkfun
#include <Arduino.h>

// Definitions and function declarations
void setup();
void loop();
void updateHandler(const char *event, const char *data);
void readingHandler(const char *event, const char *data);
String return_string(uint8_t start, uint8_t end, const char* input);
#define MAX_BRIGHTNESS 255

// Global variable delcarations
MAX30105 max30105;                    // Sensor
int32_t bufferLength;                 //data length
int32_t spo2;                         //SPO2 value
int8_t validSPO2;                     //indicator to show if the SPO2 calculation is valid
int32_t heartRate;                    //heart rate value
int8_t validHeartRate;                //indicator to show if the heart rate calculation is valid
String data = String(10);             //data to be sent to server
uint16_t irBuffer[100];               //infrared LED sensor data
uint16_t redBuffer[100];              //red LED sensor data
uint32_t loop_counter = 0;            //loop counter
uint32_t validated_hr = 0;            //validated heart rate data
uint32_t validated_o2 = 0;            //validated o2 data
uint32_t previous_request = 0;        //time of the previous sensor data request
uint32_t delay_time = 1800000;        //ms delay between readings (default 30 minutes)
float current_time = 0;               //current time
float constraint_time_lower = 6;      //constraint time lower
float constraint_time_upper = 22;     //constraint time upper
String storage_buffer[48];            //storage for measurements
uint8_t stored_data_points = 0;       //number of stored data points
uint32_t when_stored[48];             //when measurements are stored

LEDStatus blinkLED(RGB_COLOR_BLUE, LED_PATTERN_BLINK, LED_SPEED_FAST, LED_PRIORITY_IMPORTANT);  //LED control

// Synchronous State Machine
uint8_t state = 0;
// 0 = Waiting to request measurement
// 1 = Request measurement
// 2 = Take measurement
// 3 = Post measurement

// Web Hook Event Handler
void updateHandler(const char *event, const char *data) {
  // Handle the integration response
  Serial.println(data);
  // rate
  // rangeStart
  // rangeEnd

  // If data is returned
  if (data) {

    uint8_t i = 0;
    uint8_t data_part = 0;
    uint8_t starting_location = 0;
    uint8_t ending_location = 0;

    // For every character in the returned data:
    while (*(data + i) != '\0') {

      // If we find a semicolon:
      if (*(data + i) == ':') {

        starting_location = i;

      }
      // If we find a comma:
      else if (*(data+i) == ',') {

        ending_location = i;

        // If we're in the rate portion:
        if (data_part == 0) {
          delay_time = return_string(starting_location, ending_location, data).toInt() * 60 * 1000;

          // DEBUG
          Serial.println("Updated delay time to: " + String(delay_time));
        }
        // If we're in the start portion:
        if (data_part == 1) {
          constraint_time_lower = return_string(starting_location, ending_location, data).toInt();

          // DEBUG
          Serial.println("Updated start range to: " + String(constraint_time_lower));
        }
    
        // Move on to next portion:
        data_part = data_part + 1;

      }
      // If we find the end of the string:
      else if (*(data+i) == '}') {
        // We're in the end portion:
        constraint_time_upper = return_string(starting_location, i, data).toInt();

        // DEBUG
        Serial.println("Updated range end to: " + String(constraint_time_upper));
      }
      i++;

    }
  }
}

// Web Hook Event Handler
void readingHandler(const char *event, const char *data) {
  // Handle the integration response
  Serial.println(data);
}

// Find string returned in web hook event data
String return_string(uint8_t start, uint8_t end, const char* input) {
  uint8_t index = start + 1;
  String temp = "";
  while (index < end) {
    temp = temp + input[index];
    index++;
  }
  return temp;
}

// Setup Function
void setup() {

  Serial.begin(9600);
  Serial.println("Entering setup function.");
  max30105.begin();
  max30105.setup(60, 4, 2, 100, 411, 4096);
  Serial.println("Setup finished!");

  // Subscribe to the integration response event
  Particle.subscribe("hook-response/Update", updateHandler, MY_DEVICES);
  Particle.subscribe("hook-response/Reading", readingHandler, MY_DEVICES);

  // Timezone
  Time.zone(-7);

   // Update LED
  blinkLED.setColor(RGB_COLOR_GREEN);
  blinkLED.setSpeed(LED_SPEED_SLOW);
  blinkLED.setPattern(LED_PATTERN_FADE);
  if (!blinkLED.isActive()) blinkLED.setActive(true);
  blinkLED.on();

}

// Loop Function
void loop() {

  // If we are waiting to take a measurement
  if (state == 0) {

    // Update current time
    current_time = float(Time.hour()) + float(Time.minute()) / 60.00;
    if(current_time < 0) {
      current_time += 24.0;
    }

    // If we are in the acceptable time frame
    if (current_time > constraint_time_lower && current_time < constraint_time_upper) {

      // If we've waited enough between measurements
      if (millis() - previous_request >= 60000) { //delay_time) {

        // Transition to measurement state
        state = 1;
        previous_request = millis();
        Serial.println("Conditions met, preparing to take a measurement!");
      }
      else {
        Serial.println("Waiting for a measurement, but it hasn't been long enough yet.");
        Particle.publish("Update", String("dummy_data"), PRIVATE);

      }
    }
    else {
      Serial.println("Waiting for a measurement, but it's not in the acceptable time frame. Current time: " + String(current_time));
      // Check for updates from Particle cloud
      Particle.publish("Update", String("dummy_data"), PRIVATE);
    }

    delay(10000);
  }

  // If we are requesting a measurement
  if (state == 1) {

    Serial.println("Requesting a measurement - place finger on sensor.");

    // Enable LED
    // blinkLED.setColor((uint8_t(0) << 16) | (uint8_t(0) << 8) | uint8_t(255));
    // blinkLED.setPattern(LED_PATTERN_BLINK);
    // blinkLED.setSpeed(LED_SPEED_NORMAL);
    blinkLED.setColor(RGB_COLOR_BLUE);
    blinkLED.setPattern(LED_PATTERN_BLINK);
    blinkLED.setSpeed(LED_SPEED_FAST);
    if (!blinkLED.isActive()) blinkLED.setActive(true);
    blinkLED.on();

    // If the user places their finger on the sensor
    if (1) {

      Serial.println("Finger detected on sensor."); 
      // Transition to measurement state
      state = 2;
    }
    
  }

  // If we are taking a measurement:
  if (state == 2) {

    bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
  
    //read the first 300 samples, and determine the signal range
    Serial.println("Entering measurement loop");
    for (byte i = 0 ; i < bufferLength ; i++) {
      while (max30105.available() == false) //do we have new data?
        max30105.check(); //Check the sensor for new data

        redBuffer[i] = max30105.getRed();
        irBuffer[i] = max30105.getIR();
        max30105.nextSample(); //We're finished with this sample so move to next sample
    }

    //After gathering 100 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    //Transition to post state
    if ((50 < spo2) && (spo2 < 101) && (30 < heartRate) && (heartRate < 300)) {
      Serial.println("Valid measurement, sending data.");
      state = 3;
    }
    // If it's been 5 minutes and we haven't transitioned yet
    else if (millis() - previous_request > 600000) {
      // Update LED
      blinkLED.setColor(RGB_COLOR_GREEN);
      blinkLED.setSpeed(LED_SPEED_SLOW);
      blinkLED.setPattern(LED_PATTERN_FADE);

      // Back to waiting state
      state = 0;
    }
    else {
      Serial.println("Not a valid measurement. HR:" + String(heartRate) + " SPO2: " + String(spo2));
      state = 2; // Try getting a reading again
    }

  }

  // If we are posting data to the server
  if (state == 3) {

      // String send_data = String("{ \"beat\": \"") + String(heartRate) + "\"" + ", \"ox\": " + String(spo2) + "}";
      String send_data = String("{ \"beat\": \"") + String(heartRate) + "\"" + ", \"ox\": " + String(spo2) + ", \"time\": " + String(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL)) + "}";
      bool success = Particle.publish("Reading", String(send_data), PRIVATE);
      Serial.println(send_data);

      if (success) {
        // Transition to wait state
        state = 0;

        // Update LED
        blinkLED.setColor(RGB_COLOR_GREEN);
        blinkLED.setSpeed(LED_SPEED_SLOW);
        blinkLED.setPattern(LED_PATTERN_FADE);

        // DEBUG
        Serial.println("Data successfully sent.");

        // Try to send backed up data
        uint8_t temp_storage = stored_data_points;
        for (uint8_t i = 0; i < temp_storage; i++) {

          // If data is less than 24 hours old
          if ((millis() - when_stored[i]) < 24*60*60*1000) {

            success = success && Particle.publish("Reading", storage_buffer[i]);

            if (success) {
              // Yay!
              Serial.println("Send stored point: " + String(storage_buffer[i]));
            }
          }
          else {
            Serial.println("Stored data point too old - not sent.");
          }
        }
        if (success) {
          // If all were successful:
          stored_data_points = 0;
          Serial.println("All stored data points sent.");
        }
      }
      else {
        // Save data to be transmitted later
        Serial.println("No internet - saving data for later.");
        storage_buffer[stored_data_points] = send_data;
        when_stored[stored_data_points] = millis();
        stored_data_points++;
        state = 0;

         // Update LED
        blinkLED.setColor(RGB_COLOR_YELLOW);
        blinkLED.setSpeed(LED_SPEED_FAST);
        blinkLED.setPattern(LED_PATTERN_BLINK);
        delay(2000);

         // Re-Update LED
        blinkLED.setColor(RGB_COLOR_GREEN);
        blinkLED.setSpeed(LED_SPEED_SLOW);
        blinkLED.setPattern(LED_PATTERN_FADE);
      }

  }

}