#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WebServer.h>

// 1. 基础引脚定义
#define SDA_PIN 21  // 例如使用默认的GPIO21
#define SCL_PIN 22  // 例如使用默认的GPIO22
#define LED_BUILTIN 2
#define LED_SPEED 30

// 2. 命令定义
#define W_STAND_SIT    0
#define W_FORWARD      1
#define W_BACKWARD     2
#define W_LEFT         3
#define W_RIGHT        4
#define W_SHAKE        5
#define W_WAVE         6
#define W_DANCE        7
#define W_HEAD_UP      8
#define W_HEAD_DOWN    9
#define W_B_RIGHT      10
#define W_B_LEFT       11
#define W_B_INIT       12
#define W_HIGHER       13
#define W_LOWER        14
#define W_SET          15
#define W_TW_R         16
#define W_TW_L         17

// 3. 运动相关常量
const float KEEP = 255;
const float pi = 3.1415926;

// 4. 速度相关常量
const float spot_turn_speed = 4;
const float leg_move_speed = 8;
const float body_move_speed = 3;
const float stand_seat_speed = 1;

// 5. 气泵控制相关常量
const int PUMP_PINS[] = {2, 3, 4, 5};
const int NUM_PUMPS = 4;
const int STABLE_TIME = 200;
const float CLIMB_SPEED_RATIO = 0.4;

// 6. 机器人尺寸相关常量
const float length_a = 55;
const float length_b = 73.1;   //83.6-10.5
const float length_c = 27.5;
const float length_side = 71;

// 7. 位置相关常量
const float z_absolute = -28;
const float z_default = -50;
const float z_up = -30;
const float z_boot = z_absolute;
const float x_default = 62;
const float x_offset = 0;
const float y_start = 0;
const float y_step = 40;
const float x_step = 40;  // 添加x方向步长
const float y_default = x_default;

// 8. 舵机引脚定义
// 定义每个关节的引脚编号
#define FR_SHOULDER 0
#define FR_ELBOW    1
#define FR_FOOT     2

#define FL_SHOULDER 4
#define FL_ELBOW    5
#define FL_FOOT     6

#define RR_SHOULDER 8
#define RR_ELBOW    9
#define RR_FOOT     10

#define RL_SHOULDER 12
#define RL_ELBOW    13
#define RL_FOOT     14

// 舵机引脚映射表
const int servo_pin[4][3] = {
    {FR_SHOULDER, FR_ELBOW, FR_FOOT},  // Leg 0: 右前腿 FR
    {FL_SHOULDER, FL_ELBOW, FL_FOOT},  // Leg 1: 左前腿 FL
    {RR_SHOULDER, RR_ELBOW, RR_FOOT},  // Leg 2: 右后腿 RR
    {RL_SHOULDER, RL_ELBOW, RL_FOOT}   // Leg 3: 左后腿 RL
};

// 9. WiFi配置
// 请根据你的网络环境修改以下配置
// #define WIFI_SSID "T3-7E-EXT"
// #define WIFI_PASSWORD "Must@28881122"
#define WIFI_SSID "jetracer"
#define WIFI_PASSWORD "12345678"
#define WEB_PORT 80                    // Web服务器端口号
#define WEBSOCKET_PORT 81              // WebSocket服务器端口号

// 10. 调试相关常量
#define TIMER_INTERRUPT_DEBUG 0

// 创建WebServer实例
extern WebServer server;

#endif // DEFINITIONS_H
