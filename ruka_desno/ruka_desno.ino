#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_MPU6050 mpu;
uint8_t ruka[] = {0x00, 0x70, 0x07, 0x8A, 0x08, 0xA4};
int command;

int buttonPin1=3, buttonPin2=1;
long long startTime=0, startTime2=0,checkAngleTime=0;

Adafruit_SSD1306 display(128, 32, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // SDA,SCL

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (1);
  }

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, ruka, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.begin(115200);
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don’t continue, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);             // Big text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(32, 0);
  display.setRotation(2);
  display.println(F("Hand:"));
  display.setCursor(30, 16);
  display.println(F("Open"));
  display.display();  

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  Serial.println("Hand ESP Ready");
}

void loop() {
  checkButtons();
  checkingAngle();
  
}

void checkButtons(){
  int button1=digitalRead(buttonPin1);
  int button2=digitalRead(buttonPin2);
  Serial.println();
  Serial.println(button1);
  Serial.println(button2);
  Serial.println();
  if(button2==LOW && millis()-startTime>400){
    display.clearDisplay();
    display.setCursor(32, 0);
    display.println(F("Hand:"));
    startTime=millis();
    display.setCursor(25, 16);
    display.println(F("Closed"));
    display.display();
    int komanda=31;
    esp_now_send(ruka, (uint8_t*)&komanda, sizeof(komanda));
  }
  if(button1==LOW && millis()-startTime2>400){
    display.clearDisplay();
    display.setCursor(32, 0);
    display.println(F("Hand:"));
    startTime2=millis();
    display.setCursor(25, 16);
    display.println(F("Open"));
    display.display();
    int komanda=30;
    esp_now_send(ruka, (uint8_t*)&komanda, sizeof(komanda));
  }
}

void checkingAngle(){
  if(millis()-checkAngleTime>25){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    if (a.acceleration.y > 6.0) command = 9;  // Backward
    else if (a.acceleration.y < -6.0) command = 8;  // Forward
    else if (a.acceleration.x < -6.0) command = 7;  // Right
    else if (a.acceleration.x > 6.0)  command = 6;  // Left
    else command = 0;  // Neutral
    esp_now_send(ruka, (uint8_t*)&command, sizeof(command));
    checkAngleTime=millis();
  }
}