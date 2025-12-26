#include "web_server.h"
#include "wifi_manager.h"
#include "plugin_manager.h"
#include "sensor_manager.h"
#include <ArduinoJson.h>

// 外部全局对象
extern WiFiManager wifiManager;
extern PluginManager pluginManager;
extern SensorManager sensorManager;
extern MessageManager messageManager;

// 定义网页内容
const char* WebServerManager::index_html = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InkClock - 设备管理</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock - 设备管理</h1>
            <p>智能墨水屏万年历设备管理界面</p>
        </header>
        
        <nav>
            <ul>
                <li><a href="/" class="active">设备状态</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
            </ul>
        </nav>
        
        <main>
            <section class="status-section">
                <h2>设备状态</h2>
                <div class="status-card">
                    <h3>基本信息</h3>
                    <ul>
                        <li><strong>设备名称:</strong> InkClock</li>
                        <li><strong>固件版本:</strong> v1.0</li>
                        <li><strong>IP地址:</strong> %IP_ADDRESS%</li>
                        <li><strong>MAC地址:</strong> %MAC_ADDRESS%</li>
                        <li><strong>运行时间:</strong> %UPTIME% 秒</li>
                        <li><strong>CPU温度:</strong> %CPU_TEMP% °C</li>
                        <li><strong>剩余内存:</strong> %FREE_MEM% KB</li>
                    </ul>
                </div>
                
                <div class="status-card">
                    <h3>WiFi状态</h3>
                    <ul>
                        <li><strong>SSID:</strong> %WIFI_SSID%</li>
                        <li><strong>信号强度:</strong> %WIFI_RSSI% dBm</li>
                        <li><strong>连接状态:</strong> %WIFI_STATUS%</li>
                        <li><strong>本地IP:</strong> %IP_ADDRESS%</li>
                        <li><strong>子网掩码:</strong> %SUBNET_MASK%</li>
                        <li><strong>网关:</strong> %GATEWAY%</li>
                        <li><strong>DNS:</strong> %DNS_SERVER%</li>
                    </ul>
                </div>
                
                <div class="status-card">
                    <h3>传感器数据</h3>
                    <ul>
                        <li><strong>温度:</strong> %TEMPERATURE% °C</li>
                        <li><strong>湿度:</strong> %HUMIDITY% %</li>
                        <li><strong>传感器状态:</strong> %SENSOR_STATUS%</li>
                        <li><strong>数据更新时间:</strong> %SENSOR_UPDATE_TIME% 秒前</li>
                    </ul>
                </div>
                
                <div class="status-card">
                    <h3>电源状态</h3>
                    <ul>
                        <li><strong>电池电压:</strong> %BATTERY_VOLTAGE% V</li>
                        <li><strong>电池电量:</strong> %BATTERY_LEVEL% %</li>
                        <li><strong>充电状态:</strong> %CHARGE_STATUS%</li>
                        <li><strong>功耗模式:</strong> %POWER_MODE%</li>
                    </ul>
                </div>
                
                <div class="status-card">
                    <h3>访问二维码</h3>
                    <div class="qrcode">
                        <img src="%QR_CODE_URL%" alt="访问二维码">
                        <p>扫码访问设备管理界面</p>
                    </div>
                </div>
            </section>
        </main>
        
        <footer>
            <p>&copy; 2025 InkClock. All rights reserved.</p>
        </footer>
    </div>
</body>
</html>
)";

const char* WebServerManager::settings_html = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InkClock - 设置</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock - 设置</h1>
            <p>智能墨水屏万年历设备设置</p>
        </header>
        
        <nav>
            <ul>
                <li><a href="/">设备状态</a></li>
                <li><a href="/settings" class="active">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
            </ul>
        </nav>
        
        <main>
            <section class="settings-section">
                <h2>基本设置</h2>
                <form action="/update_settings" method="POST">
                    <div class="form-group">
                        <label for="wifi_ssid">WiFi SSID:</label>
                        <input type="text" id="wifi_ssid" name="wifi_ssid" value="%WIFI_SSID%" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="wifi_password">WiFi 密码:</label>
                        <input type="password" id="wifi_password" name="wifi_password" value="%WIFI_PASSWORD%" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="time_zone">时区:</label>
                        <input type="number" id="time_zone" name="time_zone" value="%TIME_ZONE%" step="1" min="-12" max="12" required>
                        <small>例如: 中国为+8</small>
                    </div>
                    
                    <div class="form-group">
                        <label for="display_update_interval">显示更新间隔 (分钟):</label>
                        <input type="number" id="display_update_interval" name="display_update_interval" value="%DISPLAY_UPDATE_INTERVAL%" step="1" min="1" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="weather_update_interval">天气更新间隔 (小时):</label>
                        <input type="number" id="weather_update_interval" name="weather_update_interval" value="%WEATHER_UPDATE_INTERVAL%" step="1" min="1" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="stock_update_interval">股票更新间隔 (分钟):</label>
                        <input type="number" id="stock_update_interval" name="stock_update_interval" value="%STOCK_UPDATE_INTERVAL%" step="1" min="1" required>
                    </div>
                    
                    <div class="form-group">
                        <button type="submit">保存设置</button>
                    </div>
                </form>
            </section>
        </main>
        
        <footer>
            <p>&copy; 2025 InkClock. All rights reserved.</p>
        </footer>
    </div>
</body>
</html>
)";

const char* WebServerManager::plugin_html = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InkClock - 插件管理</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock - 插件管理</h1>
            <p>智能墨水屏万年历插件管理界面</p>
        </header>
        
        <nav>
            <ul>
                <li><a href="/">设备状态</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins" class="active">插件管理</a></li>
            </ul>
        </nav>
        
        <main>
            <section class="plugins-section">
                <h2>插件列表</h2>
                
                <div class="add-plugin">
                    <h3>添加新插件</h3>
                    <form action="/add_plugin" method="POST">
                        <div class="form-group">
                            <label for="plugin_name">插件名称:</label>
                            <input type="text" id="plugin_name" name="plugin_name" required>
                        </div>
                        
                        <div class="form-group">
                            <label for="plugin_url">插件URL:</label>
                            <input type="url" id="plugin_url" name="plugin_url" required>
                            <small>输入插件功能页面的完整URL</small>
                        </div>
                        
                        <div class="form-group">
                            <label for="plugin_refresh_interval">刷新时间:</label>
                            <div class="refresh-time">
                                <input type="number" id="plugin_refresh_interval" name="plugin_refresh_interval" value="60" step="1" min="1" required>
                                <select name="plugin_refresh_unit">
                                    <option value="second">秒</option>
                                    <option value="minute" selected>分钟</option>
                                    <option value="hour">小时</option>
                                    <option value="day">天</option>
                                </select>
                            </div>
                        </div>
                        
                        <div class="form-group">
                            <button type="submit">添加插件</button>
                        </div>
                    </form>
                </div>
                
                <div class="plugin-list">
                    <h3>已安装插件</h3>
                    %PLUGIN_LIST%
                </div>
            </section>
        </main>
        
        <footer>
            <p>&copy; 2025 InkClock. All rights reserved.</p>
        </footer>
    </div>
</body>
</html>
)";

const char* WebServerManager::style_css = R"(
/* 全局样式 - 现代化设计 */
:root {
    --primary-color: #4a6fa5;
    --primary-dark: #3a5d8a;
    --secondary-color: #6c757d;
    --success-color: #28a745;
    --danger-color: #dc3545;
    --warning-color: #ffc107;
    --info-color: #17a2b8;
    --light-color: #f8f9fa;
    --dark-color: #343a40;
    --gray-color: #6c757d;
    --gray-light: #e9ecef;
    --border-radius: 12px;
    --box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
    --box-shadow-hover: 0 6px 12px rgba(0, 0, 0, 0.15);
    --transition: all 0.3s ease;
    --font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: var(--font-family);
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: var(--dark-color);
    line-height: 1.6;
    min-height: 100vh;
    padding: 20px;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
}

/* 卡片基础样式 */
.card {
    background-color: white;
    border-radius: var(--border-radius);
    box-shadow: var(--box-shadow);
    padding: 24px;
    margin-bottom: 24px;
    transition: var(--transition);
    border: none;
}

.card:hover {
    box-shadow: var(--box-shadow-hover);
    transform: translateY(-2px);
}

/* 头部样式 - 现代化设计 */
header.card {
    background: linear-gradient(135deg, var(--primary-color) 0%, var(--primary-dark) 100%);
    color: white;
    text-align: center;
    padding: 32px 24px;
}

header h1 {
    font-size: 2.5rem;
    font-weight: 700;
    margin-bottom: 8px;
    letter-spacing: -0.5px;
}

header p {
    font-size: 1.1rem;
    opacity: 0.9;
    margin: 0;
}

/* 导航样式 - 现代化设计 */
nav.card {
    padding: 0;
    background: white;
}

nav ul {
    list-style: none;
    display: flex;
    justify-content: center;
    gap: 8px;
    flex-wrap: wrap;
    padding: 8px;
    margin: 0;
}

nav ul li a {
    text-decoration: none;
    color: var(--gray-color);
    padding: 12px 20px;
    border-radius: 50px;
    font-weight: 500;
    transition: var(--transition);
    font-size: 0.95rem;
    display: block;
}

nav ul li a:hover, nav ul li a.active {
    background-color: var(--primary-color);
    color: white;
    box-shadow: var(--box-shadow);
    transform: translateY(-1px);
}

/* 主要内容样式 */
main.card {
    background: white;
}

/* 卡片样式 - 现代化设计 */
.status-card {
    background: var(--light-color);
    border: 1px solid var(--gray-light);
    border-radius: var(--border-radius);
    padding: 20px;
    margin-bottom: 20px;
    transition: var(--transition);
}

.status-card:hover {
    border-color: var(--primary-color);
    box-shadow: var(--box-shadow);
}

.status-card h3 {
    color: var(--primary-color);
    margin-bottom: 16px;
    font-size: 1.3rem;
    font-weight: 600;
    display: flex;
    align-items: center;
    gap: 8px;
}

.status-card h3::before {
    content: '';
    width: 4px;
    height: 20px;
    background-color: var(--primary-color);
    border-radius: 2px;
}

/* 列表样式 - 现代化设计 */
.status-card ul {
    list-style: none;
    margin: 0;
    padding: 0;
}

.status-card ul li {
    margin-bottom: 12px;
    padding: 12px;
    background: white;
    border-radius: 8px;
    border: 1px solid var(--gray-light);
    transition: var(--transition);
    font-size: 0.95rem;
}

.status-card ul li:hover {
    border-color: var(--primary-color);
    box-shadow: var(--box-shadow);
}

.status-card ul li strong {
    color: var(--primary-color);
    font-weight: 600;
    min-width: 120px;
    display: inline-block;
}

/* 表单样式 - 现代化设计 */
.form-group {
    margin-bottom: 24px;
}

.form-group label {
    display: block;
    margin-bottom: 8px;
    font-weight: 600;
    color: var(--dark-color);
    font-size: 0.95rem;
}

.form-group input[type="text"],
.form-group input[type="password"],
.form-group input[type="number"],
.form-group input[type="url"],
.form-group select {
    width: 100%;
    padding: 14px 16px;
    border: 2px solid var(--gray-light);
    border-radius: var(--border-radius);
    font-size: 1rem;
    font-family: var(--font-family);
    transition: var(--transition);
    background: white;
}

.form-group input:focus,
.form-group select:focus {
    outline: none;
    border-color: var(--primary-color);
    box-shadow: 0 0 0 3px rgba(74, 111, 165, 0.1);
    transform: translateY(-1px);
}

.form-group small {
    display: block;
    margin-top: 8px;
    color: var(--gray-color);
    font-size: 0.85rem;
    line-height: 1.4;
}

/* 按钮样式 - 现代化设计 */
.form-group button,
.btn {
    background-color: var(--primary-color);
    color: white;
    border: none;
    padding: 14px 24px;
    border-radius: var(--border-radius);
    cursor: pointer;
    font-size: 1rem;
    font-weight: 600;
    transition: var(--transition);
    font-family: var(--font-family);
    display: inline-flex;
    align-items: center;
    gap: 8px;
    text-decoration: none;
    text-align: center;
    box-shadow: var(--box-shadow);
}

.form-group button:hover,
.btn:hover {
    background-color: var(--primary-dark);
    transform: translateY(-2px);
    box-shadow: var(--box-shadow-hover);
}

.form-group button:active,
.btn:active {
    transform: translateY(0);
}

.btn-danger {
    background-color: var(--danger-color);
}

.btn-danger:hover {
    background-color: #c82333;
}

.btn-success {
    background-color: var(--success-color);
}

.btn-success:hover {
    background-color: #218838;
}

.btn-warning {
    background-color: var(--warning-color);
    color: var(--dark-color);
}

.btn-warning:hover {
    background-color: #e0a800;
}

/* 按钮组样式 */
.btn-group {
    display: flex;
    gap: 12px;
    flex-wrap: wrap;
    margin-top: 24px;
}

/* 刷新时间选择器 */
.refresh-time {
    display: flex;
    gap: 12px;
    align-items: center;
    flex-wrap: wrap;
}

.refresh-time input {
    flex: 1;
    min-width: 150px;
}

.refresh-time select {
    width: auto;
    min-width: 120px;
}

/* 插件列表 - 现代化设计 */
.add-plugin {
    background: var(--light-color);
    padding: 24px;
    border-radius: var(--border-radius);
    margin-bottom: 24px;
    border: 1px solid var(--gray-light);
    transition: var(--transition);
}

.add-plugin:hover {
    border-color: var(--primary-color);
    box-shadow: var(--box-shadow);
}

.add-plugin h3 {
    color: var(--primary-color);
    margin-bottom: 20px;
    font-size: 1.3rem;
    font-weight: 600;
    display: flex;
    align-items: center;
    gap: 8px;
}

.add-plugin h3::before {
    content: '+';
    width: 24px;
    height: 24px;
    background-color: var(--primary-color);
    color: white;
    border-radius: 50%;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    font-size: 1.2rem;
    font-weight: 700;
    line-height: 1;
}

.plugin-list {
    margin: 24px 0;
}

.plugin-item {
    background: var(--light-color);
    padding: 20px;
    border-radius: var(--border-radius);
    margin-bottom: 16px;
    border: 1px solid var(--gray-light);
    transition: var(--transition);
}

.plugin-item:hover {
    border-color: var(--primary-color);
    box-shadow: var(--box-shadow);
    transform: translateY(-1px);
}

.plugin-header {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    margin-bottom: 12px;
    flex-wrap: wrap;
    gap: 12px;
}

.plugin-header h4 {
    color: var(--primary-color);
    margin: 0;
    font-size: 1.2rem;
    font-weight: 600;
}

.plugin-info {
    margin-bottom: 12px;
    font-size: 0.9rem;
    color: var(--gray-color);
    line-height: 1.5;
    background: white;
    padding: 12px;
    border-radius: 8px;
    border: 1px solid var(--gray-light);
}

.plugin-actions {
    display: flex;
    gap: 8px;
    flex-wrap: wrap;
    align-items: flex-start;
}

.plugin-actions form {
    display: inline;
}

.plugin-actions .btn {
    padding: 8px 16px;
    font-size: 0.85rem;
    white-space: nowrap;
}

/* 二维码样式 - 现代化设计 */
.qrcode {
    text-align: center;
    margin: 24px 0;
    padding: 20px;
    background: white;
    border-radius: var(--border-radius);
    box-shadow: var(--box-shadow);
}

.qrcode img {
    max-width: 200px;
    border: 2px solid var(--gray-light);
    border-radius: var(--border-radius);
    padding: 16px;
    background-color: white;
    transition: var(--transition);
    box-shadow: var(--box-shadow);
}

.qrcode img:hover {
    transform: scale(1.05);
    box-shadow: var(--box-shadow-hover);
}

.qrcode p {
    margin-top: 12px;
    color: var(--gray-color);
    font-size: 0.95rem;
    font-weight: 500;
}

/* 页脚样式 - 现代化设计 */
footer {
    text-align: center;
    margin-top: 32px;
    color: white;
    font-size: 0.9rem;
    opacity: 0.9;
    padding: 16px;
    background: rgba(0, 0, 0, 0.1);
    border-radius: var(--border-radius);
}

/* 响应式设计 - 移动端优化 */
@media (max-width: 768px) {
    body {
        padding: 12px;
    }
    
    .container {
        max-width: 100%;
    }
    
    header h1 {
        font-size: 2rem;
    }
    
    nav ul {
        flex-direction: column;
        align-items: stretch;
    }
    
    nav ul li a {
        text-align: center;
    }
    
    .plugin-header {
        flex-direction: column;
        align-items: stretch;
    }
    
    .plugin-actions {
        justify-content: center;
    }
    
    .btn-group {
        justify-content: center;
    }
    
    .status-card ul li {
        padding: 10px;
        font-size: 0.9rem;
    }
    
    .status-card ul li strong {
        min-width: 100px;
        display: block;
        margin-bottom: 4px;
    }
    
    .refresh-time {
        flex-direction: column;
        align-items: stretch;
    }
    
    .refresh-time input,
    .refresh-time select {
        width: 100%;
        min-width: auto;
    }
}

/* 加载状态样式 */
.loading {
    display: inline-block;
    width: 20px;
    height: 20px;
    border: 2px solid var(--gray-light);
    border-radius: 50%;
    border-top-color: var(--primary-color);
    animation: spin 1s ease-in-out infinite;
}

@keyframes spin {
    to { transform: rotate(360deg); }
}

/* 通知样式 */
.alert {
    padding: 16px;
    border-radius: var(--border-radius);
    margin-bottom: 20px;
    font-weight: 500;
    border-left: 4px solid transparent;
}

.alert-success {
    background-color: rgba(40, 167, 69, 0.1);
    color: var(--success-color);
    border-left-color: var(--success-color);
}

.alert-error {
    background-color: rgba(220, 53, 69, 0.1);
    color: var(--danger-color);
    border-left-color: var(--danger-color);
}

.alert-warning {
    background-color: rgba(255, 193, 7, 0.1);
    color: var(--warning-color);
    border-left-color: var(--warning-color);
}

.alert-info {
    background-color: rgba(23, 162, 184, 0.1);
    color: var(--info-color);
    border-left-color: var(--info-color);
}

/* 数据显示优化 */
.data-value {
    font-weight: 700;
    color: var(--primary-color);
    font-size: 1.1rem;
}

/* 状态指示器 */
.status-indicator {
    display: inline-block;
    width: 8px;
    height: 8px;
    border-radius: 50%;
    margin-right: 8px;
    vertical-align: middle;
}

.status-indicator.online {
    background-color: var(--success-color);
    animation: pulse 2s infinite;
}

.status-indicator.offline {
    background-color: var(--danger-color);
}

.status-indicator.warning {
    background-color: var(--warning-color);
    animation: pulse 1s infinite;
}

@keyframes pulse {
    0% { opacity: 1; }
    50% { opacity: 0.5; }
    100% { opacity: 1; }
}
)";

WebServerManager::WebServerManager() : server(8080), initialized(false) {
}

WebServerManager::~WebServerManager() {
}

void WebServerManager::init() {
    DEBUG_PRINTLN("初始化Web服务器...");
    
    // 设置路由处理函数
    server.on("/", std::bind(&WebServerManager::handleRoot, this));
    server.on("/settings", std::bind(&WebServerManager::handleSettings, this));
    server.on("/plugins", std::bind(&WebServerManager::handlePlugins, this));
    server.on("/update_settings", HTTP_POST, std::bind(&WebServerManager::handleUpdateSettings, this));
    server.on("/add_plugin", HTTP_POST, std::bind(&WebServerManager::handleAddPlugin, this));
    server.on("/update_plugin", HTTP_POST, std::bind(&WebServerManager::handleUpdatePlugin, this));
    server.on("/delete_plugin", HTTP_POST, std::bind(&WebServerManager::handleDeletePlugin, this));
    server.on("/enable_plugin", HTTP_POST, std::bind(&WebServerManager::handleEnablePlugin, this));
    server.on("/disable_plugin", HTTP_POST, std::bind(&WebServerManager::handleDisablePlugin, this));
    server.on("/style.css", std::bind(&WebServerManager::handleCSS, this));
    
    // API路由 - 设备管理API
    server.on("/api", std::bind(&WebServerManager::handleApi, this));
    server.on("/api/sensor", std::bind(&WebServerManager::handleSensorData, this));
    
    // API路由 - IPv6推送功能API（合并自IPv6Server）
    server.on("/api/push", HTTP_POST, std::bind(&WebServerManager::handleMessagePush, this));
    server.on("/api/status", HTTP_GET, std::bind(&WebServerManager::handleDeviceStatus, this));
    
    server.onNotFound(std::bind(&WebServerManager::handleNotFound, this));
    
    // 启动Web服务器
    server.begin();
    
    // 启动mDNS服务
    if (!MDNS.begin("inkclock")) {
        DEBUG_PRINTLN("Error starting mDNS");
    } else {
        DEBUG_PRINTLN("mDNS started: http://inkclock.local:8080");
    }
    
    initialized = true;
    DEBUG_PRINTLN("Web服务器初始化完成，端口: 8080");
}

void WebServerManager::loop() {
    if (initialized) {
        server.handleClient();
        MDNS.update();
    }
}

void WebServerManager::handleRoot() {
    DEBUG_PRINTLN("处理根路径请求");
    
    String html = String(index_html);
    
    // 替换模板变量
    html.replace("%IP_ADDRESS%", getIPAddress());
    html.replace("%MAC_ADDRESS%", WiFi.macAddress());
    html.replace("%WIFI_SSID%", WiFi.SSID());
    html.replace("%WIFI_RSSI%", String(WiFi.RSSI()));
    html.replace("%WIFI_STATUS%", WiFi.status() == WL_CONNECTED ? "已连接" : "未连接");
    html.replace("%QR_CODE_URL%", generateQRCodeURL());
    
    server.send(200, "text/html", html);
}

void WebServerManager::handleSettings() {
    DEBUG_PRINTLN("处理设置页面请求");
    
    String html = String(settings_html);
    
    // 替换模板变量
    html.replace("%WIFI_SSID%", WiFi.SSID());
    html.replace("%WIFI_PASSWORD%", ""); // 不显示密码
    html.replace("%TIME_ZONE%", String(TIME_ZONE_OFFSET));
    html.replace("%DISPLAY_UPDATE_INTERVAL%", String(DISPLAY_UPDATE_INTERVAL / 60000));
    html.replace("%WEATHER_UPDATE_INTERVAL%", String(WEATHER_UPDATE_INTERVAL / 3600000));
    html.replace("%STOCK_UPDATE_INTERVAL%", String(STOCK_UPDATE_INTERVAL / 60000));
    
    server.send(200, "text/html", html);
}

void WebServerManager::handlePlugins() {
    DEBUG_PRINTLN("处理插件管理页面请求");
    
    String html = String(plugin_html);
    
    // 生成插件列表
    String pluginList = "";
    int pluginCount = pluginManager.getPluginCount();
    
    if (pluginCount == 0) {
        pluginList = "<p>暂无插件，请添加新插件。</p>";
    } else {
        for (int i = 0; i < pluginCount; i++) {
            PluginData plugin = pluginManager.getPlugin(i);
            
            // 生成插件类型文本
            String pluginTypeText = "";
            switch (plugin.type) {
                case PLUGIN_TYPE_NATIVE: pluginTypeText = "原生插件";
                    break;
                case PLUGIN_TYPE_URL_XML: pluginTypeText = "URL XML插件";
                    break;
                case PLUGIN_TYPE_URL_JSON: pluginTypeText = "URL JSON插件";
                    break;
                case PLUGIN_TYPE_URL_JS: pluginTypeText = "URL JS插件";
                    break;
                default: pluginTypeText = "未知类型";
                    break;
            }
            
            // 生成状态文本和类名
            String statusText = "";
            String statusClass = "";
            switch (plugin.status) {
                case PLUGIN_DISABLED: 
                    statusText = "已禁用";
                    statusClass = "offline";
                    break;
                case PLUGIN_ENABLED: 
                case PLUGIN_RUNNING: 
                    statusText = "运行中";
                    statusClass = "online";
                    break;
                case PLUGIN_ERROR: 
                    statusText = "错误";
                    statusClass = "warning";
                    break;
                default: 
                    statusText = "未知状态";
                    statusClass = "offline";
                    break;
            }
            
            // 生成插件项HTML
            pluginList += "<div class=\"plugin-item\">";
            pluginList += "  <div class=\"plugin-header\">";
            pluginList += "    <div>";
            pluginList += "      <h4>" + plugin.name + "</h4>";
            pluginList += "      <div class=\"plugin-info\">";
            pluginList += "        <p><strong>版本:</strong> " + plugin.version + "</p>";
            pluginList += "        <p><strong>类型:</strong> " + pluginTypeText + "</p>";
            pluginList += "        <p><strong>状态:</strong> <span class=\"status-indicator \" + statusClass + \"></span>" + statusText + "</p>";
            pluginList += "        <p><strong>描述:</strong> " + plugin.description + "</p>";
            pluginList += "        <p><strong>更新间隔:</strong> " + String(plugin.urlData.updateInterval / 1000) + "秒</p>";
            pluginList += "        <p><strong>最后更新:</strong> " + String((millis() - plugin.urlData.lastUpdateTime) / 1000) + "秒前</p>";
            if (!plugin.urlData.lastData.isEmpty()) {
                pluginList += "        <p><strong>最新数据:</strong> <span class=\"data-value\">" + plugin.urlData.lastData.substring(0, min(plugin.urlData.lastData.length(), 50)) + (plugin.urlData.lastData.length() > 50 ? "..." : "") + "</span></p>";
            }
            pluginList += "      </div>";
            pluginList += "    </div>";
            pluginList += "    <div class=\"plugin-actions\">";
            pluginList += "      <form action=\"/update_plugin\" method=\"POST\" style=\"display:inline;\">";
            pluginList += "        <input type=\"hidden\" name=\"plugin_name\" value=\"" + plugin.name + "\">";
            pluginList += "        <button type=\"submit\" class=\"btn btn-success\">更新</button>";
            pluginList += "      </form>";
            if (plugin.status == PLUGIN_DISABLED) {
                pluginList += "      <form action=\"/enable_plugin\" method=\"POST\" style=\"display:inline;\">";
                pluginList += "        <input type=\"hidden\" name=\"plugin_name\" value=\"" + plugin.name + "\">";
                pluginList += "        <button type=\"submit\" class=\"btn btn-success\">启用</button>";
                pluginList += "      </form>";
            } else {
                pluginList += "      <form action=\"/disable_plugin\" method=\"POST\" style=\"display:inline;\">";
                pluginList += "        <input type=\"hidden\" name=\"plugin_name\" value=\"" + plugin.name + "\">";
                pluginList += "        <button type=\"submit\" class=\"btn btn-warning\">禁用</button>";
                pluginList += "      </form>";
            }
            pluginList += "      <form action=\"/delete_plugin\" method=\"POST\" style=\"display:inline;\">";
            pluginList += "        <input type=\"hidden\" name=\"plugin_name\" value=\"" + plugin.name + "\">";
            pluginList += "        <button type=\"submit\" class=\"btn btn-danger\">删除</button>";
            pluginList += "      </form>";
            pluginList += "    </div>";
            pluginList += "  </div>";
            pluginList += "</div>";
        }
    }
    
    html.replace("%PLUGIN_LIST%", pluginList);
    
    server.send(200, "text/html", html);
}

void WebServerManager::handleUpdateSettings() {
    DEBUG_PRINTLN("处理设置更新请求");
    
    // TODO: 处理设置更新
    
    // 重定向回设置页面
    server.sendHeader("Location", "/settings");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleAddPlugin() {
    DEBUG_PRINTLN("处理添加插件请求");
    
    // 获取表单数据
    String pluginName = server.arg("plugin_name");
    String pluginUrl = server.arg("plugin_url");
    int refreshInterval = server.arg("plugin_refresh_interval").toInt();
    String refreshUnit = server.arg("plugin_refresh_unit");
    
    // 转换刷新时间为毫秒
    unsigned long refreshTime = refreshInterval;
    if (refreshUnit == "second") {
        refreshTime *= 1000;
    } else if (refreshUnit == "minute") {
        refreshTime *= 60000;
    } else if (refreshUnit == "hour") {
        refreshTime *= 3600000;
    } else if (refreshUnit == "day") {
        refreshTime *= 86400000;
    }
    
    DEBUG_PRINT("添加插件: ");
    DEBUG_PRINT(pluginName);
    DEBUG_PRINT(", URL: ");
    DEBUG_PRINT(pluginUrl);
    DEBUG_PRINT(", 刷新时间: ");
    DEBUG_PRINT(refreshTime);
    DEBUG_PRINTLN("ms");
    
    // 自动检测插件类型
    PluginType pluginType = PLUGIN_TYPE_URL_JSON; // 默认JSON类型
    if (pluginUrl.endsWith(".xml") || pluginUrl.indexOf(".xml?") != -1) {
        pluginType = PLUGIN_TYPE_URL_XML;
    } else if (pluginUrl.endsWith(".js") || pluginUrl.indexOf(".js?") != -1) {
        pluginType = PLUGIN_TYPE_URL_JS;
    }
    
    // 调用PluginManager添加插件
    pluginManager.registerURLPlugin(pluginName, "1.0", "自动添加的URL插件", pluginType, pluginUrl, refreshTime, "", "%s");
    
    // 重定向回插件管理页面
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleUpdatePlugin() {
    DEBUG_PRINTLN("处理更新插件请求");
    
    // 获取表单数据
    String pluginName = server.arg("plugin_name");
    
    // 调用PluginManager更新插件
    pluginManager.updateURLPlugin(pluginName);
    
    // 重定向回插件管理页面
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleDeletePlugin() {
    DEBUG_PRINTLN("处理删除插件请求");
    
    // 获取表单数据
    String pluginName = server.arg("plugin_name");
    
    // 调用PluginManager删除插件
    pluginManager.unregisterPlugin(pluginName);
    
    // 重定向回插件管理页面
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleEnablePlugin() {
    DEBUG_PRINTLN("处理启用插件请求");
    
    // 获取表单数据
    String pluginName = server.arg("plugin_name");
    
    // 调用PluginManager启用插件
    pluginManager.enablePlugin(pluginName);
    
    // 重定向回插件管理页面
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleDisablePlugin() {
    DEBUG_PRINTLN("处理禁用插件请求");
    
    // 获取表单数据
    String pluginName = server.arg("plugin_name");
    
    // 调用PluginManager禁用插件
    pluginManager.disablePlugin(pluginName);
    
    // 重定向回插件管理页面
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleCSS() {
    DEBUG_PRINTLN("处理CSS请求");
    server.send(200, "text/css", style_css);
}

void WebServerManager::handleNotFound() {
    DEBUG_PRINT("处理404请求: ");
    DEBUG_PRINTLN(server.uri());
    server.send(404, "text/plain", "404 Not Found");
}

/**
 * @brief 处理传感器数据API请求
 * @note 返回JSON格式的传感器数据
 */
void WebServerManager::handleSensorData() {
    DEBUG_PRINTLN("处理传感器数据API请求");
    
    // 获取传感器数据
    SensorData data = sensorManager.getSensorData();
    
    // 创建JSON响应
    DynamicJsonDocument doc(256);
    doc["status"] = "success";
    doc["timestamp"] = data.timestamp;
    doc["data"]["temperature"] = data.temperature;
    doc["data"]["humidity"] = data.humidity;
    doc["data"]["valid"] = data.valid;
    
    // 设置响应头
    server.sendHeader("Content-Type", "application/json");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    // 发送响应
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

/**
 * @brief 处理API根请求
 * @note 返回API基本信息和支持的端点
 */
void WebServerManager::handleApi() {
    DEBUG_PRINTLN("处理API根请求");
    
    // 创建JSON响应
    DynamicJsonDocument doc(512);
    doc["status"] = "success";
    doc["name"] = "InkClock API";
    doc["version"] = "1.0";
    doc["description"] = "家用网络智能墨水屏万年历API";
    
    // 添加支持的API端点
    JsonArray endpoints = doc.createNestedArray("endpoints");
    
    JsonObject endpoint1 = endpoints.createNestedObject();
    endpoint1["url"] = "/api/sensor";
    endpoint1["method"] = "GET";
    endpoint1["description"] = "获取传感器数据";
    endpoint1["response"] = "{\"status\": \"success\", \"data\": {\"temperature\": 23.5, \"humidity\": 45.2}}";
    
    JsonObject endpoint2 = endpoints.createNestedObject();
    endpoint2["url"] = "/api/plugin/{name}/data";
    endpoint2["method"] = "GET";
    endpoint2["description"] = "获取插件数据";
    endpoint2["response"] = "{\"status\": \"success\", \"data\": \"插件数据\"}";
    
    // 设置响应头
    server.sendHeader("Content-Type", "application/json");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    // 发送响应
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

String WebServerManager::getIPAddress() {
    return WiFi.localIP().toString();
}

String WebServerManager::generateQRCodeURL() {
    String url = "http://" + getIPAddress() + ":8080";
    String encodedUrl = "https://api.qrserver.com/v1/create-qr-code/?size=200x200&data=";
    
    // URL编码
    for (int i = 0; i < url.length(); i++) {
        char c = url.charAt(i);
        if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedUrl += c;
        } else {
            encodedUrl += String("%" + String((uint8_t)c, HEX)).toUpperCase();
        }
    }
    
    return encodedUrl;
}

/**
 * @brief 处理消息推送API请求
 * @note 接收JSON格式的消息并添加到消息管理器
 */
void WebServerManager::handleMessagePush() {
    // 检查Content-Type
    String contentType = server.header("Content-Type");
    if (contentType != "application/json") {
        sendJsonResponse("{\"error\": \"Invalid Content-Type, application/json required\"}", 400);
        return;
    }
    
    // 读取请求体
    String body = server.arg("plain");
    if (body.length() == 0) {
        sendJsonResponse("{\"error\": \"Empty request body\"}", 400);
        return;
    }
    
    // 解析JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        String errorStr = "{\"error\": \"Invalid JSON: ";
        errorStr += error.c_str();
        errorStr += "\"}";
        sendJsonResponse(errorStr, 400);
        return;
    }
    
    // 检查必填字段
    if (!doc.containsKey("content")) {
        sendJsonResponse("{\"error\": \"Missing required field: content\"}", 400);
        return;
    }
    
    // 提取消息内容
    String content = doc["content"].as<String>();
    String sender = doc.containsKey("sender") ? doc["sender"].as<String>() : "Direct Push";
    String type = doc.containsKey("type") ? doc["type"].as<String>() : "text";
    
    // 转换消息类型
    MessageType messageType = MESSAGE_TEXT;
    if (type == "image") {
        messageType = MESSAGE_IMAGE;
    } else if (type == "audio") {
        messageType = MESSAGE_AUDIO;
    }
    
    // 添加消息到消息管理器
    bool success = messageManager.addMessage(sender, content, messageType);
    
    if (success) {
        sendJsonResponse("{\"success\": true, \"message\": \"Message pushed successfully\"}");
        DEBUG_PRINTLN("收到直接推送消息: " + content);
    } else {
        sendJsonResponse("{\"error\": \"Failed to push message\"}", 500);
        DEBUG_PRINTLN("消息推送失败: " + content);
    }
}

/**
 * @brief 处理设备状态API请求
 * @note 返回设备当前状态信息
 */
void WebServerManager::handleDeviceStatus() {
    DynamicJsonDocument doc(1024);
    
    doc["status"] = "online";
    doc["ip_address"] = getIPAddress();
    doc["ipv6_address"] = WiFi.localIPv6().toString();
    doc["mac_address"] = WiFi.macAddress();
    doc["time"] = getCurrentTime();
    
    String json;
    serializeJson(doc, json);
    
    sendJsonResponse(json);
}

/**
 * @brief 发送JSON响应
 * @param json JSON字符串
 * @param statusCode HTTP状态码
 */
void WebServerManager::sendJsonResponse(const String& json, int statusCode) {
    server.send(statusCode, "application/json", json);
}

/**
 * @brief 获取当前时间
 * @return 当前时间字符串，格式：YYYY-MM-DD HH:MM:SS
 */
String WebServerManager::getCurrentTime() {
    // 从NTP获取当前时间
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char timeString[20];
    sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d",
            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    return String(timeString);
}