// Ensure the header file is included only once per compile cycle.
#ifndef SOIL_MOISTURE_CODE_H
#define SOIL_MOISTURE_CODE_H

#include <Arduino.h>

class soilDetector {
private:
    // Pins for controlling and reading from the soil moisture sensor.
    int soilPin;
    int soilPowerPin;

    // Constants and variables for handling and averaging the sensor readings.
   int numReadings = 10;  // Fixed number of readings to average.
    int readings[numReadings];          // Array to store past readings.
    int readIndex = 0;                  // Current index in the readings array.
    int total = 0;                      // Sum of the readings in the array.
    int average = 0;                    // Calculated average of the readings.
    
    // Constants and variables for error handling.
    //do not change
    int errorValue = -1;   // Special value indicating sensor error.
    int sensorLow = 200;   // Minimum expected sensor value.
    int sensorHigh = 850;  // Maximum expected sensor value.
    //you can change agin
    float ema = 0.0;                    // Exponential moving average of the sensor readings.
    //dont change
    float alpha = 0.1;            // Smoothing factor for EMA calculation.
    int errorCount = 0;                 // Counter for consecutive sensor errors.
    //dont change this either lol
    int maxErrorCount = 3;        // Threshold for maximum allowable consecutive errors.

public:
    // Constructor initializes sensor pins and prepares the sensor.
    soilDetector(int powerPin, int sensorPin) : soilPowerPin(powerPin), soilPin(sensorPin) {
        pinMode(soilPowerPin, OUTPUT);
        digitalWrite(soilPowerPin, LOW);  // Ensure sensor power is off initially.
        // Initialize all readings in the buffer to zero.
        for (int thisReading = 0; thisReading < numReadings; thisReading++) {
            readings[thisReading] = 0;
        }
    }

    // Reads and processes the moisture level from the soil sensor.
    // Utilizes Exponential Moving Average (EMA) for smoothing sensor output
// EMA Reference: https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average


/*
  Reads and processes the moisture level from the soil moisture sensor.
 
  This function measures the soil moisture by activating the sensor, reading the analog value,
  and then immediately deactivating the sensor to conserve power. The measurement is processed to
  determine if it falls within a pre-defined valid range. If the reading is outside this range, it is 
  considered an error, and error counting logic is applied. If consecutive errors exceed a defined threshold,
  an error value is returned. Otherwise, the function calculates an exponential moving average (EMA)
  to smooth out the reading fluctuations over time, which helps in mitigating the impact of transient noise
  and outliers in the sensor data.
 
   steps in the function:
  1. Power on the sensor and read the moisture level.
  2. Check if the reading is within the expected range (sensorLow to sensorHigh).
  3. If out of range, increment an error counter. Return an error code if too many consecutive errors occur.
  4. If within range, reset the error counter and update the EMA of moisture levels.
  5. Update a circular buffer with the latest EMA value and compute a running average of these values.
  6. Convert the latest EMA value to a percentage scale for consistency across different sensor calibrations.
 
  References:
  - Exponential Moving Average (EMA): https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
  - Handling Sensor Data: https://learn.sparkfun.com/tutorials/soil-moisture-sensor-hookup-guide
  - Circular Buffer for Data Smoothing: https://en.wikipedia.org/wiki/Circular_buffer
  - Error Handling in Sensor Readings: https://www.embedded.com/design/prototyping-and-development/4008246/Error-handling-in-real-time-measurement-applications

  - C++ code for EMA algorithom https://stackoverflow.com/questions/37300684/implementing-exponential-moving-average-in-c
 
 */

    int soilMoistLevel() {
        digitalWrite(soilPowerPin, HIGH);  // Power on the sensor.
        delay(10);  // Short delay to stabilize sensor 
        int moistureLevel = analogRead(soilPin);  // Read moisture level from sensor.
        digitalWrite(soilPowerPin, LOW);  // Power off the sensor

        // Handle readings outside the expected range by incrementing error count var
        if (moistureLevel < sensorLow || moistureLevel > sensorHigh) {
            errorCount++;
            if (errorCount > maxErrorCount) {
                return errorValue;  // Return an error code if too many consecutive errors.
            }
        } else {
            errorCount = 0;  // Reset error count on a valid reading.

            // Update the exponential moving average.
            ema = alpha * moistureLevel + (1 - alpha) * ema;
            ema = static_cast<int>(ema); //Convert float to int for int math
        }

        // Maintain a running total of the last 'numReadings' EMA values for averaging.

        // Subtracts the oldest reading in the buffer from the running total.
        // Updates the oldest reading slot with the new EMA value.
        //Adds this new EMA value to the running total, effectively replacing the old value in the total with the new one.
        // Advances the index to the next position in the circular buffer, wrapping around if the end is reached.
        //Computes the average of the readings by dividing the total by the number of readings
        total -= readings[readIndex];
        readings[readIndex] = ema;
        total += readings[readIndex];
        readIndex = (readIndex + 1) % numReadings;
        average = total / numReadings;


        // Convert the EMA value to a percentage 
        return map(ema, sensorLow, sensorHigh, 0, 100);
    }

    // Provides a smoothed average moisture level based on recent readings.
    int getAverageMoisture() {
        if (average < sensorLow || average > sensorHigh) {
            return errorValue;  // Return error if average is out of expected range.
        }
        // Map the average value to a percentage.
        return map(average, sensorLow, sensorHigh, 0, 100);
    }
};

#endif // SOIL_MOISTURE_CODE_H
