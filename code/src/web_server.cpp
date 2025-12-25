#include "web_server.h"
#include "wifi_manager.h"
#include "plugin_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern PluginManager pluginManager;

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
                    </ul>
                </div>
                
                <div class="status-card">
                    <h3>WiFi状态</h3>
                    <ul>
                        <li><strong>SSID:</strong> %WIFI_SSID%</li>
                        <li><strong>信号强度:</strong> %WIFI_RSSI% dBm</li>
                        <li><strong>连接状态:</strong> %WIFI_STATUS%</li>
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
/* 全局样式 */
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: Arial, sans-serif;
    background-color: #f4f4f4;
    color: #333;
    line-height: 1.6;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 20px;
}

/* 头部样式 */
header {
    background-color: #4a6fa5;
    color: white;
    padding: 20px;
    border-radius: 8px;
    text-align: center;
    margin-bottom: 20px;
}

header h1 {
    font-size: 2em;
    margin-bottom: 10px;
}

/* 导航样式 */
nav {
    background-color: white;
    padding: 15px;
    border-radius: 8px;
    margin-bottom: 20px;
    box-shadow: 0 2px 5px rgba(0,0,0,0.1);
}

nav ul {
    list-style: none;
    display: flex;
    justify-content: center;
    gap: 20px;
}

nav ul li a {
    text-decoration: none;
    color: #333;
    padding: 10px 15px;
    border-radius: 4px;
    transition: background-color 0.3s;
}

nav ul li a:hover, nav ul li a.active {
    background-color: #4a6fa5;
    color: white;
}

/* 主要内容样式 */
main {
    background-color: white;
    padding: 20px;
    border-radius: 8px;
    box-shadow: 0 2px 5px rgba(0,0,0,0.1);
}

/* 卡片样式 */
.status-card {
    background-color: #f9f9f9;
    padding: 20px;
    border-radius: 8px;
    margin-bottom: 20px;
    border: 1px solid #ddd;
}

.status-card h3 {
    color: #4a6fa5;
    margin-bottom: 15px;
    font-size: 1.2em;
}

.status-card ul {
    list-style: none;
}

.status-card ul li {
    margin-bottom: 10px;
    padding-left: 20px;
    position: relative;
}

.status-card ul li::before {
    content: "•";
    color: #4a6fa5;
    font-weight: bold;
    position: absolute;
    left: 0;
}

/* 表单样式 */
.form-group {
    margin-bottom: 20px;
}

.form-group label {
    display: block;
    margin-bottom: 5px;
    font-weight: bold;
}

.form-group input[type="text"],
.form-group input[type="password"],
.form-group input[type="number"],
.form-group input[type="url"],
.form-group select {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-size: 1em;
}

.form-group small {
    display: block;
    margin-top: 5px;
    color: #666;
    font-size: 0.8em;
}

.form-group button {
    background-color: #4a6fa5;
    color: white;
    padding: 12px 20px;
    border: none;
    border-radius: 4px;
    font-size: 1em;
    cursor: pointer;
    transition: background-color 0.3s;
}

.form-group button:hover {
    background-color: #3a5a8a;
}

/* 刷新时间选择器 */
.refresh-time {
    display: flex;
    gap: 10px;
    align-items: center;
}

.refresh-time input {
    flex: 1;
}

.refresh-time select {
    width: auto;
    min-width: 100px;
}

/* 插件列表 */
.add-plugin {
    background-color: #f9f9f9;
    padding: 20px;
    border-radius: 8px;
    margin-bottom: 20px;
    border: 1px solid #ddd;
}

.add-plugin h3 {
    color: #4a6fa5;
    margin-bottom: 15px;
    font-size: 1.2em;
}

.plugin-item {
    background-color: #f9f9f9;
    padding: 15px;
    border-radius: 8px;
    margin-bottom: 15px;
    border: 1px solid #ddd;
    position: relative;
}

.plugin-item h4 {
    color: #4a6fa5;
    margin-bottom: 10px;
    font-size: 1.1em;
}

.plugin-item .plugin-info {
    margin-bottom: 10px;
    color: #666;
    font-size: 0.9em;
}

.plugin-item .plugin-actions {
    display: flex;
    gap: 10px;
}

.plugin-item .plugin-actions form {
    display: inline;
}

.plugin-item .plugin-actions button {
    background-color: #e74c3c;
    color: white;
    padding: 8px 15px;
    border: none;
    border-radius: 4px;
    font-size: 0.9em;
    cursor: pointer;
    transition: background-color 0.3s;
}

.plugin-item .plugin-actions button:hover {
    background-color: #c0392b;
}

.plugin-item .plugin-actions .edit-btn {
    background-color: #f39c12;
}

.plugin-item .plugin-actions .edit-btn:hover {
    background-color: #e67e22;
}

/* 二维码样式 */
.qrcode {
    text-align: center;
}

.qrcode img {
    max-width: 200px;
    height: auto;
    margin-bottom: 10px;
}

/* 页脚样式 */
footer {
    text-align: center;
    margin-top: 20px;
    color: #666;
    font-size: 0.9em;
}

/* 响应式设计 */
@media (max-width: 768px) {
    nav ul {
        flex-direction: column;
        align-items: center;
    }
    
    .refresh-time {
        flex-direction: column;
        align-items: stretch;
    }
    
    .refresh-time select {
        width: 100%;
    }
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
    server.on("/style.css", std::bind(&WebServerManager::handleCSS, this));
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
    // TODO: 从PluginManager获取插件列表并生成HTML
    pluginList = "<p>暂无插件，请添加新插件。</p>";
    
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
    
    // TODO: 调用PluginManager添加插件
    
    // 重定向回插件管理页面
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleUpdatePlugin() {
    DEBUG_PRINTLN("处理更新插件请求");
    
    // TODO: 处理更新插件
    
    // 重定向回插件管理页面
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleDeletePlugin() {
    DEBUG_PRINTLN("处理删除插件请求");
    
    // TODO: 处理删除插件
    
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