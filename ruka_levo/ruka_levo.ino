#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>

Adafruit_MPU6050 mpu;
uint8_t ruka[] = {0x00, 0x70, 0x07, 0x8A, 0x08, 0xA4};
int command;
long long checkAngleTime=0;

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

  Serial.println("Hand ESP Ready");
}

void loop() {
  checkingAngle();
  
}

void checkingAngle(){
  if(millis()-checkAngleTime>25){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    if (a.acceleration.y > 6.0) command = 1;  
    else if (a.acceleration.y < -6.0) command = 2;  
    else if (a.acceleration.x < -6.0) command = 3;  
    else if (a.acceleration.x > 6.0)  command = 4;  
    else command = 0;  // Neutral
    esp_now_send(ruka, (uint8_t*)&command, sizeof(command));
    checkAngleTime=millis();
  }
}