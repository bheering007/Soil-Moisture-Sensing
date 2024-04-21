/** 
  This file contains the implementation of the system's main control functions,
  interfacing with various sensors and devices to maintain optimal environmental conditions.
 
  References:
  - Arduino Programming: https://www.arduino.cc/reference/en/
  - LiquidCrystal Library: https://www.arduino.cc/en/Reference/LiquidCrystal
  - Ultrasonic Sensor Library: https://www.arduinolibraries.info/libraries/ultrasonic
  - Exponential Moving Average (EMA): https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
  - Soil Moisture Sensor Guide: https://learn.sparkfun.com/tutorials/soil-moisture-sensor-hookup-guide
  - Robust Error Handling in Embedded C: https://www.embedded.com/robust-error-handling-in-embedded-c/
  - Arduino Motor Control Tutorial: https://www.arduino.cc/en/Tutorial/Foundations/Motor
 
  Main driver built by Brice Heering
  4-21-2024

  WaterPump header built by James Hornback
  LcdScreen header based off code by lydia, built by Brice Heering
  SoilMoisture header built by Brice Heering
  UltrasonicCode header built by Brice Heering

  pin assignments and connection with wiring and hardware specifics
  were dictated by Dylan and Aiden M
 */


// Include external header files for various sensors and devices used in the system.
#include "ultrasonicCode.h"
#include "soilMoistureCode.h"
#include "waterPumpCode.h"
#include "lcdScreenCode.h"
#include <avr/sleep.h>

// Define constants for pin assignments to enhance readability and reusability.
#define SOIL_POWER_PIN 8
#define SOIL_SENSOR_PIN A0
#define PUMP_PIN 11
#define ULTRASONIC_TRIG_PIN 12
#define ULTRASONIC_ECHO_PIN 13
#define POTENTIOMETER_PIN A5

// Create object instances for each sensor and device in the system.
resSense waterSensor(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
waterPump waterPump(PUMP_PIN);
soilDetector soilSensor(SOIL_POWER_PIN, SOIL_SENSOR_PIN);
lcdScreen lcd(6, 7, 5, 4, 3, 2);

// Variables to manage timing and state for pumping.
unsigned long lastActionTime = 0;
unsigned long pumpDuration = 5000; // Duration for which the pump should run, set to 5000 milliseconds (5 seconds).
bool isPumping = false;

// Setup function, runs once when the system starts.
void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud rate.
  pinMode(SOIL_POWER_PIN, OUTPUT); // Set the power pin for the soil sensor as output.
  digitalWrite(SOIL_POWER_PIN, LOW); // Ensure the power pin is initially turned off.

  lcd.begin(16, 2); // Initialize the LCD display to 16 columns and 2 rows.
  set_sleep_mode(SLEEP_MODE_IDLE);  // Configure the microcontroller to sleep but keep peripherals running.
}

// Main loop, executes continuously after the setup function.
void loop() {
  unsigned long currentMillis = millis(); // Get the current time in milliseconds.

  performChecks(); // Perform sensor readings and decision making.
  sleep_mode(); // Put the CPU to sleep while waiting for the next loop iteration.
}

// Perform sensor checks and updates to the system state.
void performChecks() {
  if (!isPumping) { // If not currently pumping,
    int potValue = analogRead(POTENTIOMETER_PIN); // Read the value from the potentiometer.
    int plantType = map(potValue, 0, 1023, 1, 3); // Map the potentiometer value to a plant type (1 to 3).
    int soilMoisture = soilSensor.soilMoistLevel(); // Get the current soil moisture level.

    if (soilMoisture == -1) {  // If an error is detected in the soil moisture reading,
      lcd.displayError("Soil Sensor Error"); // Display an error message on the LCD.
      return; // Exit the function to avoid executing with faulty data.
    }

    int avgSoilMoisture = soilSensor.getAverageMoisture(); // Calculate the average soil moisture over time.
    if (avgSoilMoisture == -1) {
      lcd.displayError("Avg Soil Error"); // Handle and display errors for average moisture readings.
      return;
    }

    lcdUpdate(plantType, soilMoisture, avgSoilMoisture); // Update the LCD display with new data.
    checkPlantWateringNeeds(plantType, avgSoilMoisture, millis()); // Decide if watering is needed based on the soil moisture.
  }
}

// Update the information displayed on the LCD screen.
void lcdUpdate(int plantType, int soilMoisture, int avgSoilMoisture) {
  String tankStatus = waterSensor.containerStatus() ? "Full" : "Empty"; // Check if the water container is full or empty.
  String pumpStatus = isPumping ? "Yes" : "No"; // Check if the pump is currently active.
  lcd.lcdPrint(tankStatus, pumpStatus, avgSoilMoisture, "Plant " + String(plantType)); // Print the status to the LCD.
}

// Check if the plant needs watering based on type and soil moisture.
void checkPlantWateringNeeds(int plantType, int avgSoilMoisture, unsigned long currentMillis) {
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
  switch (plantType) { // Select the base threshold based on plant type.
    case 1: return 20 + (moistureLevel < 20 ? 5 : 0); // Adjust threshold if current moisture is below a baseline.
    case 2: return 40 + (moistureLevel < 40 ? 5 : 0);
    case 3: return 60 + (moistureLevel < 60 ? 5 : 0);
    default: return 100; // Set a high default threshold to prevent unintended watering.
  }
}
