// fingerprintid.h
#ifndef FINGERPRINTID_H
#define FINGERPRINTID_H

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

extern Adafruit_Fingerprint finger;  // Declare the fingerprint sensor object
//extern int attempts;  // Declare, but don't define
uint8_t getFingerprintID();
void enrollFingerprint();

#endif
