#ifndef INDEX_HTML_H
#define INDEX_HTML_H

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GruBot Control</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        :root {
            /* 主色调 */
            --bg-dark: #000000;          /* 纯黑背景 */
            --bg-card: rgba(255, 255, 255, 0.08);  /* 半透明卡片背景 */
            --text-color: #ffffff;        /* 文字颜色 */
            
            /* 功能色 */
            --primary-color: #0A84FF;    /* iOS 蓝色 */
            --danger-color: #FF453A;     /* iOS 红色 */
            --secondary-color: #32D74B;   /* iOS 绿色 */
            --purple-color: #BF5AF2;      /* iOS 紫色 */
            --orange-color: #FF9F0A;      /* iOS 橙色 */
            
            /* 控制台相关 */
            --console-bg: rgba(0, 0, 0, 0.5);  /* 半透明黑色 */
            --console-text: rgba(255, 255, 255, 0.8);
            --console-time: var(--primary-color);
            
            /* 状态颜色 */
            --status-connected: var(--secondary-color);
            --status-disconnected: var(--danger-color);
            --status-normal: var(--secondary-color);
            --status-warning: var(--orange-color);
            --status-error: var(--danger-color);
        }

        @supports (backdrop-filter: blur(20px)) {
            .card, .header, .status {
                backdrop-filter: blur(20px);
                -webkit-backdrop-filter: blur(20px);
            }
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Text', 'SF Pro Icons', 'Helvetica Neue', sans-serif;
            background: radial-gradient(circle at top right, #1a1a1a, #000000);
            color: var(--text-color);
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            box-sizing: border-box;
        }

        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 20px;
            background: var(--bg-card);
            border-radius: 16px;
            margin-bottom: 20px;
            border: 1px solid rgba(255, 255, 255, 0.1);
        }

        .header h1 {
            margin: 0;
            font-size: 24px;
            font-weight: 600;
            letter-spacing: -0.5px;
        }

        .status-indicator {
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 8px 16px;
            background: rgba(255, 255, 255, 0.05);
            border-radius: 20px;
            font-size: 14px;
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background-color: var(--status-normal);
        }

        .main-container {
            display: grid;
            grid-template-columns: 1fr 1.2fr;
            gap: 20px;
        }

        .card {
            background: var(--bg-card);
            border-radius: 16px;
            padding: 20px;
            border: 1px solid rgba(255, 255, 255, 0.1);
        }

        .card-title {
            margin: 0 0 16px 0;
            font-size: 14px;
            color: rgba(255, 255, 255, 0.6);
            text-transform: uppercase;
            letter-spacing: 1.5px;
            font-weight: 600;
        }

        .btn {
            padding: 8px;
            border: none;
            border-radius: 8px;
            color: var(--text-color);
            font-weight: 500;
            font-size: 12px;
            cursor: pointer;
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
            background: rgba(255, 255, 255, 0.1);
            width: 100%;
            height: 32px;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .btn:hover { background: rgba(255, 255, 255, 0.15); }
        .btn:active { transform: scale(0.98); }
        .btn.active { background: rgba(255, 255, 255, 0.3); }

        .btn-primary { background: rgba(10, 132, 255, 0.3); }
        .btn-primary:hover { background: rgba(10, 132, 255, 0.4); }
        .btn-danger { background: rgba(255, 69, 58, 0.3); }
        .btn-danger:hover { background: rgba(255, 69, 58, 0.4); }
        .btn-secondary { background: rgba(50, 215, 75, 0.3); }
        .btn-secondary:hover { background: rgba(50, 215, 75, 0.4); }
        .btn-purple { background: rgba(191, 90, 242, 0.3); }
        .btn-purple:hover { background: rgba(191, 90, 242, 0.4); }
        .btn-orange { background: rgba(255, 159, 10, 0.3); }
        .btn-orange:hover { background: rgba(255, 159, 10, 0.4); }

        .movement-pad {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 8px;
            justify-items: center;
            padding: 8px;
            margin-bottom: 12px;
        }

        .action-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 8px;
            margin-bottom: 12px;
        }

        .status-section {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }

        .status-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 8px;
        }

        .status-item {
            background: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            padding: 8px;
            text-align: center;
        }

        .status-item-title {
            font-size: 11px;
            color: rgba(255, 255, 255, 0.6);
            margin: 0 0 4px 0;
        }

        .servo-indicator {
            width: 100%;
            aspect-ratio: 1;
            position: relative;
            max-width: 80px;
            margin: 0 auto;
        }

        .servo-value {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            font-size: 12px;
            font-weight: bold;
        }

        .console-container {
            flex: 1;
            min-height: 200px;
            background: var(--bg-card);
            border-radius: 16px;
            padding: 16px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            position: relative;
        }

        .console-controls {
            display: flex;
            gap: 8px;
            margin-bottom: 8px;
        }

        .console {
            background: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            padding: 12px;
            font-family: 'SF Mono', 'Menlo', monospace;
            height: 300px;
            overflow-y: auto;
            color: var(--console-text);
            font-size: 12px;
            line-height: 1.4;
            position: relative;
        }

        .console-scroll {
            position: absolute;
            right: 8px;
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .scroll-btn {
            background: rgba(255, 255, 255, 0.1);
            border: none;
            color: var(--text-color);
            width: 24px;
            height: 24px;
            border-radius: 4px;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 14px;
            transition: background 0.2s;
        }

        .scroll-btn:hover {
            background: rgba(255, 255, 255, 0.2);
        }

        .scroll-btn:active {
            background: rgba(255, 255, 255, 0.3);
        }

        .scroll-up {
            top: 8px;
        }

        .scroll-down {
            bottom: 8px;
        }

        .keyboard-hints {
            position: absolute;
            top: 5px;
            right: 5px;
            font-size: 10px;
            color: rgba(255, 255, 255, 0.4);
        }

        .touch-area {
            position: fixed;
            bottom: 20px;
            right: 20px;
            width: 150px;
            height: 150px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 50%;
            display: none;
            touch-action: none;
        }

        .touch-indicator {
            width: 40px;
            height: 40px;
            background: var(--primary-color);
            border-radius: 50%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
        }

        @media (max-width: 768px) {
            .main-container {
                grid-template-columns: 1fr;
            }
            
            .touch-area {
                display: block;
            }

            .status-grid {
                grid-template-columns: repeat(4, 1fr);
            }

            .servo-indicator {
                max-width: 60px;
            }
        }

        .mode-section {
            margin-top: 16px;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
            padding-top: 16px;
        }

        .mode-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 8px;
            margin-bottom: 12px;
        }

        .sequence-section {
            margin-top: 16px;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
            padding-top: 16px;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>GruBot Control</h1>
        <div class="status-indicator">
            <div class="status-dot"></div>
            <span>已连接</span>
        </div>
    </div>
    
    <div class="main-container">
        <div class="card">
            <h2 class="card-title">控制面板</h2>
            <div class="movement-pad">
                <div></div>
                <button class="btn btn-primary" onclick="sendCommand('forward')" data-key="ArrowUp">
                    前进
                    <span class="keyboard-hints">↑</span>
                </button>
                <div></div>
                <button class="btn btn-primary" onclick="sendCommand('left')" data-key="ArrowLeft">
                    左转
                    <span class="keyboard-hints">←</span>
                </button>
                <button class="btn btn-danger" onclick="sendCommand('stop')" data-key="Space">
                    停止
                    <span class="keyboard-hints">空格</span>
                </button>
                <button class="btn btn-primary" onclick="sendCommand('right')" data-key="ArrowRight">
                    右转
                    <span class="keyboard-hints">→</span>
                </button>
                <div></div>
                <button class="btn btn-primary" onclick="sendCommand('backward')" data-key="ArrowDown">
                    后退
                    <span class="keyboard-hints">↓</span>
                </button>
                <div></div>
            </div>

            <div class="action-grid">
                <button class="btn btn-secondary" onclick="sendCommand('stand')">站立</button>
                <button class="btn btn-secondary" onclick="sendCommand('sit')">坐下</button>
                <button class="btn btn-secondary" onclick="sendCommand('wave')">挥手</button>
                <button class="btn btn-secondary" onclick="sendCommand('dance')">跳舞</button>
                <button class="btn btn-purple" onclick="sendCommand('body_left')">身体左移</button>
                <button class="btn btn-purple" onclick="sendCommand('body_right')">身体右移</button>
                <button class="btn btn-purple" onclick="sendCommand('head_up')">头部上抬</button>
                <button class="btn btn-purple" onclick="sendCommand('head_down')">头部下压</button>
                <button class="btn btn-orange" onclick="sendCommand('wave')">摆手</button>
                <button class="btn btn-orange" onclick="sendCommand('shake')">握手</button>
            </div>

            <div class="mode-section">
                <h3 class="card-title" style="margin: 0 0 8px 0;">模式切换</h3>
                <div class="mode-grid">
                    <button class="btn" onclick="switchMode('normal')" id="mode-normal">普通模式</button>
                    <button class="btn" onclick="switchMode('climb')" id="mode-climb">攀爬模式</button>
                    <button class="btn" onclick="switchMode('dance')" id="mode-dance">舞蹈模式</button>
                    <button class="btn" onclick="switchMode('demo')" id="mode-demo">演示模式</button>
                </div>
            </div>

            <div class="sequence-section">
                <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
                    <h3 class="card-title" style="margin: 0;">动作序列</h3>
                    <div style="display: flex; gap: 4px;">
                        <button class="btn" style="width: auto; padding: 4px 8px;" onclick="startSequence()">运行</button>
                        <button class="btn" style="width: auto; padding: 4px 8px;" onclick="clearSequence()">清除</button>
                        <button class="btn" style="width: auto; padding: 4px 8px;" onclick="saveSequence()">保存</button>
                    </div>
                </div>
                <div class="sequence-actions" id="action-sequence">
                    <!-- 动作序列将在这里显示 -->
                </div>
            </div>
        </div>

        <div class="status-section">
            <div class="card" style="padding: 16px;">
                <h2 class="card-title" style="margin: 0 0 12px 0;">
                    系统状态
                    <button class="btn" style="width: auto; padding: 4px 8px; font-size: 12px;" onclick="refreshStatus()">刷新</button>
                </h2>
                <div class="status-grid">
                    <!-- 12个电机状态 -->
                    <div class="status-item">
                        <h3 class="status-item-title">电机 1</h3>
                        <div class="servo-indicator">
                            <canvas id="servo1"></canvas>
                            <div class="servo-value">90°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 2</h3>
                        <div class="servo-indicator">
                            <canvas id="servo2"></canvas>
                            <div class="servo-value">45°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 3</h3>
                        <div class="servo-indicator">
                            <canvas id="servo3"></canvas>
                            <div class="servo-value">180°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">气泵 1</h3>
                        <div class="servo-indicator">
                            <canvas id="pump1"></canvas>
                            <div class="servo-value">开启</div>
                        </div>
                    </div>
                    
                    <!-- 第二行 -->
                    <div class="status-item">
                        <h3 class="status-item-title">电机 4</h3>
                        <div class="servo-indicator">
                            <canvas id="servo4"></canvas>
                            <div class="servo-value">120°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 5</h3>
                        <div class="servo-indicator">
                            <canvas id="servo5"></canvas>
                            <div class="servo-value">60°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 6</h3>
                        <div class="servo-indicator">
                            <canvas id="servo6"></canvas>
                            <div class="servo-value">90°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">气泵 2</h3>
                        <div class="servo-indicator">
                            <canvas id="pump2"></canvas>
                            <div class="servo-value">关闭</div>
                        </div>
                    </div>

                    <!-- 第三行 -->
                    <div class="status-item">
                        <h3 class="status-item-title">电机 7</h3>
                        <div class="servo-indicator">
                            <canvas id="servo7"></canvas>
                            <div class="servo-value">150°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 8</h3>
                        <div class="servo-indicator">
                            <canvas id="servo8"></canvas>
                            <div class="servo-value">30°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 9</h3>
                        <div class="servo-indicator">
                            <canvas id="servo9"></canvas>
                            <div class="servo-value">75°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">气泵 3</h3>
                        <div class="servo-indicator">
                            <canvas id="pump3"></canvas>
                            <div class="servo-value">开启</div>
                        </div>
                    </div>

                    <!-- 第四行 -->
                    <div class="status-item">
                        <h3 class="status-item-title">电机 10</h3>
                        <div class="servo-indicator">
                            <canvas id="servo10"></canvas>
                            <div class="servo-value">100°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 11</h3>
                        <div class="servo-indicator">
                            <canvas id="servo11"></canvas>
                            <div class="servo-value">85°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">电机 12</h3>
                        <div class="servo-indicator">
                            <canvas id="servo12"></canvas>
                            <div class="servo-value">160°</div>
                        </div>
                    </div>
                    <div class="status-item">
                        <h3 class="status-item-title">气泵 4</h3>
                        <div class="servo-indicator">
                            <canvas id="pump4"></canvas>
                            <div class="servo-value">关闭</div>
                        </div>
                    </div>
                </div>
            </div>

            <div class="console-container">
                <h2 class="card-title" style="margin: 0;">串口监视器</h2>
                <div class="console-controls">
                    <button class="btn" style="width: auto; padding: 4px 8px; font-size: 12px;" onclick="clearConsole()">清除</button>
                    <button class="btn" style="width: auto; padding: 4px 8px; font-size: 12px;" onclick="toggleAutoScroll()">
                        <span id="autoScrollText">自动滚动: 开</span>
                    </button>
                </div>
                <div class="console" id="console">
                    <div class="console-scroll">
                        <button class="scroll-btn scroll-up" onclick="scrollConsole(-100)">↑</button>
                        <button class="scroll-btn scroll-down" onclick="scrollConsole(100)">↓</button>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="touch-area">
        <div class="touch-indicator"></div>
    </div>

    <script>
        let commandInterval = null;
        let charts = {};
        const consoleElement = document.getElementById('console');
        let autoScroll = true;
        let currentMode = 'normal';
        let actionSequence = [];

        // 初始化电机角度显示
        function initServoGauge(canvasId, initialValue) {
            const ctx = document.getElementById(canvasId).getContext('2d');
            return new Chart(ctx, {
                type: 'doughnut',
                data: {
                    datasets: [{
                        data: [initialValue, 360 - initialValue],
                        backgroundColor: [
                            'rgba(10, 132, 255, 0.6)',
                            'rgba(255, 255, 255, 0.1)'
                        ],
                        borderWidth: 0
                    }]
                },
                options: {
                    circumference: 270,
                    rotation: 225,
                    cutout: '80%',
                    responsive: true,
                    maintainAspectRatio: true,
                    plugins: {
                        legend: {
                            display: false
                        }
                    }
                }
            });
        }

        // 初始化气泵状态显示
        function initPumpGauge(canvasId, isOn) {
            const ctx = document.getElementById(canvasId).getContext('2d');
            return new Chart(ctx, {
                type: 'doughnut',
                data: {
                    datasets: [{
                        data: [100],
                        backgroundColor: [
                            isOn ? 'rgba(50, 215, 75, 0.6)' : 'rgba(255, 69, 58, 0.6)'
                        ],
                        borderWidth: 0
                    }]
                },
                options: {
                    circumference: 360,
                    rotation: 0,
                    cutout: '80%',
                    responsive: true,
                    maintainAspectRatio: true,
                    plugins: {
                        legend: {
                            display: false
                        }
                    }
                }
            });
        }

        // 更新所有图表
        function updateCharts() {
            // 模拟数据更新
            for (let i = 1; i <= 12; i++) {
                const angle = Math.floor(Math.random() * 180);
                charts[`servo${i}`].data.datasets[0].data = [angle, 360 - angle];
                charts[`servo${i}`].update();
                document.querySelector(`#servo${i} + .servo-value`).textContent = `${angle}°`;
            }

            // 更新气泵状态
            for (let i = 1; i <= 4; i++) {
                const isOn = Math.random() > 0.5;
                charts[`pump${i}`].data.datasets[0].backgroundColor = [
                    isOn ? 'rgba(50, 215, 75, 0.6)' : 'rgba(255, 69, 58, 0.6)'
                ];
                charts[`pump${i}`].update();
                document.querySelector(`#pump${i} + .servo-value`).textContent = isOn ? '开启' : '关闭';
            }
        }

        // 初始化所有图表
        function initCharts() {
            // 初始化电机图表
            for (let i = 1; i <= 12; i++) {
                charts[`servo${i}`] = initServoGauge(`servo${i}`, Math.floor(Math.random() * 180));
            }

            // 初始化气泵图表
            for (let i = 1; i <= 4; i++) {
                charts[`pump${i}`] = initPumpGauge(`pump${i}`, Math.random() > 0.5);
            }

            // 定期更新
            setInterval(updateCharts, 1000);
        }

        // 键盘控制
        const keyCommandMap = {
            'ArrowUp': 'forward',
            'ArrowDown': 'backward',
            'ArrowLeft': 'left',
            'ArrowRight': 'right',
            'Space': 'stop'
        };

        document.addEventListener('keydown', (e) => {
            const command = keyCommandMap[e.code];
            if (command) {
                e.preventDefault();
                const btn = document.querySelector(`[data-key="${e.code}"]`);
                if (btn && !btn.classList.contains('active')) {
                    btn.classList.add('active');
                    startCommand(command);
                }
            }
        });

        document.addEventListener('keyup', (e) => {
            const command = keyCommandMap[e.code];
            if (command) {
                e.preventDefault();
                const btn = document.querySelector(`[data-key="${e.code}"]`);
                if (btn) {
                    btn.classList.remove('active');
                    stopCommand();
                }
            }
        });

        // 触摸控制
        const touchArea = document.querySelector('.touch-area');
        const touchIndicator = document.querySelector('.touch-indicator');

        touchArea.addEventListener('pointerdown', handleTouch);
        touchArea.addEventListener('pointermove', handleTouch);
        touchArea.addEventListener('pointerup', () => {
            stopCommand();
            touchIndicator.style.transform = 'translate(-50%, -50%)';
        });

        function handleTouch(e) {
            const rect = touchArea.getBoundingClientRect();
            const centerX = rect.width / 2;
            const centerY = rect.height / 2;
            
            const x = e.clientX - rect.left - centerX;
            const y = e.clientY - rect.top - centerY;
            
            const angle = Math.atan2(y, x);
            const distance = Math.min(Math.hypot(x, y), 50);
            
            const moveX = Math.cos(angle) * distance;
            const moveY = Math.sin(angle) * distance;
            
            touchIndicator.style.transform = `translate(${moveX}px, ${moveY}px)`;
            
            if (distance > 20) {
                const command = getCommandFromAngle(angle);
                startCommand(command);
            } else {
                stopCommand();
            }
        }

        function getCommandFromAngle(angle) {
            const deg = angle * 180 / Math.PI;
            if (deg > -45 && deg <= 45) return 'right';
            if (deg > 45 && deg <= 135) return 'backward';
            if (deg > 135 || deg <= -135) return 'left';
            return 'forward';
        }

        // 命令控制
        function startCommand(cmd) {
            if (commandInterval) {
                clearInterval(commandInterval);
            }
            sendCommand(cmd);
            commandInterval = setInterval(() => sendCommand(cmd), 200);
        }

        function stopCommand() {
            if (commandInterval) {
                clearInterval(commandInterval);
                commandInterval = null;
                sendCommand('stop');
            }
        }

        function toggleAutoScroll() {
            autoScroll = !autoScroll;
            document.getElementById('autoScrollText').textContent = `自动滚动: ${autoScroll ? '开' : '关'}`;
        }

        function scrollConsole(amount) {
            const console = document.getElementById('console');
            console.scrollTop += amount;
        }

        function addConsoleMessage(message) {
            const time = new Date().toLocaleTimeString();
            const line = document.createElement('div');
            line.className = 'console-line';
            line.innerHTML = `<span class="console-time">[${time}]</span> ${message}`;
            consoleElement.appendChild(line);
            if (autoScroll) {
                consoleElement.scrollTop = consoleElement.scrollHeight;
            }
        }

        function clearConsole() {
            consoleElement.innerHTML = '';
        }

        function sendCommand(cmd) {
            fetch('/command?cmd=' + cmd)
                .then(response => response.text())
                .then(data => {
                    console.log('命令已发送:', cmd);
                    console.log('服务器响应:', data);
                    addConsoleMessage(`发送命令: ${cmd}`);
                    
                    // 更新状态指示灯
                    const statusDot = document.querySelector('.status-dot');
                    statusDot.style.backgroundColor = 'var(--status-connected)';
                })
                .catch(error => {
                    console.error('发送命令失败:', error);
                    addConsoleMessage(`发送命令失败: ${cmd}`);
                    
                    // 更新状态指示灯为错误状态
                    const statusDot = document.querySelector('.status-dot');
                    statusDot.style.backgroundColor = 'var(--status-error)';
                });
        }

        function refreshStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    data.servoAngles.forEach((angle, index) => {
                        charts[`servo${index + 1}`].data.datasets[0].data = [angle, 360 - angle];
                        charts[`servo${index + 1}`].update();
                        document.querySelector(`#servo${index + 1} + .servo-value`).textContent = `${angle}°`;
                    });
                    data.pumpStates.forEach((state, index) => {
                        const isOn = state === 1;
                        charts[`pump${index + 1}`].data.datasets[0].backgroundColor = [
                            isOn ? 'rgba(50, 215, 75, 0.6)' : 'rgba(255, 69, 58, 0.6)'
                        ];
                        charts[`pump${index + 1}`].update();
                        document.querySelector(`#pump${index + 1} + .servo-value`).textContent = isOn ? '开启' : '关闭';
                    });
                })
                .catch(error => console.error('Error fetching status:', error));
        }

        // 按钮长按控制
        document.querySelectorAll('.btn').forEach(btn => {
            btn.addEventListener('mousedown', () => {
                const command = btn.onclick.toString().match(/sendCommand\('(.+)'\)/)[1];
                startCommand(command);
            });
            
            btn.addEventListener('mouseup', () => {
                stopCommand();
            });
            
            btn.addEventListener('mouseleave', () => {
                stopCommand();
            });
        });

        // 添加鼠标滚轮支持
        document.getElementById('console').addEventListener('wheel', function(e) {
            if (!autoScroll) {
                e.preventDefault();
                this.scrollTop += e.deltaY;
            }
        });

        // 初始化
        window.addEventListener('load', () => {
            initCharts();
            addConsoleMessage('系统启动...');
            setInterval(refreshStatus, 1000); // 每秒刷新一次状态
        });

        function switchMode(mode) {
            currentMode = mode;
            document.querySelectorAll('.btn').forEach(btn => {
                btn.classList.remove('active');
            });
            document.getElementById(`mode-${mode}`).classList.add('active');
            addConsoleMessage(`切换到${mode}模式`);
        }

        function addToSequence(action) {
            actionSequence.push(action);
            updateSequenceDisplay();
        }

        function updateSequenceDisplay() {
            const container = document.getElementById('action-sequence');
            container.innerHTML = '';
            actionSequence.forEach((action, index) => {
                const chip = document.createElement('div');
                chip.className = 'action-chip';
                chip.innerHTML = `
                    ${action}
                    <span class="remove" onclick="removeAction(${index})">×</span>
                `;
                container.appendChild(chip);
            });
        }

        function removeAction(index) {
            actionSequence.splice(index, 1);
            updateSequenceDisplay();
        }

        function clearSequence() {
            actionSequence = [];
            updateSequenceDisplay();
        }

        function startSequence() {
            if (actionSequence.length === 0) {
                addConsoleMessage('没有可执行的动作序列');
                return;
            }
            addConsoleMessage('开始执行动作序列...');
            actionSequence.forEach((action, index) => {
                setTimeout(() => {
                    sendCommand(action);
                }, index * 1000);
            });
        }

        function saveSequence() {
            if (actionSequence.length === 0) {
                addConsoleMessage('没有可保存的动作序列');
                return;
            }
            const sequenceName = prompt('请输入序列名称：');
            if (sequenceName) {
                // 这里可以添加保存到本地存储或发送到服务器的逻辑
                addConsoleMessage(`动作序列 "${sequenceName}" 已保存`);
            }
        }

        // 修改原有的sendCommand函数
        const originalSendCommand = sendCommand;
        sendCommand = function(cmd) {
            originalSendCommand(cmd);
            // 如果不是从序列中执行的，就添加到序列中
            if (!actionSequence.includes(cmd)) {
                addToSequence(cmd);
            }
        }

        // 初始化
        document.getElementById('mode-normal').classList.add('active');
    </script>
</body>
</html> 
)rawliteral";

#endif // INDEX_HTML_H 