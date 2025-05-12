#include "ground.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include "index.h"  // 添加新的头文件引用

// 全局变量定义
bool print_reach = false;
bool climbing_mode = false;
float move_speed = 1.4;
float speed_multiple = 1;

// 创建Web服务器实例
WebServer server(80);  // 添加Web服务器实例定义

// 创建PWM控制器实例
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// 创建WebSocket实例
WebSocketsServer webSocket = WebSocketsServer(81);

// 创建服务状态实例
service_status_t sst;  // 添加服务状态实例定义

// Web页面HTML
// static const char PROGMEM INDEX_HTML[] = R"rawliteral(
// ... 原有的HTML内容 ...
// )rawliteral";

/* Servos --------------------------------------------------------------------*/
//define 12 servos for 4 legs
//Servo servo[4][3];
//define servos' ports
// const int servo_pin[4][3] = {{0, 1, 2}, {4, 5, 6}, {8, 9, 10}, {12, 13, 14}};
/* Size of the robot ---------------------------------------------------------*/
// const float length_a = 55;
// const float length_b = 77.5;
// const float length_c = 27.5;
// const float length_side = 71;
// const float z_absolute = -28;
/* Constants for movement ----------------------------------------------------*/
// const float z_default = -50, z_up = -30, z_boot = z_absolute;
// const float x_default = 62, x_offset = 0;
// const float y_start = 0, y_step = 40;
// const float y_default = x_default;
// const float spot_turn_speed = 4;
// const float leg_move_speed = 8;
// const float body_move_speed = 3;
// const float stand_seat_speed = 1;
//functions' parameter
// const float KEEP = 255;
//define PI for calculation
// const float pi = 3.1415926;
/* Constants for turn --------------------------------------------------------*/
// 转向相关常量定义
const float temp_a = sqrt(pow(2 * x_default + length_side, 2) + pow(y_step, 2));
const float temp_b = 2 * (y_start + y_step) + length_side;
const float temp_c = sqrt(pow(2 * x_default + length_side, 2) + pow(2 * y_start + y_step + length_side, 2));
const float temp_alpha = acos((pow(temp_a, 2) + pow(temp_b, 2) - pow(temp_c, 2)) / 2 / temp_a / temp_b);

// 转向位置计算
const float turn_x1 = (temp_a - length_side) / 2;
const float turn_y1 = y_start + y_step / 2;
const float turn_x0 = turn_x1 - temp_b * cos(temp_alpha);
const float turn_y0 = temp_b * sin(temp_alpha) - turn_y1 - length_side;
/* ---------------------------------------------------------------------------*/
String lastComm = "";

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
    double pulselength;
    pulselength = 1000000;  // 1,000,000 微秒/秒
    pulselength /= 60;      // 将频率设为60Hz
    pulselength /= 4096;    // 12位分辨率,将信号分为4096份
    pulse *= 1000000;       // 将输入的秒转换为微秒
    pulse /= pulselength;   // 计算PWM值
    pwm.setPWM(n, 0, pulse);// 设置第n个舵机的PWM信号
}

bool checkStability(int moving_leg) {
    // 1. 获取支撑腿数量和位置
    float support_points[3][3];  // 最多3个支撑点的坐标
    int support_count = 0;
    
    // 收集支撑点信息
    for(int i = 0; i < 4; i++) {
        if(i != moving_leg && sst.site_now[i][2] == z_default) {
            if(support_count < 3) {
                support_points[support_count][0] = sst.site_now[i][0];
                support_points[support_count][1] = sst.site_now[i][1];
                support_points[support_count][2] = sst.site_now[i][2];
                support_count++;
            }
        }
    }
    
    // 2. 基本稳定性检查：至少需要3个支撑点
    if(support_count < 3) {
        if(print_reach) Serial.println("警告：支撑点不足3个");
        return false;
    }
    
    // 3. 计算支撑三角形的重心
    float center_x = 0, center_y = 0;
    for(int i = 0; i < 3; i++) {
        center_x += support_points[i][0];
        center_y += support_points[i][1];
    }
    center_x /= 3;
    center_y /= 3;
    
    // 4. 估算机器人重心位置（假设在几何中心）
    float cog_x = 0, cog_y = 0;
    int total_legs = 0;
    for(int i = 0; i < 4; i++) {
        if(i != moving_leg) {
            cog_x += sst.site_now[i][0];
            cog_y += sst.site_now[i][1];
            total_legs++;
        }
    }
    cog_x /= total_legs;
    cog_y /= total_legs;
    
    // 5. 计算重心到支撑三角形各边的距离
    float margin = 15.0;  // 安全裕度（单位：mm）
    
    // 使用向量叉积判断点是否在三角形内部
    float area1 = (support_points[1][0] - support_points[0][0]) * (cog_y - support_points[0][1]) - 
                 (support_points[1][1] - support_points[0][1]) * (cog_x - support_points[0][0]);
    float area2 = (support_points[2][0] - support_points[1][0]) * (cog_y - support_points[1][1]) - 
                 (support_points[2][1] - support_points[1][1]) * (cog_x - support_points[1][0]);
    float area3 = (support_points[0][0] - support_points[2][0]) * (cog_y - support_points[2][1]) - 
                 (support_points[0][1] - support_points[2][1]) * (cog_x - support_points[2][0]);
    
    bool is_inside = (area1 >= -margin && area2 >= -margin && area3 >= -margin) ||
                    (area1 <= margin && area2 <= margin && area3 <= margin);
    
    if(!is_inside && print_reach) {
        Serial.println("警告：重心不在支撑三角形内");
        Serial.printf("重心位置: (%.2f, %.2f)\n", cog_x, cog_y);
    }
    
    // 6. 爬墙模式额外检查
    if(climbing_mode) {
        // 检查支撑点的分布是否合理（避免三点共线）
        float min_area = 100.0;  // 最小三角形面积阈值
        float triangle_area = abs(area1 + area2 + area3) / 2;
        
        if(triangle_area < min_area) {
            if(print_reach) Serial.println("警告：支撑三角形面积过小");
            return false;
        }
    }
    
    return is_inside;
}

void set_site(int leg, float x, float y, float z) {
    // 爬墙模式安全检查
    if(climbing_mode && z != KEEP && z != sst.site_now[leg][2]) {
        if(!checkStability(leg)) {
            Serial.println("警告：移动不安全，取消操作");
            return;
        }
        // 如果是抬腿动作
        if(z > sst.site_now[leg][2]) {
            controlPump(leg, false);  // 关闭气泵
            delay(STABLE_TIME);       // 等待气压释放
        }
    }
    // 原有的位置计算代码
    float length_x = 0, length_y = 0, length_z = 0;
    if (x != KEEP)
        length_x = x - sst.site_now[leg][0];
    if (y != KEEP)
        length_y = y - sst.site_now[leg][1];
    if (z != KEEP)
        length_z = z - sst.site_now[leg][2];
    float length = sqrt(pow(length_x, 2) + pow(length_y, 2) + pow(length_z, 2));
    float speed_factor = climbing_mode ? CLIMB_SPEED_RATIO : 1.0;
    sst.temp_speed[leg][0] = length_x / length * move_speed * speed_multiple * speed_factor;
    sst.temp_speed[leg][1] = length_y / length * move_speed * speed_multiple * speed_factor;
    sst.temp_speed[leg][2] = length_z / length * move_speed * speed_multiple * speed_factor;
    if (x != KEEP)
        sst.site_expect[leg][0] = x;
    if (y != KEEP)
        sst.site_expect[leg][1] = y;
    if (z != KEEP)
        sst.site_expect[leg][2] = z;
    // 爬墙模式下的着地后处理
    if(climbing_mode && z != KEEP && z < sst.site_now[leg][2]) {
        wait_reach(leg);         // 等待到达位置
        controlPump(leg, true);  // 开启气泵
        delay(STABLE_TIME);      // 等待吸附稳定
        // 吸附测试
        float test_height = 5.0;
        float original_z = z;
        set_site(leg, KEEP, KEEP, z + test_height);
        wait_reach(leg);
        delay(50);
        set_site(leg, KEEP, KEEP, original_z);
        wait_reach(leg);
    }
}

/*
  - is_stand
   ---------------------------------------------------------------------------*/
bool is_stand(void) {
    if (sst.site_now[0][2] == z_default)
        return true;
    else
        return false;
}//判断有无站立

/*
  - sit
  - blocking function
   ---------------------------------------------------------------------------*/
void sit(void) {
    move_speed = stand_seat_speed;
    Serial.printf("[SERVO] Set sit speed: %d\n", move_speed);  // 添加调试信息
    
    for (int leg = 0; leg < 4; leg++) {
        set_site(leg, KEEP, KEEP, z_boot);  // 保持X/Y坐标，仅降低Z轴到坐下高度
    }
    
    wait_all_reach();  // 阻塞等待所有舵机到达目标位置
    Serial.println("[SERVO] Sit down completed");  // 修改为更准确的完成提示
}

/*
  - stand
  - blocking function
   ---------------------------------------------------------------------------*/
void stand(void) {
    move_speed = stand_seat_speed;
    for (int leg = 0; leg < 4; leg++) {
        set_site(leg, KEEP, KEEP, z_default);
    }
    wait_all_reach();
    
}

/*
  - Body init
  - blocking function
  //初始化机器人

   ---------------------------------------------------------------------------*/
void b_init(void) {
    //stand();
    set_site(0, x_default, y_default, z_default);
    set_site(1, x_default, y_default, z_default);
    set_site(2, x_default, y_default, z_default);
    set_site(3, x_default, y_default, z_default);
    wait_all_reach();
    Serial.println("Body twist right");
}

/*
  - spot turn to left
  - blocking function
  - parameter step steps wanted to turn
   ---------------------------------------------------------------------------*/
void turn_left(unsigned int step) {
    move_speed = spot_turn_speed;
    while (step-- > 0) {
        if (sst.site_now[3][1] == y_start) {
            //leg 3&1 move
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x1 - x_offset, turn_y1, z_default);
            set_site(1, turn_x0 - x_offset, turn_y0, z_default);
            set_site(2, turn_x1 + x_offset, turn_y1, z_default);
            set_site(3, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(3, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x1 + x_offset, turn_y1, z_default);
            set_site(1, turn_x0 + x_offset, turn_y0, z_default);
            set_site(2, turn_x1 - x_offset, turn_y1, z_default);
            set_site(3, turn_x0 - x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(1, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_default);
            set_site(1, x_default + x_offset, y_start, z_up);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            set_site(1, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //leg 0&2 move
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_up);
            set_site(1, turn_x1 + x_offset, turn_y1, z_default);
            set_site(2, turn_x0 - x_offset, turn_y0, z_default);
            set_site(3, turn_x1 - x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x0 - x_offset, turn_y0, z_default);
            set_site(1, turn_x1 - x_offset, turn_y1, z_default);
            set_site(2, turn_x0 + x_offset, turn_y0, z_default);
            set_site(3, turn_x1 + x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(2, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_up);
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();

            set_site(2, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - spot turn to right
  - blocking function
  - parameter step steps wanted to turn
   ---------------------------------------------------------------------------*/
void turn_right(unsigned int step) {
    move_speed = spot_turn_speed;
    while (step-- > 0) {
        if (sst.site_now[2][1] == y_start) {
            //leg 2&0 move
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x0 - x_offset, turn_y0, z_default);
            set_site(1, turn_x1 - x_offset, turn_y1, z_default);
            set_site(2, turn_x0 + x_offset, turn_y0, z_up);
            set_site(3, turn_x1 + x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(2, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_default);
            set_site(1, turn_x1 + x_offset, turn_y1, z_default);
            set_site(2, turn_x0 - x_offset, turn_y0, z_default);
            set_site(3, turn_x1 - x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_up);
            set_site(1, x_default + x_offset, y_start, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //leg 1&3 move
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x1 + x_offset, turn_y1, z_default);
            set_site(1, turn_x0 + x_offset, turn_y0, z_up);
            set_site(2, turn_x1 - x_offset, turn_y1, z_default);
            set_site(3, turn_x0 - x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(1, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x1 - x_offset, turn_y1, z_default);
            set_site(1, turn_x0 - x_offset, turn_y0, z_default);
            set_site(2, turn_x1 + x_offset, turn_y1, z_default);
            set_site(3, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(3, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_default);
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - go forward
  - blocking function
  - parameter step steps wanted to go
   ---------------------------------------------------------------------------*/
void step_forward(unsigned int step) {
    move_speed = leg_move_speed;
    while (step-- > 0) {
        if (sst.site_now[2][1] == y_start) {
            //leg 2&1 move
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default + x_offset, y_start, z_default);
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //      leg 0&3 move
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_default);
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - go back
  - blocking function
  - parameter step steps wanted to go
   ---------------------------------------------------------------------------*/
void step_back(unsigned int step) {
    move_speed = leg_move_speed;
    while (step-- > 0) {
        if (sst.site_now[3][1] == y_start) {
            //leg 3&0 move
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(1, x_default + x_offset, y_start, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //leg 1&2 move
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

// add by RegisHsu

void body_left(int i) {
    set_site(0, sst.site_now[0][0] + i, KEEP, KEEP);
    set_site(1, sst.site_now[1][0] + i, KEEP, KEEP);
    set_site(2, sst.site_now[2][0] - i, KEEP, KEEP);
    set_site(3, sst.site_now[3][0] - i, KEEP, KEEP);
    wait_all_reach();
}

void body_right(int i) {
    set_site(0, sst.site_now[0][0] - i, KEEP, KEEP);
    set_site(1, sst.site_now[1][0] - i, KEEP, KEEP);
    set_site(2, sst.site_now[2][0] + i, KEEP, KEEP);
    set_site(3, sst.site_now[3][0] + i, KEEP, KEEP);
    wait_all_reach();
}

void hand_wave(int i) {
    float x_tmp;
    float y_tmp;
    float z_tmp;
    move_speed = 1;
    if (sst.site_now[3][1] == y_start) {
        body_right(15);
        x_tmp = sst.site_now[2][0];
        y_tmp = sst.site_now[2][1];
        z_tmp = sst.site_now[2][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(2, turn_x1, turn_y1, 50);
            wait_all_reach();
            set_site(2, turn_x0, turn_y0, 50);
            wait_all_reach();
        }
        set_site(2, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_left(15);
    } else {
        body_left(15);
        x_tmp = sst.site_now[0][0];
        y_tmp = sst.site_now[0][1];
        z_tmp = sst.site_now[0][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(0, turn_x1, turn_y1, 50);
            wait_all_reach();
            set_site(0, turn_x0, turn_y0, 50);
            wait_all_reach();
        }
        set_site(0, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_right(15);
    }
}

void hand_shake(int i) {
    float x_tmp;
    float y_tmp;
    float z_tmp;
    move_speed = 1;
    if (sst.site_now[3][1] == y_start) {
        body_right(15);
        x_tmp = sst.site_now[2][0];
        y_tmp = sst.site_now[2][1];
        z_tmp = sst.site_now[2][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(2, x_default - 30, y_start + 2 * y_step, 55);
            wait_all_reach();
            set_site(2, x_default - 30, y_start + 2 * y_step, 10);
            wait_all_reach();
        }
        set_site(2, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_left(15);
    } else {
        body_left(15);
        x_tmp = sst.site_now[0][0];
        y_tmp = sst.site_now[0][1];
        z_tmp = sst.site_now[0][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(0, x_default - 30, y_start + 2 * y_step, 55);
            wait_all_reach();
            set_site(0, x_default - 30, y_start + 2 * y_step, 10);
            wait_all_reach();
        }
        set_site(0, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_right(15);
    }
}

void head_up(int i) {
    set_site(0, KEEP, KEEP, sst.site_now[0][2] - i);
    set_site(1, KEEP, KEEP, sst.site_now[1][2] + i);
    set_site(2, KEEP, KEEP, sst.site_now[2][2] - i);
    set_site(3, KEEP, KEEP, sst.site_now[3][2] + i);
    wait_all_reach();
}

void head_down(int i) {
    set_site(0, KEEP, KEEP, sst.site_now[0][2] + i);
    set_site(1, KEEP, KEEP, sst.site_now[1][2] - i);
    set_site(2, KEEP, KEEP, sst.site_now[2][2] + i);
    set_site(3, KEEP, KEEP, sst.site_now[3][2] - i);
    wait_all_reach();
}

void body_dance(int i) {
    float body_dance_speed = 2;
    sit();
    move_speed = 1;
    set_site(0, x_default, y_default, KEEP);
    set_site(1, x_default, y_default, KEEP);
    set_site(2, x_default, y_default, KEEP);
    set_site(3, x_default, y_default, KEEP);
    // print_reach = true;
    wait_all_reach();
    stand();
    set_site(0, x_default, y_default, z_default - 20);
    set_site(1, x_default, y_default, z_default - 20);
    set_site(2, x_default, y_default, z_default - 20);
    set_site(3, x_default, y_default, z_default - 20);
    wait_all_reach();
    move_speed = body_dance_speed;
    head_up(30);
    for (int j = 0; j < i; j++) {
        if (j > i / 4)
            move_speed = body_dance_speed * 2;
        if (j > i / 2)
            move_speed = body_dance_speed * 3;
        set_site(0, KEEP, y_default - 20, KEEP);
        set_site(1, KEEP, y_default + 20, KEEP);
        set_site(2, KEEP, y_default - 20, KEEP);
        set_site(3, KEEP, y_default + 20, KEEP);
        wait_all_reach();
        set_site(0, KEEP, y_default + 20, KEEP);
        set_site(1, KEEP, y_default - 20, KEEP);
        set_site(2, KEEP, y_default + 20, KEEP);
        set_site(3, KEEP, y_default - 20, KEEP);
        wait_all_reach();
    }
    move_speed = body_dance_speed;
    head_down(30);
    b_init();
}

// This gets set as the default handler, and gets called when no other command matches.
// 当没有其他命令匹配时，这个函数会被设置为默认处理程序，并被调用
void unrecognized(const char *command) {
    Serial.println("What?");
}

void action_cmd() {
    if (Serial.available()) {
        char val = Serial.read();
        int n_step = 1;
        servos_cmd(val, n_step);
    }
}

void servos_cmd(int action_mode, int n_step) {
    switch (action_mode) {
        case '0':
            if (!is_stand()) stand();
            step_forward(n_step);
            break;
        case '1':
            if (!is_stand()) stand();
            step_back(n_step);
            break;
        case '2':
            if (!is_stand()) stand();
            turn_left(n_step);
            break;
        case '3':
            if (!is_stand()) stand();
            turn_right(n_step);
            break;
        case '4':
            sit();
            break;
        case '5':
            hand_shake(n_step);
            break;
        case '6':
            hand_wave(n_step);
            break;
        case '7':
            body_dance(10);
            break;
        case '8':
            stand();
            break;
        case 'L':
            body_left(15);
            break;
        case 'R':
            body_right(15);
            break;
        case 'U':
            head_up(15);
            break;
        case 'D':
            head_down(15);
            break;
        case 'S':
            break;
        default:
            customSerial("未知命令");
            break;
    }
}

/*
  - trans site from cartesian to polar
  - mathematical model 2/2
   ---------------------------------------------------------------------------*/
void cartesian_to_polar(float &alpha, float &beta, float &gamma, float x, float y, float z) {
    //calculate w-z degree
    // 计算水平面投影和修正长度
    float v, w;
    w = (x >= 0 ? 1 : -1) * (sqrt(pow(x, 2) + pow(y, 2)));
    v = w - length_c;
    alpha = atan2(z, v) + acos((pow(length_a, 2) - pow(length_b, 2) + pow(v, 2) + pow(z, 2)) / 2 / length_a / sqrt(pow(v, 2) + pow(z, 2)));
    beta = acos((pow(length_a, 2) + pow(length_b, 2) - pow(v, 2) - pow(z, 2)) / 2 / length_a / length_b);
    //calculate x-y-z degree
    // 计算水平旋转角度（绕z轴）    
    gamma = (w >= 0) ? atan2(y, x) : atan2(-y, -x);
    //trans degree pi->180
    // 将弧度转换为角度
    alpha = alpha / pi * 180.0;
    beta = beta / pi * 180.0;
    gamma = gamma / pi * 180.0;
}

// PWM调试输出函数
// 参数说明：
// pin - 舵机控制引脚编号
// off - PWM关闭时间值（0-4095）
void print_final_PWM(int pin, uint16_t off) {
#ifdef TIMER_INTERRUPT_DEBUG  // 仅在调试模式启用串口输出
    customSerial(String("[PWM]\t引脚:" + String(pin) + "\t关闭时间:" + String(off)).c_str());  // 使用中文标签输出调试信息
#endif
}

/*
  - trans site from polar to microservos
  - mathematical model map to fact
  - the errors saved in eeprom will be add
   ---------------------------------------------------------------------------*/
/**
 * @brief 将极坐标角度转换为舵机控制信号
 * 
 * 该函数将计算得到的关节角度转换为实际的舵机控制信号，考虑了机械校准和偏移量。
 * 每个腿部的三个关节（肩、肘、脚）角度需要分别处理，并最终转换为PWM信号。
 * 
 * @param leg 腿部编号（0-3对应右前、右后、左前、左后）
 * @param alpha 肘关节角度（极坐标计算值）
 * @param beta 膝关节角度（极坐标计算值）
 * @param gamma 肩关节角度（极坐标计算值）
 */
void polar_to_servo(int leg, float alpha, float beta, float gamma) {
    // 定义腿部名称数组
    const char* legNames[] = {"FR", "FL", "RR", "RL"};
    
    // 输出原始角度
    if(print_reach) {
        String debugMsg = String("[") + legNames[leg] + String("] 原始角度:\n") +
                         "肩关节(gamma): " + String(gamma) + "°\n" +
                         "肘关节(alpha): " + String(alpha) + "°\n" +
                         "腿关节(beta):  " + String(beta) + "°";
        customSerial(debugMsg.c_str());
    }

    // 根据不同腿部应用基准角度和校准值
    float calibrated_alpha, calibrated_beta, calibrated_gamma;
    switch(leg) {
        case 0: // FR (右前腿)
            alpha = 85 - alpha - sst.FRElbow;   // 基准角度85°
            beta = beta + 40 - sst.FRFoot;      // 基准角度40°
            gamma += 115 - sst.FRShdr;          // 基准角度115°
            break;
        case 1: // FL (左前腿)
            alpha += 75 + sst.FLElbow;          // 基准角度75°
            beta = 140 - beta + sst.FLFoot;     // 基准角度140°
            gamma = 115 - gamma + sst.FLShdr;    // 基准角度115°
            break;
        case 2: // RR (右后腿)
            alpha += 90 + sst.RRElbow;         // 基准角度90°
            beta = 115 - beta + sst.RRFoot;    // 基准角度115°
            gamma = 115 - gamma + sst.RRShdr;  // 基准角度115°
            break;
        case 3: // RL (左后腿)
            alpha = 90 - alpha - sst.RLElbow;  // 基准角度90°
            beta = beta + 50 - sst.RLFoot;     // 基准角度50°
            gamma += 100 - sst.RLShdr;         // 基准角度100°
            break;
    }

    // 输出校准后角度
    if(print_reach) {
        String calibMsg = String("[") + legNames[leg] + String("] 校准后角度:\n") +
                         "肩关节(gamma): " + String(gamma) + "°\n" +
                         "肘关节(alpha): " + String(alpha) + "°\n" +
                         "腿关节(beta):  " + String(beta) + "°";
        customSerial(calibMsg.c_str());
    }

    // 计算PWM值并设置舵机
    int servo_pin[4][3] = {{0, 1, 2}, {4, 5, 6}, {8, 9, 10}, {12, 13, 14}};
    uint16_t pwm_alpha = angleToPWM(constrain(alpha, 0, 180));
    uint16_t pwm_beta = angleToPWM(constrain(beta, 0, 180));
    uint16_t pwm_gamma = angleToPWM(constrain(gamma, 0, 180));

    // 设置PWM信号
    pwm.setPWM(servo_pin[leg][0], 0, pwm_alpha);
    pwm.setPWM(servo_pin[leg][1], 0, pwm_beta);
    pwm.setPWM(servo_pin[leg][2], 0, pwm_gamma);

    // 输出PWM值
    if(print_reach) {
        String pwmMsg = String("[") + legNames[leg] + String("] PWM值:\n") +
                       "肩关节(pin " + String(servo_pin[leg][0]) + "): " + String(pwm_alpha) + "\n" +
                       "肘关节(pin " + String(servo_pin[leg][1]) + "): " + String(pwm_beta) + "\n" +
                       "腿关节(pin " + String(servo_pin[leg][2]) + "): " + String(pwm_gamma);
        customSerial(pwmMsg.c_str());
        customSerial("------------------------");
    }
}

// 信号量句柄声明（用于多线程资源保护）
SemaphoreHandle_t Semaphore;

// 舵机控制服务线程主函数
// 参数说明：
// data - 线程参数指针（此处未直接使用，通过全局变量sst访问状态）
void servos_service(void * data) {
    for (;;) {
        float alpha, beta, gamma;
        xSemaphoreTake(Semaphore, portMAX_DELAY);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                if (abs(sst.site_now[i][j] - sst.site_expect[i][j]) >= abs(sst.temp_speed[i][j]))
                    sst.site_now[i][j] += sst.temp_speed[i][j];
                else
                    sst.site_now[i][j] = sst.site_expect[i][j];
            }
            cartesian_to_polar(alpha, beta, gamma, sst.site_now[i][0], sst.site_now[i][1], sst.site_now[i][2]);
            polar_to_servo(i, alpha, beta, gamma);
        }
        sst.rest_counter++;
        xSemaphoreGive(Semaphore);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// 舵机系统启动初始化（包含安全位置初始化）
void servos_start() {
    sit();      // 首先让机器人进入坐下状态确保安全
    b_init();   // 执行基础姿态初始化
}

TaskHandle_t Task0; // 舵机控制任务句柄

// 将角度转换为PWM值的辅助函数
int angleToPWM(int angle) {
    // 使用标准的舵机PWM范围：500-2500
    return (int)(500 + (angle / 180.0) * (2500 - 500));
}

// 舵机系统初始化函数
void servos_init() {
    // ================== 系统基础设置 ==================
    setCpuFrequencyMhz(80);
    int cpuSpeed = getCpuFrequencyMhz();
    
    // ================== 继电器控制初始化 ==================
    for(int i = 0; i < NUM_PUMPS; i++) {
        pinMode(PUMP_PINS[i], OUTPUT);
        digitalWrite(PUMP_PINS[i], HIGH);  // 继电器高电平为关闭状态
    }
    climbing_mode = false;  // 确保爬墙模式初始为关闭状态
    
    // ================== PWM舵机库初始化 ==================
    Wire.begin(SDA_PIN, SCL_PIN);     // 初始化I2C通信
    pwm.begin();                      // 启动PWM控制器
    pwm.setPWMFreq(60);               // 设置PWM频率为60Hz（模拟舵机标准频率）
    
    delay(1000); // 等待PWM控制器稳定
    
    // 测试代码：将所有舵机移动到90度位置
    customSerial("正在将所有舵机移动到90度位置...");
    for (int leg = 0; leg < 4; leg++) {
        for (int joint = 0; joint < 3; joint++) {
            int servo_pin = leg * 3 + joint;
            int pwm_value = angleToPWM(90);
            pwm.setPWM(servo_pin, 0, pwm_value);
            customSerial(String("设置舵机 " + String(servo_pin) + " PWM值: " + String(pwm_value)).c_str());
            delay(100); // 给每个舵机一些响应时间
        }
    }
    delay(2000); // 等待舵机到达位置
    
    // ================== 舵机参数初始化 ==================
    set_site(2, x_default - x_offset, y_start + y_step, z_boot); // 右后腿 RR (leg 2)
    set_site(3, x_default - x_offset, y_start + y_step, z_boot); // 左后腿 RL (leg 3)
    set_site(0, x_default + x_offset, y_start, z_boot);          // 右前腿 FR (leg 0)
    set_site(1, x_default + x_offset, y_start, z_boot);          // 左前腿 FL (leg 1)
    
    // 同步当前坐标到期望坐标
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            sst.site_now[i][j] = sst.site_expect[i][j];
        }
    }

    // ================== 创建舵机控制任务 ==================
    Semaphore = xSemaphoreCreateMutex(); // 创建互斥信号量

    // 在核心1创建实时控制任务
    xTaskCreatePinnedToCore(
        servos_service,    // 任务函数
        "ServoService",    // 任务名称
        10000,             // 堆栈大小（字节）
        (void *)&sst,      // 共享状态参数
        1,                 // 任务优先级（高于loop）
        &Task0,            // 任务句柄
        1                  // 运行在核心1
    );

    // ================== 执行初始动作序列 ==================
    servos_start(); // 执行安全启动流程

    // ================== WiFi连接初始化 ==================
    customSerial("\n====== WiFi 连接信息 ======");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // 等待WiFi连接
    int wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
        delay(500);
        customSerial(".");
        wifiAttempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        customSerial("\n=== WiFi 连接成功! ===");
        customSerial(String("IP地址: " + WiFi.localIP().toString()).c_str());
        customSerial("========================");
        
        // 设置Web服务器路由
        server.on("/", handleRoot);
        server.on("/command", handleCommand);
        server.on("/status", handleStatus);
        server.begin();
    } else {
        customSerial("\n=== WiFi 连接失败! ===");
        customSerial("请检查WiFi设置");
        customSerial("========================");
    }

    // 初始化WebSocket服务器
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

// LED状态控制变量（当前未使用）
uint32_t ledPulse = 0;
uint32_t ledSpeed = LED_SPEED;

// 主循环处理函数
void servos_loop() {
    webSocket.loop();
    server.handleClient();
    action_cmd();
}

void controlPump(int legIndex, bool state) {
    if(legIndex >= 0 && legIndex < NUM_PUMPS) {
        digitalWrite(PUMP_PINS[legIndex], !state);  // 取反，因为继电器是高电平关闭
        customSerial(String("继电器 " + String(legIndex) + ": " + (state ? "开启" : "关闭")).c_str());
    }
}

void enterClimbingMode() {
    customSerial("[模式] 切换到爬墙模式");
    if(!climbing_mode) {
        climbing_mode = true;
        // 确保所有腿都在地面上
        if(!checkStability(-1)) {
            customSerial("无法进入爬墙模式：姿态不稳定");
            climbing_mode = false;
            return;
        }
        // 开启所有气泵
        for(int i = 0; i < NUM_PUMPS; i++) {
            controlPump(i, true);
        }
        delay(STABLE_TIME * 2);  // 等待所有气泵稳定
        customSerial("已进入爬墙模式");
    }
}

void exitClimbingMode() {
    customSerial("[模式] 退出爬墙模式");
    if(climbing_mode) {
        // 确保机器人安全着地
        if(!checkStability(-1)) {
            customSerial("无法安全退出爬墙模式");
            return;
        }
        climbing_mode = false;
        // 关闭所有气泵
        for(int i = 0; i < NUM_PUMPS; i++) {
            controlPump(i, false);
        }
        customSerial("已退出爬墙模式");
    }
}

/*
  - wait one end point move to expect site
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_reach(int leg) {
    while (1) {
        if (sst.site_now[leg][0] == sst.site_expect[leg][0]) {
            if (sst.site_now[leg][1] == sst.site_expect[leg][1]) {
                if (sst.site_now[leg][2] == sst.site_expect[leg][2]) {
                    break;
                }
            }
        }
    }
}

/*
  - wait all end points move to expect site
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_all_reach(void) {
    // 因为机器人有4条腿,所以需要等待所有4条腿都到达目标位置
    // 如果是小于3,就只会等待前3条腿,最后一条腿的位置就不会被检查
    for (int i = 0; i < 4; i++) {
        wait_reach(i);
    }
}

// 处理根路径请求
void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

// 处理命令请求
void handleCommand() {
    String cmd = server.arg("cmd");
    customSerial(("[命令] 接收到: " + cmd).c_str());
    int n_step = 1;
    if (server.hasArg("step")) {
        n_step = server.arg("step").toInt();
        if (n_step <= 0) n_step = 1;
    }
    bool valid = true;
    if (cmd == "forward") {
        if (getMode() == GROUND)
            trot_gait_ground(n_step);
        else
            crawl_gait_climb_forward(n_step);
    } else if (cmd == "backward") {
        if (getMode() == GROUND)
            step_back(n_step);
        else
            crawl_gait_climb_back(n_step);
    } else if (cmd == "left") {
        if (getMode() == GROUND)
            turn_left(n_step);
        else
            crawl_gait_climb_left(n_step);
    } else if (cmd == "right") {
        if (getMode() == GROUND)
            turn_right(n_step);
        else
            crawl_gait_climb_right(n_step);
    } else if (cmd == "sit") {
        sit();
    } else if (cmd == "stand") {
        stand();
    } else if (cmd == "set_mode_ground") {
        setMode(GROUND);
    } else if (cmd == "set_mode_climb") {
        setMode(CLIMB);
    } else if (cmd == "climb_forward") {
        crawl_gait_climb_forward(n_step);
    } else if (cmd == "climb_back") {
        crawl_gait_climb_back(n_step);
    } else if (cmd == "climb_left") {
        crawl_gait_climb_left(n_step);
    } else if (cmd == "climb_right") {
        crawl_gait_climb_right(n_step);
    } else {
        valid = false;
        customSerial(("[错误] 未知命令: " + cmd).c_str());
    }
    if (valid) {
        customSerial(("[命令] 执行完成: " + cmd).c_str());
    }
    String response = "命令已执行: " + cmd;
    server.send(200, "text/plain", response);
}

// 处理状态检查请求
void handleStatus() {
    String jsonResponse = "{";
    jsonResponse += "\"servoAngles\":[";
    int angles[12] = {sst.FRFoot, sst.FRElbow, sst.FRShdr, sst.FLFoot, sst.FLElbow, sst.FLShdr, sst.RRFoot, sst.RRElbow, sst.RRShdr, sst.RLFoot, sst.RLElbow, sst.RLShdr};
    for (int i = 0; i < 12; i++) {
        jsonResponse += String(angles[i]);
        if (i < 11) jsonResponse += ",";
    }
    jsonResponse += "],\"pumpStates\":[";
    for (int i = 0; i < 4; i++) {
        int state = digitalRead(PUMP_PINS[i]) == LOW ? 1 : 0; // 低电平为开启
        jsonResponse += String(state);
        if (i < 3) jsonResponse += ",";
    }
    jsonResponse += "]}";
    server.send(200, "application/json", jsonResponse);
}

// 自定义串口输出函数
void customSerial(const char* message) {
    Serial.println(message);
    webSocket.broadcastTXT(message);  // 通过WebSocket发送到所有客户端
}

// WebSocket事件处理函数
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            customSerial("WebSocket客户端断开连接");
            break;
        case WStype_CONNECTED:
            customSerial("WebSocket客户端连接成功");
            break;
        case WStype_TEXT:
            // 处理收到的文本消息
            break;
    }
}

// 新增：让所有舵机保持在90度
void move_all_servos_to_90() {
    for (int leg = 0; leg < 4; leg++) {
        for (int joint = 0; joint < 3; joint++) {
            pwm.setPWM(servo_pin[leg][joint], 0, angleToPWM(90));
            Serial.printf("leg %d joint %d pwm %d\n", leg, joint, angleToPWM(90));
        }
    }
}

// 模式管理相关实现
static RobotMode current_mode = GROUND;
void setMode(RobotMode mode) {
    current_mode = mode;
    if (mode == GROUND) {
        customSerial("[模式] 切换到地面模式");
    } else {
        customSerial("[模式] 切换到爬墙模式");
    }
}
RobotMode getMode() {
    return current_mode;
}

// 地面步态（对角步态）接口实现
void trot_gait_ground(int step) {
    // 这里直接调用原有的step_forward等地面步态函数
    step_forward(step);
}

// 删除从这里开始的所有爬墙相关函数:
// void crawl_gait_climb(int step)
// void crawl_gait_climb_forward(int step)
// void crawl_gait_climb_back(int step)
// void crawl_gait_climb_left(int step)
// void crawl_gait_climb_right(int step)  