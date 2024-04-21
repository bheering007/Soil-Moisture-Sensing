// Guard to prevent the header file from being included more than once
#ifndef WATER_PUMP_CODE_H
#define WATER_PUMP_CODE_H

#include <Arduino.h> // Include the Arduino base library for basic IO functions and definitions

// Define the waterPump class to manage the operation of a water pump connected to an Arduino
class waterPump {
private:
  int pump_pin; // Private variable to store the Arduino pin number connected to the water pump

public:
  // Constructor for the waterPump class
  // @param pin: the Arduino pin number that controls the water pump
  waterPump(int pin) : pump_pin(pin) { // Initialization list to directly set the pump_pin variable
    pinMode(pump_pin, OUTPUT); // Configure the pump pin as an OUTPUT to control the water pump
  }

  // Method to start the water pump
  // @param duration: the time in milliseconds for which the pump should be active
  // Note: the duration parameter is currently not used to stop the pump after a specific time
  void runPump(int duration) {
    digitalWrite(pump_pin, HIGH); // Set the pump pin to HIGH, turning the pump ON
  }

  // Method to stop the water pump
  void stop() {
    digitalWrite(pump_pin, LOW); // Set the pump pin to LOW, turning the pump OFF
  }
};

// End of the include guard
#endif // WATER_PUMP_CODE_H
