#ifndef GROUND_H
#define GROUND_H

#include <Arduino.h>
#include "Definitions.cpp"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// 状态结构体定义
typedef struct {
    float site_now[4][3];     //real-time coordinates of the end of each leg
    float site_expect[4][3];  //expected coordinates of the end of each leg
    float temp_speed[4][3];   //each axis' speed, needs to be recalculated before each movement
    int32_t FRFoot = 0;
    int32_t FRElbow = 0;
    int32_t FRShdr = 0;
    int32_t FLFoot = 0;
    int32_t FLElbow = 0;
    int32_t FLShdr = 0;
    int32_t RRFoot = 0;
    int32_t RRElbow = 0;
    int32_t RRShdr = 0;
    int32_t RLFoot = 0;
    int32_t RLElbow = 0;
    int32_t RLShdr = 0;
    int32_t rest_counter = 0;  //+1/0.02s, for automatic rest
} service_status_t;

// 外部变量声明
extern Adafruit_PWMServoDriver pwm;
extern WebServer server;
extern WebSocketsServer webSocket;
extern service_status_t sst;
extern bool print_reach;
extern bool climbing_mode;
extern float move_speed;
extern float speed_multiple;

// 基础函数声明
void wait_all_reach(void);
void wait_reach(int leg);
void set_site(int leg, float x, float y, float z);
void setServoPulse(uint8_t n, double pulse);
bool checkStability(int moving_leg = -1);
void customSerial(const char* message);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleStatus(void);
void action_cmd(void);

// 运动控制函数声明
void servos_init(void);
void servos_loop(void);
void servos_cmd(int action_mode, int n_step);
void servos_start(void);
void servos_service(void * data);
void handleRoot(void);
void handleCommand(void);

// 动作函数声明
void sit(void);
void stand(void);
void step_forward(unsigned int step);
void step_back(unsigned int step);
void turn_left(unsigned int step);
void turn_right(unsigned int step);
void hand_shake(int i);
void hand_wave(int i);
void body_dance(int i);
void b_init(void);
void body_left(int i);
void body_right(int i);
void head_up(int i);
void head_down(int i);

// 气泵控制函数声明
void controlPump(int legIndex, bool state);
void enterClimbingMode(void);
void exitClimbingMode(void);

// 新增：声明让所有舵机保持在90度的函数
void move_all_servos_to_90();
int angleToPWM(int angle);

// 新增：三足吸附爬墙步态函数声明
void crawl_gait_climb(int step);

// 新增：墙面四方向步态函数声明
void crawl_gait_climb_forward(int step);
void crawl_gait_climb_back(int step);
void crawl_gait_climb_left(int step);
void crawl_gait_climb_right(int step);

// 模式管理相关声明
enum RobotMode { GROUND, CLIMB };
void setMode(RobotMode mode);
RobotMode getMode();

// 地面步态（对角步态）声明
void trot_gait_ground(int step);

#endif // GROUND_H 