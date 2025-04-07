#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include "lcd.h"                // Your custom LCD library
#include "fingerprintid.h"      // Your custom fingerprint functions

// Hardware Definitions
#define MOTOR_STEPS 200         // Steps per revolution for your stepper
#define STEP_PIN 9              // Stepper step pin
#define DIR_PIN 8               // Stepper direction pin
#define EN_PIN 7                // Stepper enable pin (active LOW)
#define INTERRUPT_PIN 3       // Must be interrupt-capable (D2/D3 on most Arduino)
#define BUTTON_PIN 5            // Fingerprint enrollment button
#define SIGNAL_TOESP 13

// Fingerprint Template ID for Emergency Unlock
const int ADMIN_TEMPLATE_ID = 1;

// System State Variables
volatile bool systemLockdown = false;  // Changed by interrupt
bool doorOpen = false;                // Track door state
unsigned long doorOpenTime = 0;        // For auto-closing timer

bool nomatchfound=false;

// Fingerprint Sensor Setup
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2,4);  // RX, TX for fingerprint sensor
#else
#define mySerial Serial
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  // delay(2000);  // Wait 2 seconds to ensure all components are powered up


  pinMode(SIGNAL_TOESP, OUTPUT);

  digitalWrite(SIGNAL_TOESP, HIGH );

  // Initialize hardware
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), lockdownInterrupt, FALLING);
  
  // Motor control pins
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);  // Enable motor driver

  // Serial communication
  Serial.begin(9600);
  // while (!Serial);  // For USB serial only

  // Fingerprint sensor
  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println("Fingerprint sensor not found!");
    updateLCD("ERROR", "Sensor Not Found");
    // while (1);
  }
  
  // System ready
  finger.getTemplateCount();
  Serial.print("Templates stored: ");
  Serial.println(finger.templateCount);
  // updateLCD("SHB Secure Door", "Scan Finger");


  lcd.init();
  lcd.backlight();
  lcd.clear();
  delay(500);
}

void loop() {
  updateLCD("SHB Secure Door", "Scan Finger");
  if (systemLockdown) {
    updateLCD("! LOCKDOWN !", "Admin Required");
    Serial.println("Lockdown activated via interrupt!");
    handleLockdown();
  } else {
    handleNormalOperation();
  }
}

// ======================= INTERRUPT HANDLER =======================
void lockdownInterrupt() {
  systemLockdown = true;
  // digitalWrite(EN_PIN, HIGH);  // Disable motor immediately
  
}

// ===================== NORMAL OPERATION MODE =====================
void handleNormalOperation() {
  digitalWrite(SIGNAL_TOESP, HIGH );

  // Check enrollment button
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(250);  // Simple debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      enrollFingerprint();
      while (digitalRead(BUTTON_PIN) == LOW);  // Wait for release
    }
  }

  // Check fingerprint
  int fingerprintID = getFingerprintID();
  
  if (fingerprintID > 0) {
    updateLCD("FingerPrint", "Recognized");
    delay(1000);
    openDoor();
    delay(10000);
    closeDoor();
  }
  else if (nomatchfound){ 
    nomatchfound=false;
    updateLCD("FingerPrint", "Not Recognized");
    delay(1000);
  }

  // Auto-close door after 10 seconds
  // if (doorOpen && (millis() - doorOpenTime > 10000)) {
  //   closeDoor();
  // }
}

// ======================= LOCKDOWN MODE =======================
void handleLockdown() {
  // Only check for admin fingerprint
  int fingerprintID = getFingerprintID();
  
  if (fingerprintID == ADMIN_TEMPLATE_ID) {
    systemLockdown = false;
    updateLCD("Lockdown Lifted", "Welcome Admin");
    digitalWrite(EN_PIN, LOW);  // Re-enable motor
    digitalWrite(SIGNAL_TOESP, LOW );
    delay(4000);
    digitalWrite(SIGNAL_TOESP, HIGH );
    updateLCD("SHB Secure Door", "Ready - Scan Finger");
  } else if (fingerprintID > 0) {
    updateLCD("INTRUDER ALERT", "Admin Only");
    delay(1000);
    updateLCD("! LOCKDOWN !", "Admin Required");
  }
}

// ===================== DOOR CONTROL FUNCTIONS =====================
void openDoor() {
  updateLCD("Access Granted", "Door Opening...");
  digitalWrite(DIR_PIN, HIGH);  // Set direction
  
  // Move motor (400 steps = 2 revolutions @ 200 steps/rev)
  for (int i = 0; i < 250; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(5000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(5000);
  }
  
  doorOpen = true;
  // doorOpenTime = millis();
  updateLCD("Door Open", "Enter Now");
}

void closeDoor() {
  updateLCD("Closing", "Please Wait...");
  digitalWrite(DIR_PIN, LOW);  // Reverse direction
  
  for (int i = 0; i < 250; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(5000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(5000);
  }
  
  doorOpen = false;
  updateLCD("Door Closed", "");
}

// ====================== FINGERPRINT FUNCTIONS ======================
// (Your existing getFingerprintID() and enrollFingerprint() functions here)
// Ensure getFingerprintID() returns the template ID or 0 if no match