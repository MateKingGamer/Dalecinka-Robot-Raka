#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// --- MG90S PCA9685 Settings ---
#define SERVOMIN  105 
#define SERVOMAX  495 
#define SERVO_FREQ 50 

// --- Hybrid Pin Setup ---
Servo base;
const int BASE_PIN = 14; // Base stays on ESP32

// The rest go on PCA9685
const int ARM1_CH = 1;
const int ARM2_CH = 2;
const int WRIST_CH = 3;
const int GRABBER_CH = 4;

int baseValue=90, arm1Value=45, arm2Value=45, wristValue=90, grabberValue=175;

// PCA9685 Helper
void setServoAngle(uint8_t channel, int angle) {
  if(angle < 0) angle = 0;
  if(angle > 175) angle = 175; 
  int pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(channel, 0, pulse);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }
  esp_now_register_recv_cb(onReceive);

  // Init Base on ESP32 (with safe MG90S limits)
  base.attach(BASE_PIN, 500, 2400);

  // Init Rest on PCA9685
  pwm.begin();
  pwm.setOscillatorFrequency(27000000); 
  pwm.setPWMFreq(SERVO_FREQ);
  delay(150);

  defaultServo();
}

void loop() {
  // Empty - Arm is completely driven by ESP-NOW interrupts
}

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len > 0) { 
    int command = data[0];
    Serial.println(command);
    
    // Base commands use the standard ESP32 .write()
    if(command==4){
      baseValue-=1;
      if(baseValue<0)baseValue=0;
      base.write(baseValue);
    }else if(command==3){
      baseValue+=1;
      if(baseValue>175) baseValue=175;
      base.write(baseValue);
      
    // The rest use the PCA9685 helper
    }else if(command==1){
      arm1Value-=1;
      if(arm1Value<0)arm1Value=0;
      setServoAngle(ARM1_CH, arm1Value);
    }else if(command==2){
      arm1Value+=1;
      if(arm1Value>175) arm1Value=175;
      setServoAngle(ARM1_CH, arm1Value);
    }else if(command==7){
      arm2Value-=1;
      if(arm2Value<0)arm2Value=0;
      setServoAngle(ARM2_CH, arm2Value);
    }else if(command==6){
      arm2Value+=1;
      if(arm2Value>175) arm2Value=175;
      setServoAngle(ARM2_CH, arm2Value);
    }else if(command==9){
      wristValue-=1;
      if(wristValue<0)wristValue=0;
      setServoAngle(WRIST_CH, wristValue);
    }else if(command==8){
      wristValue+=1;
      if(wristValue>175) wristValue=175;
      setServoAngle(WRIST_CH, wristValue);
    }else if(command==30){
      setServoAngle(GRABBER_CH, 175);
    }else if(command==31){
      setServoAngle(GRABBER_CH, 0);
    }
  }
}

void defaultServo(){
  baseValue=90; arm1Value=45; arm2Value=45; wristValue=90; grabberValue=175;
  
  base.write(baseValue);
  delay(500);
  setServoAngle(ARM1_CH, arm1Value);
  delay(500);
  setServoAngle(ARM2_CH, arm2Value);
  delay(500);
  setServoAngle(WRIST_CH, wristValue);
  delay(500);
  setServoAngle(GRABBER_CH, grabberValue);
  delay(500);
}