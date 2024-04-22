/** 
  This file contains the implementation of the system's main control functions,
  interfacing with various sensors and devices to maintain optimal environmental conditions.
 
  references:
  - Arduino Programming: https://www.arduino.cc/reference/en/
  - LiquidCrystal Library: https://www.arduino.cc/en/Reference/LiquidCrystal
  - Ultrasonic Sensor Library: https://www.arduinolibraries.info/libraries/ultrasonic
  - Exponential Moving Average (EMA): https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
  - Soil Moisture Sensor Guide: https://learn.sparkfun.com/tutorials/soil-moisture-sensor-hookup-guide
  - Robust Error Handling in Embedded Systems: https://www.embedded.com/robust-error-handling-in-embedded-c/
  - Arduino Motor Control Tutorial: https://www.arduino.cc/en/Tutorial/Foundations/Motor
  - Arduino Non-Blocking Code: https://projecthub.arduino.cc/viliamk/easy-and-transparent-non-blocking-code-using-millis-5bf837

 
  Main driver built by Brice Heering
  4-21-2024

  WaterPump header built by James Hornback
  LcdScreen header based off code by lydia, built by Brice Heering
  SoilMoisture header built by Brice Heering
  UltrasonicCode header built by Brice Heering

  pin assignments and connection with wiring and hardware specifics
  were dictated by Dylan and Aiden M


PIN ASSIGNMENTS

DIGITAL:
0-> 
1->
2-> LCD D7
3-> LCD D6
4-> LCD D5
5-> LCD D4
6-> LCD RS
7-> LCD E
8-> Soil Moisture VCC Pin 
9->
10->
11->To transister to power water pump
12->Ultrasonic trigger pin
13->Ultrasonic echo pin

ANALOG:
A0-> Soil Sensor analog, "sig" Pin
A1-> 
A2->
A3->
A4->
A5-> potentiometer middle pin


*/



// Includes of opject headers for the program to work properly
#include "ultrasonicCode.h"
#include "soilMoistureCode.h"
#include "waterPumpCode.h"
#include "lcdScreenCode.h"
// Includes to control the cpu and enter power saving mode
#include <avr/sleep.h>

// Define constants for pin assignments
//Using recommended standard names for pins
#define SOIL_POWER_PIN 8
#define SOIL_SENSOR_PIN A0
#define PUMP_PIN 11
#define ULTRASONIC_TRIG_PIN 12
#define ULTRASONIC_ECHO_PIN 13
#define POTENTIOMETER_PIN A5

// Create object instances for each device in the system.
resSense waterSensor(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
waterPump waterPump(PUMP_PIN);
soilDetector soilSensor(SOIL_POWER_PIN, SOIL_SENSOR_PIN);
lcdScreen lcd(6, 7, 5, 4, 3, 2); //Pins assigned in lcdScreenCode.h

// Variables to manage timing and state for pumping.
int lastActionTime = 0; //Tracks the last time stamp in which the pump ran, used to make code nonblocking
int pumpDuration = 5000; // Duration for which the pump should run, set to 5000 milliseconds (5 seconds).
bool isPumping = false; 

// Setup 
void setup() {
  Serial.begin(9600); // Start serial communication for debug
  pinMode(SOIL_POWER_PIN, OUTPUT); // Set the power pin for the soil sensor as output.
  digitalWrite(SOIL_POWER_PIN, LOW); // Ensure the power pin is initially turned off. IMPORTANT FOR SENSOR LIFETIME!!

  lcd.begin(16, 2); // Initialize the LCD display
  set_sleep_mode(SLEEP_MODE_IDLE);  // Configure the microcontroller to sleep but keep peripherals running, 
                                    //basically stops the cpu but allows LCD and Soil sense to keep working
}

// Main loop
void loop() {
  int currentMillis = millis(); // Get the current time in milliseconds, used for clocking

  performChecks(); // Perform sensor readings and main logic body
  sleep_mode(); // Put the CPU to sleep while waiting
}

// Perform sensor checks and updates to the system state.
void performChecks() {
  if (!isPumping) { // check if currently pumping,
    int potValue = analogRead(POTENTIOMETER_PIN); // Read the value from the potentiometer
    int plantType = map(potValue, 0, 1023, 1, 3); // Map the potentiometer value to a plant type (1 thru 3)
    int soilMoisture = soilSensor.soilMoistLevel(); // Get the current soil moisture level. INCLUDES EMA ADJUSTMENT AND SMOOTHING!

    if (soilMoisture == -1) {  // error is detected in the soil moisture reading
      lcd.displayError("Soil Sensor Error"); // Display the error message on  LCD.
      return; 
    }

    int avgSoilMoisture = soilSensor.getAverageMoisture(); // Calculate the average soil moisture over time.
    if (avgSoilMoisture == -1) {
      lcd.displayError("Avg Soil Error"); // Handle and display errors for moisture readings.
      return;
    }

    lcdUpdate(plantType, soilMoisture, avgSoilMoisture); // Update the LCD display with new data
     // Decide if watering is needed based on the soil moisture, and handle plant watering
    checkPlantWateringNeeds(plantType, avgSoilMoisture, millis()); 
   
  }
}

// Update the information displayed on the LCD 
void lcdUpdate(int plantType, int soilMoisture, int avgSoilMoisture) {

  //Check the state of the water resovoir 
  String tankStatus;
  if (waterSensor.containerStatus() == true){
    tankStatus = "Full";
  }
  else {
    tankStatus = "Empty";
  }

  //Check if the pump is currently active
  String pumpStatus;
  if (isPumping == true){
    pumpStatus = "yes";
  }
  else {
    pumpStatus = "No";
  }
  // Print the status to the LCD.
  lcd.lcdPrint(tankStatus, pumpStatus, avgSoilMoisture, "Plant " + String(plantType)); 
}

// Check if the plant needs watering based on type and soil moisture.
void checkPlantWateringNeeds(int plantType, int avgSoilMoisture, int currentMillis) {
  if (isPumping && currentMillis - lastActionTime >= pumpDuration) {
    waterPump.stop(); // Stop the pump if it has been running for the set duration.
    isPumping = false; // Update the pumping state to false.
  }

  if (!isPumping && waterSensor.containerStatus()) { // If not pumping and the water container is full,
    int threshold = determineThreshold(plantType, avgSoilMoisture); // Determine the moisture threshold needed for watering.

    if (avgSoilMoisture < threshold) { // If the moisture is below the threshold,
      waterPump.runPump(pumpDuration); // Start the pump.
      lastActionTime = currentMillis; // Record the time when the pump started.
      isPumping = true; // Update the pumping state to true.
    }
  }
}

// Determine the threshold for watering based on plant type and current moisture level.
int determineThreshold(int plantType, int moistureLevel) {
  switch (plantType) { // Select the base threshold based on plant type using switch cases
    case 1: return 20 + (moistureLevel < 20 ? 5 : 0); // Changes the threshold needed to water the plant based on the watering needs
    case 2: return 40 + (moistureLevel < 40 ? 5 : 0); //Basically runs three if else statements to check the moisture level provided
    case 3: return 60 + (moistureLevel < 60 ? 5 : 0); //and then calculate from that the actual threshold for the soil sensor
    default: return 100; // Set a high default threshold to prevent unintended watering.
  }
}
