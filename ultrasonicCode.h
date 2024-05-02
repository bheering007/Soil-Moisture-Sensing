// Ensure this header is included only once in any compilation unit.
#ifndef ULTRASONIC_CODE_H
#define ULTRASONIC_CODE_H

#include <Ultrasonic.h>  // Include the library for handling ultrasonic sensors.

class resSense {
private:
    // Private member to hold the Ultrasonic object which handles the ultrasonic sensor operations.
    Ultrasonic ultrasonic;

public:
    // Constructor to initialize the Ultrasonic object with specified trigger and echo pins.
    resSense(int triggerPin, int echoPin) : ultrasonic(triggerPin, echoPin) {}

    // Function to determine the status of the container based on the ultrasonic sensor readings.
    bool containerStatus() {
        int distance = ultrasonic.read();  // Measure the distance using the ultrasonic sensor.

        // Check if the distance reading is either 0 (indicating no echo received)
        // or greater than 300 (which we consider beyond our measurement interest).
        if (distance == 0 || distance > 300) {
            return false;  // Return false if the measurement is out of the expected range,
                           // assuming this indicates a problem or the container is empty.
        }

        // Consider the container as not full if the distance to the surface is greater than 20 units.
        // You might adjust this threshold based on the specific application needs.
        return distance < 20;  // Return true if the measured distance is less than 20 units,
                               // shows that the container is sufficiently full.
    }
};

#endif // ULTRASONIC_CODE_H
