#include <Arduino.h>
#include <ESP32Servo.h>

// 创建舵机对象
Servo myservo;

// 定义舵机控制引脚
const int servoPin = 2;  // 使用GPIO2作为舵机控制引脚

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  Serial.println("ESP32-S3-Nano 舵机测试程序");

  // 允许分配所有可用的PWM通道
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // 将舵机连接到指定引脚
  myservo.setPeriodHertz(50);  // 标准50Hz PWM
  myservo.attach(servoPin);    // 连接到舵机
  myservo.write(90); // 上电直接转到90度
}

void loop() {
  // 从0度到180度
  for(int pos = 0; pos <= 180; pos += 1) {
    myservo.write(pos);
    Serial.print("舵机角度: ");
    Serial.println(pos);
    delay(15);  // 等待15ms让舵机到达指定位置
  }
  
  // 从180度到0度
  for(int pos = 180; pos >= 0; pos -= 1) {
    myservo.write(pos);
    Serial.print("舵机角度: ");
    Serial.println(pos);
    delay(15);
  }
} 