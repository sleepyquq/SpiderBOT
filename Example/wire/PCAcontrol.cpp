#include <Wire.h>  
#include <Adafruit_PWMServoDriver.h>  

// 指定 I2C 引脚为 7 和 8  
#define SDA_PIN 7  
#define SCL_PIN 8  

// 创建 PCA9685 驱动板对象  
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();  

// 舵机参数设置  
#define SERVOMIN  150  // 脉冲宽度对应 0 度位置  
#define SERVOMAX  600  // 脉冲宽度对应 180 度位置  

void setup() {  
  Serial.begin(115200);  
  Serial.println("\n\nPCA9685 舵机控制测试");  
  
  // 初始化 I2C，使用引脚 7 (SDA) 和 8 (SCL)  
  Wire.begin(SDA_PIN, SCL_PIN);  
  
  // 扫描 I2C 设备  
  scanI2CDevices();  
  
  // 初始化 PCA9685  
  Serial.println("尝试初始化 PCA9685...");  
  if (!pwm.begin()) {  
    Serial.println("PCA9685 初始化失败。请检查接线。");  
    while (1);  
  }  
  Serial.println("PCA9685 初始化成功");  
  
  // 设置 PWM 频率为 50Hz (适合大多数舵机)  
  pwm.setPWMFreq(50);  
  Serial.println("PWM 频率设置为 50Hz");  
  
  delay(100);  
}  

void loop() {  
  Serial.println("测试通道 0 的舵机...");  
  
  Serial.println("将舵机移动到 90 度");  
  int pulseWidth = map(90, 0, 180, SERVOMIN, SERVOMAX);  
  pwm.setPWM(0, 0, pulseWidth);  
  Serial.print("脉冲宽度: ");  
  Serial.println(pulseWidth);  
  delay(2000);  
  
  Serial.println("将舵机移动到 0 度");  
  pulseWidth = map(0, 0, 180, SERVOMIN, SERVOMAX);  
  pwm.setPWM(0, 0, pulseWidth);  
  Serial.print("脉冲宽度: ");  
  Serial.println(pulseWidth);  
  delay(2000);  
  
  Serial.println("将舵机移动到 180 度");  
  pulseWidth = map(180, 0, 180, SERVOMIN, SERVOMAX);  
  pwm.setPWM(0, 0, pulseWidth);  
  Serial.print("脉冲宽度: ");  
  Serial.println(pulseWidth);  
  delay(2000);  
  
  // 测试其他通道  
  Serial.println("测试通道 1 的舵机...");  
  pwm.setPWM(1, 0, map(90, 0, 180, SERVOMIN, SERVOMAX));  
  delay(2000);  
}  

void scanI2CDevices() {  
  byte error, address;  
  int nDevices = 0;  
  
  Serial.println("扫描 I2C 设备...");  
  
  for(address = 1; address < 127; address++) {  
    Wire.beginTransmission(address);  
    error = Wire.endTransmission();  
    
    if (error == 0) {  
      Serial.print("发现 I2C 设备，地址: 0x");  
      if (address < 16) {  
        Serial.print("0");  
      }  
      Serial.println(address, HEX);  
      nDevices++;  
    }  
  }  
  
  if (nDevices == 0) {  
    Serial.println("未找到 I2C 设备");  
  } else {  
    Serial.println("扫描完成");  
  }  
}  