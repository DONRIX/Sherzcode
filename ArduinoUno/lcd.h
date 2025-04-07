#ifndef LCD_H
#define LCD_H

#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;  // Declare globally so lcd.cpp and main.ino share the same object
//extern String currentScreen;   // Track last displayed message

void updateLCD(String line1, String line2);

#endif
