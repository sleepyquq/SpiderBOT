#include <WiFi.h>
#include <WebServer.h>
#include "../src/gruandbot/RobotControl.h"  // 修正为相对路径

// WiFi配置
const char* ssid = "YourSSID";
const char* password = "YourPassword";

// WebServer实例
WebServer server(80);

// 根页面处理函数
void handleRoot() {
    String html = "<html><body>";
    html += "<h1>四足机器人控制面板</h1>";
    html += "<button onclick=\"fetch('/cmd?cmd=forward')\">前进</button> ";
    html += "<button onclick=\"fetch('/cmd?cmd=backward')\">后退</button> ";
    html += "<button onclick=\"fetch('/cmd?cmd=left')\">左转</button> ";
    html += "<button onclick=\"fetch('/cmd?cmd=right')\">右转</button><br><br>";
    html += "<button onclick=\"fetch('/cmd?cmd=sit')\">坐下</button> ";
    html += "<button onclick=\"fetch('/cmd?cmd=stand')\">站立</button> ";
    html += "<button onclick=\"fetch('/cmd?cmd=shake')\">摇手</button> ";
    html += "<button onclick=\"fetch('/cmd?cmd=wave')\">挥手</button> ";
    html += "<button onclick=\"fetch('/cmd?cmd=dance')\">跳舞</button> ";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

// 机器人动作命令处理
void handleCommand() {
    String command = server.arg("cmd");
    int steps = server.arg("steps").toInt();
    if(steps == 0) steps = 1; // 默认步数
    if(command == "forward") servos_cmd('0', steps);
    else if(command == "backward") servos_cmd('1', steps);
    else if(command == "left") servos_cmd('2', steps);
    else if(command == "right") servos_cmd('3', steps);
    else if(command == "sit") servos_cmd('4', steps);
    else if(command == "stand") servos_cmd('8', steps);
    else if(command == "shake") servos_cmd('5', steps);
    else if(command == "wave") servos_cmd('6', steps);
    else if(command == "dance") servos_cmd('7', steps);
    server.send(200, "text/plain", "命令已执行");
}

// 404处理
void handleNotFound() {
    String message = "未找到资源\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\n";
    server.send(404, "text/plain", message);
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\n正在连接WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("已连接，IP地址: ");
    Serial.println(WiFi.localIP());

    // 路由
    server.on("/", handleRoot);
    server.on("/cmd", handleCommand);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("Web服务器已启动");

    // 初始化机器人（如有需要）
    servos_init();
}

void loop() {
    server.handleClient();
    delay(2);
} 