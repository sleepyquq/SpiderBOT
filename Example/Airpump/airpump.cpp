// 定义四路继电器的控制引脚
const int RELAY_PINS[] = {
    5, 6, 7, 8  // 这里的引脚号根据实际连接修改
};
const int NUM_RELAYS = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);

// 记录继电器状态的数组（true表示开启，false表示关闭）
bool relayStates[4] = {false, false, false, false};

void setup() {
    // 初始化串口通信，用于调试和控制
    Serial.begin(115200);
    Serial.println("四路继电器控制系统已启动");

    // 设置所有继电器引脚为输出模式
    for (int i = 0; i < NUM_RELAYS; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], LOW);  // 初始状态设为关闭
    }
}

// 控制单个继电器的函数
void controlRelay(int relayNum, bool state) {
    if (relayNum >= 0 && relayNum < NUM_RELAYS) {
        digitalWrite(RELAY_PINS[relayNum], state ? HIGH : LOW);
        relayStates[relayNum] = state;
        Serial.print("继电器 ");
        Serial.print(relayNum + 1);
        Serial.println(state ? " 已开启" : " 已关闭");
    }
}

// 控制所有继电器的函数
void controlAllRelays(bool state) {
    for (int i = 0; i < NUM_RELAYS; i++) {
        controlRelay(i, state);
    }
    Serial.println(state ? "所有继电器已开启" : "所有继电器已关闭");
}

// 切换单个继电器状态的函数
void toggleRelay(int relayNum) {
    if (relayNum >= 0 && relayNum < NUM_RELAYS) {
        controlRelay(relayNum, !relayStates[relayNum]);
    }
}

void loop() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case '1':
            case '2':
            case '3':
            case '4':
                // 切换对应继电器的状态
                toggleRelay(cmd - '1');
                break;
                
            case 'a':
            case 'A':
                // 开启所有继电器
                controlAllRelays(true);
                break;
                
            case 'b':
            case 'B':
                // 关闭所有继电器
                controlAllRelays(false);
                break;
                
            case 's':
            case 'S':
                // 显示所有继电器的状态
                Serial.println("继电器状态：");
                for (int i = 0; i < NUM_RELAYS; i++) {
                    Serial.print("继电器 ");
                    Serial.print(i + 1);
                    Serial.println(relayStates[i] ? " : 开启" : " : 关闭");
                }
                break;
        }
    }
}