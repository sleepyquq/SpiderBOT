#include "ground.h"

// 三足吸附爬墙步态：每次只移动一条腿，其余三腿始终吸附
// step：前进步数
void crawl_gait_climb(int step) {
    if (!climbing_mode) {
        Serial.println("错误：未进入爬墙模式！");
        return;
    }
    
    int leg_order[4] = {0, 3, 2, 1}; // 可自定义顺序
    for (int s = 0; s < step; s++) {
        for (int i = 0; i < 4; i++) {
            int leg = leg_order[i];
            if (!checkStability(leg)) {
                Serial.println("警告：姿态不稳定，取消移动");
                return;
            }
            controlPump(leg, false);
            delay(STABLE_TIME);
            set_site(leg, KEEP, KEEP, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, sst.site_now[leg][1] + y_step, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, KEEP, z_default);
            wait_reach(leg);
            controlPump(leg, true);
            delay(STABLE_TIME);
        }
    }
}

// 墙面前进
void crawl_gait_climb_forward(int step) {
    if (!climbing_mode) {
        Serial.println("错误：未进入爬墙模式！");
        return;
    }
    
    for (int s = 0; s < step; s++) {
        for (int i = 0; i < 4; i++) {
            int leg = i;
            if (!checkStability(leg)) {
                Serial.println("警告：姿态不稳定，取消移动");
                return;
            }
            controlPump(leg, false);
            delay(STABLE_TIME);
            set_site(leg, KEEP, KEEP, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, sst.site_now[leg][1] + y_step, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, KEEP, z_default);
            wait_reach(leg);
            controlPump(leg, true);
            delay(STABLE_TIME);
        }
    }
}

// 墙面后退
void crawl_gait_climb_back(int step) {
    if (!climbing_mode) {
        Serial.println("错误：未进入爬墙模式！");
        return;
    }
    
    for (int s = 0; s < step; s++) {
        for (int i = 0; i < 4; i++) {
            int leg = i;
            if (!checkStability(leg)) {
                Serial.println("警告：姿态不稳定，取消移动");
                return;
            }
            controlPump(leg, false);
            delay(STABLE_TIME);
            set_site(leg, KEEP, KEEP, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, sst.site_now[leg][1] - y_step, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, KEEP, z_default);
            wait_reach(leg);
            controlPump(leg, true);
            delay(STABLE_TIME);
        }
    }
}

// 墙面左移
void crawl_gait_climb_left(int step) {
    if (!climbing_mode) {
        Serial.println("错误：未进入爬墙模式！");
        return;
    }
    
    for (int s = 0; s < step; s++) {
        for (int i = 0; i < 4; i++) {
            int leg = i;
            if (!checkStability(leg)) {
                Serial.println("警告：姿态不稳定，取消移动");
                return;
            }
            controlPump(leg, false);
            delay(STABLE_TIME);
            set_site(leg, KEEP, KEEP, z_up);
            wait_reach(leg);
            set_site(leg, sst.site_now[leg][0] - x_step, KEEP, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, KEEP, z_default);
            wait_reach(leg);
            controlPump(leg, true);
            delay(STABLE_TIME);
        }
    }
}

// 墙面右移
void crawl_gait_climb_right(int step) {
    if (!climbing_mode) {
        Serial.println("错误：未进入爬墙模式！");
        return;
    }
    
    for (int s = 0; s < step; s++) {
        for (int i = 0; i < 4; i++) {
            int leg = i;
            if (!checkStability(leg)) {
                Serial.println("警告：姿态不稳定，取消移动");
                return;
            }
            controlPump(leg, false);
            delay(STABLE_TIME);
            set_site(leg, KEEP, KEEP, z_up);
            wait_reach(leg);
            set_site(leg, sst.site_now[leg][0] + x_step, KEEP, z_up);
            wait_reach(leg);
            set_site(leg, KEEP, KEEP, z_default);
            wait_reach(leg);
            controlPump(leg, true);
            delay(STABLE_TIME);
        }
    }
} 