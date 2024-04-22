// Ensure this header file is included only once per compile cycle.
#ifndef LCD_SCREEN_CODE_H
#define LCD_SCREEN_CODE_H

#include <LiquidCrystal.h> // Include the LiquidCrystal library for handling LCD operations.

// lcdScreen class extends the LiquidCrystal class to add custom functionalities.
class lcdScreen : public LiquidCrystal {
public:
  // Constructor initializes the LCD with pin configurations and default messages.
  lcdScreen(int rs, int en, int d4, int d5, int d6, int d7)
    : LiquidCrystal(rs, en, d4, d5, d6, d7) {
    begin(16, 2); // Initialize the LCD to 16 columns and 2 rows.
    print("Welcome AquaMate");  // Display a welcome message.
    delay(2000);                // Pause for 2 seconds to allow the welcome message to be read.
    clear();                    // Clear the display after the pause.
  }

  // Display operational status and information related to the system.
  void lcdPrint(const String &wtl, const String &wps, int smp, const String &insystem) {
    clear();                    // Clear the LCD
    setCursor(0, 0);            // Set cursor to the beginning of the top row
    print("Moist: " + String(smp) + "%");  // Display soil moisture percentage
    setCursor(0, 1);            // Move the cursor to the beginning of the second line.
    print("Tank: " + wtl + " Pump: " + wps); // Display tank status and pump status
  }

  // Display error messages clearly on the LCD.
  void displayError(const String &error) {
    clear();                    // Clear the screen 
    setCursor(0, 0);            
    print("Error:");            //Show error message with "Error:".
    setCursor(0, 1);            
    print(error); // Display the actual error message, passed in by refrence when called.
  }

  // Display warning messages on the LCD.
  void displayWarning(const String &warning) {
    clear();                    
    setCursor(0, 0);            
    print("Warning:");          // Show "Warning:".
    setCursor(0, 1);            // 
    print(warning);             // Show the actual warning message, passed in.
  }
};

#endif // LCD_SCREEN_CODE_H
