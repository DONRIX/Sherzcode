#include <Keypad.h>
#include <Wire.h>
// #include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "lcd.h"

#include <ESP32Servo.h>


#define BUZZER_PIN 2  
#define PUSH_BUTTON_TO_ENTER_PINENTRY 32
#define RELAY 15
#define SERVO_PIN 13  
Servo myServo;
#define OFFSET 0           // Calibration offset
const byte ROWS = 4;
const byte COLS = 3;  // Using 4x3 layout

byte pinattempts =2;




// const char* ssid = "vivo Y33s";
// const char* password = "12345679";

const char* ssid = "LEKSHITH";
const char* password = "12345679";

const int signalPin = 26;;  // TO ARDUINIO

const int signalFromArduino = 33;  // ESP32's free GPIO pin (or 33)


// Define the keys (4x3 layout)
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// ESP32 pin definitions (adjust according to your wiring)
byte rowPins[ROWS] = {4, 14, 27, 5};  // GPIO pins for rows
byte colPins[COLS] = {18, 19, 23};    // GPIO pins for columns

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String VAULT_PIN = "1234";  // Set password
String inputPassword = ""; // Store entered password


String botToken = "97987999764376474";  // From BotFather telgram
// String chatID = "5176254934";  // From @userinfobot
String message = "Intruder Alert! ";
String chatID = "314212636236";  // From @userinfobot telegram

boolean pin_is_correct = false;

boolean relayactive= false;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  // UART2: RX=16, TX=17

  pinMode(BUZZER_PIN, OUTPUT);

  ledcDetachPin(signalPin);  // Stops PWM on GPIO 26

  pinMode(signalPin, OUTPUT);  
  pinMode(signalFromArduino, INPUT_PULLUP);  // Use internal pull-up
  digitalWrite(signalPin, HIGH);  // Default state (inactive)

  ledcSetup(1, 1000, 8); // Channel 1, 1000Hz, 8-bit resolution
  ledcAttachPin(BUZZER_PIN, 1);
  delay(1000);
  myServo.attach(SERVO_PIN);
  if(!myServo.attach(SERVO_PIN)){
    Serial.print("servo failed");
  }
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  pinMode(PUSH_BUTTON_TO_ENTER_PINENTRY, INPUT_PULLUP);  // Enable internal pull-up
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  
  // Initialize LCD
  Wire.begin();  // Initialize I2C for ESP32
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  updateLCD("BANK VAULT ROOM", "Scan Face/Enter Pin");
  Serial.print("initiated"); 
}

void tone(int pin, int freq, int duration) {
  ledcSetup(2, freq, 8);          // Channel 0, frequency, 8-bit resolution
  ledcAttachPin(pin,2);          // Attach buzzer to channel 1
  ledcWriteTone(2, freq);         // Play tone
  delay(duration);                // Wait
  ledcWrite(2, 0);               // Stop tone
}
void updateLCDwithasterick() {
  lcd.setCursor(2, 1);  // Position after "> "
  
  // Display asterisks instead of numbers
  for (int i = 0; i < inputPassword.length(); i++) {
    lcd.print('*');
  }
  
  // Clear remaining characters if password was deleted
  for (int i = inputPassword.length(); i < 16 - 2; i++) {
    lcd.print(' ');
  }
}
void playSecurityAlarm() {
  for (int i = 0; i < 20; i++) {
    // Alternate between high and low frequencies
   tone(BUZZER_PIN, (i % 2 == 0) ? 3000 : 2000, 80);
    delay(80);  // Sync with tone duration
  }
  
}

void slowMove(int startPos, int endPos, int stepDelay) {
  startPos = constrain(startPos + OFFSET, 0, 180);
  endPos = constrain(endPos + OFFSET, 0, 180);
  
  if (startPos < endPos) {
    for (int pos = startPos; pos <= endPos; pos++) {
      myServo.write(pos);
      delay(stepDelay);
    }
  } else {
    for (int pos = startPos; pos >= endPos; pos--) {
      myServo.write(pos);
      delay(stepDelay);
    }
  }
}

void sendTelegramMessage() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;
    
    http.begin(url);
    int httpResponseCode = http.GET();
    
    // if (httpResponseCode > 0) {
    //   Serial.println("Message sent successfully!");
    // } else {
    //   Serial.println("Error sending message");
    // }
    
    http.end();
  }
}

void KeypadEntry() {
  unsigned long lastKeyPressTime = millis();  // Tracks last keypress time
  const unsigned long timeoutPeriod = 5000;  // 10-second timeout (adjust as needed)
  while(millis() - lastKeyPressTime < timeoutPeriod && pinattempts > 0){
  char key = keypad.getKey();
  updateLCD("Enter PIN", ">");

  if (key) {
    lastKeyPressTime = millis();  
    Serial.print("Key Pressed: ");
    Serial.println(key);

    // Short beep for each key press
    tone(BUZZER_PIN, 1000, 100);
    
    if (key == '*') {  
      inputPassword = "";  // Reset input
      Serial.println("Password reset.");
      
      updateLCD("Password","reset");
      updateLCD("Enter PIN", ">");

      // Distinct reset sound: Two short beeps
      tone(BUZZER_PIN, 800, 100);
      delay(150);
      tone(BUZZER_PIN, 800, 100);
    } 
    else if (key == '#') {  
      if (inputPassword == VAULT_PIN) {
        Serial.println("Access Granted!");
        pinattempts=2;
        updateLCD("Access Granted", "");
        tone(BUZZER_PIN, 2000, 300); // Success tone
        delay(1000);
        
        pin_is_correct=true;

        return; //correct pin
        
      } else {
        Serial.println("Access Denied!");
        pinattempts--;
        updateLCD("Access Denied!", "");
        lcd.setCursor(0, 1);
        lcd.print(pinattempts);
        lcd.setCursor(2, 1);
        lcd.print("Attempts Left!");

        tone(BUZZER_PIN, 500, 600);  // Error tone
        delay(1000);
        
        
        
      }
      inputPassword = ""; // Reset after checking
    } 
    else {
      if (inputPassword.length() < 4) {  // Limit password length
        inputPassword += key;
        updateLCDwithasterick();
      } else {
        tone(BUZZER_PIN, 300, 300);  // Error sound for max length
      }
    }
  }
  }

}

void mechanism(){
  digitalWrite(signalPin, HIGH);

  // Serial.println("1");
  // delay(3000);

  updateLCD("Vault Opening!", "Stand Clear!");

  // Serial.println("relay gonna go on");
  // delay(3000);
  digitalWrite(RELAY, LOW); //active low
  // Serial.println("relayon");
  // delay(3000);
  relayactive=true;
  delay(1000);
  Serial.println("Moving to 90°...");
  
  myServo.write(30);
  slowMove(30, 90,100);  // Move slowly from 30° to 90°
  Serial.println(" action 1.");
  delay(5000);
  

  Serial.print("Moving to 30°...");
  slowMove(90, 30, 100);  // Move slowly from 90° to 30°
  Serial.println(" action close");
  delay(2000);
  

  digitalWrite(RELAY, HIGH);
  relayactive=false;
  inputPassword = "";
  updateLCD("BANK VAULT ROOM", "Scan Face/Enter Pin");

}

void PinentryMode(){
    if (digitalRead(PUSH_BUTTON_TO_ENTER_PINENTRY) == LOW && relayactive==false){

  KeypadEntry();
  if(pin_is_correct){
    // updateLCD("Vault Opening!", "Stand Clear!");
    mechanism();
    pin_is_correct=false;
    return;
  }
  if(pinattempts==0){
    return;
  }
  updateLCD("Exiting PinEntry mode", "");
  delay(2000);
  updateLCD("BANK VAULT ROOM!", "Face/Enter Pin");

  
}
}

void loop() {

  

  if (pinattempts==0){
    updateLCD("SECURITY BREACH!", "LOCKDOWN MODE ON!");

    digitalWrite(signalPin, HIGH);  // Start HIGH
    delay(100);                     // Brief delay
    digitalWrite(signalPin, LOW);   // Falling edge
    delay(500);                     // Hold LOW for 500ms
    digitalWrite(signalPin, HIGH);  // Return to HIGH
    

    sendTelegramMessage();
    
    

    while (!digitalRead(signalFromArduino) == LOW) {
    playSecurityAlarm();
    }

    
    updateLCD("RESUMING NORMAL", "OPERATION");
    delay(1000);
    updateLCD("BANK VAULT ROOM", "Scan Face/Enter Pin");

    ledcWrite(1, 0);  
    pinattempts=2;
  }

  if (Serial2.available()) {
    String receivedData = Serial2.readStringUntil('\n');  // Read until newline
    Serial.println("Received: " + receivedData);
    receivedData.trim();
    if (receivedData.equals("Face Detected")) {
      updateLCD("Face Detected!", "WELCOME ^_^");
      delay(1000);
      mechanism();
    }
    else if (receivedData.equals("No Match Found")) {     
      updateLCD("No Match Found", "Who Are You?");
      delay(2000);
    
    }
  }

  PinentryMode();



}
