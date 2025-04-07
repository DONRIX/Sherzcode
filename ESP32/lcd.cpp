#include "lcd.h"
#include <Wire.h>

#include "Arduino.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Define LCD here, not in main.ino
String currentScreen = "";  // Define globally so it works in lcd.cpp

void updateLCD(String line1, String line2) {
  String newScreen = line1 + "\n" + line2;  // Create a single string for comparison
  if (newScreen != currentScreen) {  // Only update if different
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    currentScreen = newScreen;  // Save what was displayed
  }
}

// void updateLCD(String line1, String line2) {

//   // if (line1.length() == 0) line1 = " ";  // Prevent empty first line
//   // if (line2.length() == 0) line2 = " ";  // Prevent empty second line

//   // Serial.print("Updating LCD -> Line 1: '"); Serial.print(line1);
//   // Serial.print("', Line 2: '"); Serial.println(line2);
  
//   // Serial.print("Line 1: "); Serial.println(line1);
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print(line1);
//   lcd.setCursor(0, 1);
//   lcd.print(line2);
//   //currentScreen = line1 + "\n" + line2;
// }

