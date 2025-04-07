#include "esp_camera.h"
#include <WiFi.h>
// #define FACEREGNZED 13  // GPIO13 as input
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"



// const char* ssid = "vivo Y33s";
// const char* password = "12345679";

const char* ssid = "LEKSHITH";
const char* password = "12345679";



void startCameraServer();

boolean matchFace = false;
long prevMillis = 0;
int interval = 5000;

void setup() {
  Serial.begin(115200);  //uart to esp32
  

  
  // pinMode(FACEREGNZED, OUTPUT); 
  // digitalWrite(FACEREGNZED, HIGH);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  // config.pixel_format = PIXFORMAT_RGB565;


  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // =(".");
  }
  // Serial.println("\nWiFi connected");

  startCameraServer();
  // Serial.print("Camera Ready! Use 'http://");
  // Serial.print(WiFi.localIP());
  // Serial.println("' to connect");
}

void loop() {
  if (matchFace) {
    // digitalWrite(FACEREGNZED, LOW);
    Serial.println("Face Detected");  // Send message to other ESP32 via UART
    delay(15000);
    matchFace = false;
    // prevMillis = millis();
  }

  // if (millis() - prevMillis > interval) {
  //   matchFace = false;
  //   // digitalWrite(FACEREGNZED, HIGH);
  // }
}
