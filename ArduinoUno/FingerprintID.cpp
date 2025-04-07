#include "Arduino.h"
#include "fingerprintid.h"  // Corrected capitalization
#include "lcd.h"  // Include LCD functions
#include <Adafruit_Fingerprint.h>

extern bool nomatchfound;

uint8_t getFingerprintID() {
  delay(500);
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("✅ Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("⚠️ No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("❌ Communication error1");
      return 0;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("❌ Imaging error");
      return 0;
    default:
      Serial.println("❌ Unknown error");
      return 0;
  }

  // Image successfully taken
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("✅ Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("❌ Image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("❌ Communication error2");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("❌ Could not find fingerprint features");
      return 0;
    default:
      Serial.println("❌ Unknown error");
      return 0;
  }

  // Search for a fingerprint match
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Found a print match!");
    Serial.print("🔍 Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);

    updateLCD("Fingerprint", "Recognized!");  // Fixed function call
    return finger.fingerID;
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("❌ Communication error3");
    return 0;
  } 
  else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("❌ Did not find a match");
    //attempts++;  // Increase failed attempt count
    //Serial.print("⚠️ Attempt "); Serial.print(attempts);
    //Serial.println("/5 failed");
    

    // updateLCD("Fingerprint", "Not Recognized!!");
    // delay(2000);
    nomatchfound = true;
    return 0;
  } else {
    Serial.println("❌ Unknown error");
    return 0;
  }
}

int findEmptyID() {
    for (int id = 1; id <= 127; id++) {  // Fingerprint sensor supports IDs from 1 to 127
        if (finger.loadModel(id) != FINGERPRINT_OK) {
            return id;  // Return first empty slot
        }
    }
    return -1;  // No empty slots found
}


void enrollFingerprint() {
    int id = findEmptyID();  // Find first free template slot
    if (id == -1) {
        Serial.println("No free slots available!");
        return;
    }
    Serial.print("Enrolling fingerprint at ID: ");
    Serial.println(id);

    Serial.println("Place your finger...");
    updateLCD("Enrollment Mode", "Place Finger..");

    unsigned long startTime1 = millis();  // Record the start time

    while (millis() - startTime1 < 10000) {  // Check if 10 seconds passed
        if (finger.getImage() == FINGERPRINT_OK) {  // Fingerprint detected
            Serial.println("Fingerprint Captured.");
            // Continue with enrollment process...
            break;
        }
        
    }

    if (millis() - startTime1 >= 10000) {  // Check timeout
        Serial.println("⏳ Enrollment timeout! Exiting...");
        updateLCD("Timeout", "Try Again!");
        return;
    //while (finger.getImage() != FINGERPRINT_OK);
    }   

    if (finger.image2Tz(1) != FINGERPRINT_OK) {
        Serial.println("Failed to process first scan!");
        return;
    }

    Serial.println("Remove finger...");
    updateLCD("Remove finger..."," ");
    delay(1000);
    Serial.println("Place the same finger again...");
    updateLCD("Place Finger", "Again!!!");

    unsigned long startTime2 = millis();  // Record the start time

    while (millis() - startTime2 < 10000) {  // Check if 10 seconds passed
        if (finger.getImage() == FINGERPRINT_OK) {  // Fingerprint detected
            Serial.println("Fingerprint Captured.");
            // Continue with enrollment process...
            break;
        }
    }
   // while (finger.getImage() != FINGERPRINT_OK);

    if (finger.image2Tz(2) != FINGERPRINT_OK) {
        Serial.println("Failed to process second scan!");
        return;
    }

    if (finger.createModel() != FINGERPRINT_OK) {
        Serial.println("Fingerprint match failed!");
        return;
    }

    if (finger.storeModel(id) != FINGERPRINT_OK) {
        Serial.println("Failed to store fingerprint!");
        return;
    }

    Serial.println("Fingerprint enrolled successfully!");
    updateLCD("Fingerprint", "registered OK");
    delay(1000);
}



