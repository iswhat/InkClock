#include "web_server.h"
#include "../services/wifi_manager.h"
#include "../extensions/plugin_manager.h"
#include "../modules/sensor_manager.h"
#include <ArduinoJson.h>

// 外部全局对象
extern WiFiManager wifiManager;
extern PluginManager pluginManager;
extern SensorManager sensorManager;
extern MessageManager messageManager;
extern GeoManager geoManager;

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
                <li><a href="/plugin_list">推荐插件</a></li>
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
    <style>
        /* 设置页面扩展样式 */
        .settings-container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 24px;
            margin: 24px 0;
        }
        
        .settings-card {
            background: var(--light-color);
            border: 1px solid var(--gray-light);
            border-radius: var(--border-radius);
            padding: 24px;
            transition: var(--transition);
        }
        
        .settings-card:hover {
            border-color: var(--primary-color);
            box-shadow: var(--box-shadow);
        }
        
        .settings-card h3 {
            color: var(--primary-color);
            margin-bottom: 20px;
            font-size: 1.3rem;
            font-weight: 600;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .settings-card h3::before {
            content: '';
            width: 4px;
            height: 20px;
            background-color: var(--primary-color);
            border-radius: 2px;
        }
        
        .form-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin-bottom: 12px;
        }
        
        @media (max-width: 768px) {
            .form-row {
                grid-template-columns: 1fr;
            }
        }
        
        /* 开关样式 */
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .toggle-slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        
        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        
        input:checked + .toggle-slider {
            background-color: var(--primary-color);
        }
        
        input:focus + .toggle-slider {
            box-shadow: 0 0 1px var(--primary-color);
        }
        
        input:checked + .toggle-slider:before {
            transform: translateX(26px);
        }
        
        /* 分组样式 */
        .form-group.checkbox-group {
            display: flex;
            align-items: center;
            gap: 12px;
        }
        
        .form-group.checkbox-group label {
            margin-bottom: 0;
        }
        
        /* 状态提示 */
        .status-message {
            background: rgba(40, 167, 69, 0.1);
            color: var(--success-color);
            padding: 12px;
            border-radius: 8px;
            margin: 20px 0;
            border-left: 4px solid var(--success-color);
        }
        
        .status-message.error {
            background: rgba(220, 53, 69, 0.1);
            color: var(--danger-color);
            border-left-color: var(--danger-color);
        }
        
        /* 按钮容器 */
        .button-container {
            display: flex;
            gap: 12px;
            flex-wrap: wrap;
            margin-top: 32px;
        }
    </style>
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
                <li><a href="/plugin_list">推荐插件</a></li>
            </ul>
        </nav>
        
        <main>
            %STATUS_MESSAGE%
            
            <form action="/update_settings" method="POST">
                <div class="settings-container">
                    <!-- WiFi设置 -->
                    <div class="settings-card">
                        <h3>WiFi设置</h3>
                        <div class="form-group">
                            <label for="wifi_ssid">WiFi SSID:</label>
                            <input type="text" id="wifi_ssid" name="wifi_ssid" value="%WIFI_SSID%" required>
                        </div>
                        
                        <div class="form-group">
                            <label for="wifi_password">WiFi 密码:</label>
                            <input type="password" id="wifi_password" name="wifi_password" value="%WIFI_PASSWORD%" required>
                            <small>密码长度至少8个字符</small>
                        </div>
                    </div>
                    
                    <!-- 时间设置 -->
                    <div class="settings-card">
                        <h3>时间设置</h3>
                        <div class="form-group">
                            <label for="time_zone">时区:</label>
                            <input type="number" id="time_zone" name="time_zone" value="%TIME_ZONE%" step="0.5" min="-12" max="14" required>
                            <small>例如: 中国为+8</small>
                        </div>
                        
                        <div class="form-row">
                            <div class="form-group">
                                <label for="ntp_server">NTP服务器:</label>
                                <input type="text" id="ntp_server" name="ntp_server" value="%NTP_SERVER%" required>
                            </div>
                            
                            <div class="form-group">
                                <label for="ntp_server_backup">备用NTP服务器:</label>
                                <input type="text" id="ntp_server_backup" name="ntp_server_backup" value="%NTP_SERVER_BACKUP%" required>
                            </div>
                        </div>
                    </div>
                    
                    <!-- 显示设置 -->
                    <div class="settings-card">
                        <h3>显示设置</h3>
                        <div class="form-row">
                            <div class="form-group">
                                <label for="display_update_interval">显示更新间隔 (分钟):</label>
                                <input type="number" id="display_update_interval" name="display_update_interval" value="%DISPLAY_UPDATE_INTERVAL%" step="1" min="1" required>
                            </div>
                            
                            <div class="form-group">
                                <label for="display_rotation">显示旋转角度:</label>
                                <select id="display_rotation" name="display_rotation">
                                    <option value="0" %DISPLAY_ROTATION_0%>0度</option>
                                    <option value="90" %DISPLAY_ROTATION_90%>90度</option>
                                    <option value="180" %DISPLAY_ROTATION_180%>180度</option>
                                    <option value="270" %DISPLAY_ROTATION_270%>270度</option>
                                </select>
                            </div>
                        </div>
                        
                        <div class="form-group checkbox-group">
                            <label for="display_inverse">显示反色:</label>
                            <div class="toggle-switch">
                                <input type="checkbox" id="display_inverse" name="display_inverse" %DISPLAY_INVERSE%>
                                <span class="toggle-slider"></span>
                            </div>
                        </div>
                    </div>
                    
                    <!-- 天气设置 -->
                    <div class="settings-card">
                        <h3>天气设置</h3>
                        <div class="form-row">
                            <div class="form-group">
                                <label for="weather_update_interval">天气更新间隔 (小时):</label>
                                <input type="number" id="weather_update_interval" name="weather_update_interval" value="%WEATHER_UPDATE_INTERVAL%" step="1" min="1" required>
                            </div>
                            
                            <div class="form-group">
                                <label for="weather_api_key">天气API密钥:</label>
                                <input type="text" id="weather_api_key" name="weather_api_key" value="%WEATHER_API_KEY%" placeholder="输入API密钥">
                            </div>
                        </div>
                        
                        <div class="form-group">
                            <label for="weather_api_key_backup">备用天气API密钥:</label>
                            <input type="text" id="weather_api_key_backup" name="weather_api_key_backup" value="%WEATHER_API_KEY_BACKUP%" placeholder="输入备用API密钥">
                        </div>
                    </div>
                    
                    <!-- 地理位置设置 -->
                    <div class="settings-card">
                        <h3>地理位置设置</h3>
                        
                        <div class="form-group checkbox-group">
                            <label for="auto_detect_location">自动检测地理位置:</label>
                            <div class="toggle-switch">
                                <input type="checkbox" id="auto_detect_location" name="auto_detect_location" %AUTO_DETECT_LOCATION%>
                                <span class="toggle-slider"></span>
                            </div>
                        </div>
                        
                        <div class="form-group">
                            <label for="city_id">城市ID:</label>
                            <input type="text" id="city_id" name="city_id" value="%CITY_ID%" placeholder="例如: 101010100">
                        </div>
                        
                        <div class="form-row">
                            <div class="form-group">
                                <label for="city_name">城市名称:</label>
                                <input type="text" id="city_name" name="city_name" value="%CITY_NAME%" placeholder="例如: 北京">
                            </div>
                        </div>
                        
                        <div class="form-row">
                            <div class="form-group">
                                <label for="latitude">纬度:</label>
                                <input type="number" id="latitude" name="latitude" value="%LATITUDE%" step="0.01" placeholder="例如: 39.9042">
                            </div>
                            
                            <div class="form-group">
                                <label for="longitude">经度:</label>
                                <input type="number" id="longitude" name="longitude" value="%LONGITUDE%" step="0.01" placeholder="例如: 116.4074">
                            </div>
                        </div>
                    </div>
                    
                    <!-- 插件设置 -->
                    <div class="settings-card">
                        <h3>插件设置</h3>
                        <div class="form-row">
                            <div class="form-group">
                                <label for="stock_update_interval">股票更新间隔 (分钟):</label>
                                <input type="number" id="stock_update_interval" name="stock_update_interval" value="%STOCK_UPDATE_INTERVAL%" step="1" min="1" required>
                            </div>
                        </div>
                        
                        <div class="form-group checkbox-group">
                            <label for="auto_update_plugins">自动更新插件:</label>
                            <div class="toggle-switch">
                                <input type="checkbox" id="auto_update_plugins" name="auto_update_plugins" %AUTO_UPDATE_PLUGINS%>
                                <span class="toggle-slider"></span>
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- 保存按钮 -->
                <div class="button-container">
                    <button type="submit" class="btn btn-primary">保存设置</button>
                    <button type="button" class="btn btn-secondary" onclick="resetForm()">重置表单</button>
                </div>
            </form>
        </main>
        
        <footer>
            <p>&copy; 2025 InkClock. All rights reserved.</p>
        </footer>
    </div>
    
    <script>
        // 表单重置功能
        function resetForm() {
            const form = document.querySelector('form');
            form.reset();
        }
        
        // 自动检测地理位置开关
        const autoDetectCheckbox = document.getElementById('auto_detect_location');
        const geoFields = document.querySelectorAll('#city_id, #city_name, #latitude, #longitude');
        
        // 初始状态设置
        function updateGeoFields() {
            const isAutoDetect = autoDetectCheckbox.checked;
            geoFields.forEach(field => {
                field.disabled = isAutoDetect;
                field.style.opacity = isAutoDetect ? '0.5' : '1';
            });
        }
        
        // 监听开关变化
        autoDetectCheckbox.addEventListener('change', updateGeoFields);
        
        // 初始化
        updateGeoFields();
        
        // 表单验证
        document.querySelector('form').addEventListener('submit', function(e) {
            // 可以在这里添加自定义验证逻辑
        });
    </script>
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
                <li><a href="/plugin_list">推荐插件</a></li>
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

const char* WebServerManager::plugin_list_html = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InkClock - 推荐插件</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock - 推荐插件</h1>
            <p>智能墨水屏万年历推荐插件列表</p>
        </header>
        
        <nav>
            <ul>
                <li><a href="/">设备状态</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
                <li><a href="/plugin_list" class="active">推荐插件</a></li>
            </ul>
        </nav>
        
        <main>
            <section class="plugins-section">
                <h2>推荐插件列表</h2>
                <p>以下是推荐的网络插件，您可以将其添加到您的设备设置中。点击"添加到设备"按钮即可快速添加。</p>
                
                <div class="recommended-plugins">
                    <!-- 每日古诗插件 -->
                    <div class="plugin-item">
                        <h3>每日古诗</h3>
                        <div class="plugin-info">
                            <p><strong>类型:</strong> URL JSON插件</p>
                            <p><strong>描述:</strong> 每天获取一首经典古诗，展示在您的万年历上</p>
                            <p><strong>更新频率:</strong> 每天</p>
                            <p><strong>作者:</strong> iswhat</p>
                            <p><strong>插件URL:</strong> <span class="plugin-url">http://<device_ip>:8080/plugin/daily_poem/index.php</span></p>
                        </div>
                        <div class="plugin-actions">
                            <button class="btn btn-primary" onclick="copyUrl('http://<device_ip>:8080/plugin/daily_poem/index.php')">复制URL</button>
                            <button class="btn btn-success" onclick="addPlugin('每日古诗', 'http://<device_ip>:8080/plugin/daily_poem/index.php', '86400', 'second')">添加到设备</button>
                            <button class="btn btn-secondary" onclick="window.open('http://<device_ip>:8080/plugin/daily_poem/index.php', '_blank')">预览效果</button>
                        </div>
                    </div>
                    
                    <!-- 每日英语单词插件 -->
                    <div class="plugin-item">
                        <h3>每日英语单词</h3>
                        <div class="plugin-info">
                            <p><strong>类型:</strong> URL JSON插件</p>
                            <p><strong>描述:</strong> 每天获取一个英语单词，包含发音、释义和例句</p>
                            <p><strong>更新频率:</strong> 每天</p>
                            <p><strong>作者:</strong> iswhat</p>
                            <p><strong>插件URL:</strong> <span class="plugin-url">http://<device_ip>:8080/plugin/daily_word/index.php</span></p>
                        </div>
                        <div class="plugin-actions">
                            <button class="btn btn-primary" onclick="copyUrl('http://<device_ip>:8080/plugin/daily_word/index.php')">复制URL</button>
                            <button class="btn btn-success" onclick="addPlugin('每日英语单词', 'http://<device_ip>:8080/plugin/daily_word/index.php', '86400', 'second')">添加到设备</button>
                            <button class="btn btn-secondary" onclick="window.open('http://<device_ip>:8080/plugin/daily_word/index.php', '_blank')">预览效果</button>
                        </div>
                    </div>
                </div>
            </section>
        </main>
        
        <footer>
            <p>&copy; 2025 InkClock. All rights reserved.</p>
        </footer>
    </div>
    
    <script>
        // 获取设备IP地址
        const deviceIp = location.hostname;
        
        // 替换所有设备IP占位符
        document.querySelectorAll('.plugin-url').forEach(el => {
            el.textContent = el.textContent.replace('<device_ip>', deviceIp);
        });
        
        // 复制URL到剪贴板
        function copyUrl(url) {
            // 替换IP地址
            const fullUrl = url.replace('<device_ip>', deviceIp);
            
            if (navigator.clipboard) {
                navigator.clipboard.writeText(fullUrl).then(() => {
                    alert('URL已复制到剪贴板！');
                }).catch(err => {
                    fallbackCopyTextToClipboard(fullUrl);
                });
            } else {
                fallbackCopyTextToClipboard(fullUrl);
            }
        }
        
        // 备用复制方法
        function fallbackCopyTextToClipboard(text) {
            const textArea = document.createElement('textarea');
            textArea.value = text;
            document.body.appendChild(textArea);
            textArea.select();
            
            try {
                const successful = document.execCommand('copy');
                if (successful) {
                    alert('URL已复制到剪贴板！');
                } else {
                    alert('复制失败，请手动复制');
                }
            } catch (err) {
                alert('复制失败，请手动复制');
            }
            
            document.body.removeChild(textArea);
        }
        
        // 添加插件到设备
        function addPlugin(name, url, interval, unit) {
            // 替换IP地址
            const fullUrl = url.replace('<device_ip>', deviceIp);
            
            // 构建表单数据
            const form = document.createElement('form');
            form.method = 'POST';
            form.action = '/add_plugin';
            
            // 添加表单字段
            const nameInput = document.createElement('input');
            nameInput.type = 'hidden';
            nameInput.name = 'plugin_name';
            nameInput.value = name;
            form.appendChild(nameInput);
            
            const urlInput = document.createElement('input');
            urlInput.type = 'hidden';
            urlInput.name = 'plugin_url';
            urlInput.value = fullUrl;
            form.appendChild(urlInput);
            
            const intervalInput = document.createElement('input');
            intervalInput.type = 'hidden';
            intervalInput.name = 'plugin_refresh_interval';
            intervalInput.value = interval;
            form.appendChild(intervalInput);
            
            const unitInput = document.createElement('input');
            unitInput.type = 'hidden';
            unitInput.name = 'plugin_refresh_unit';
            unitInput.value = unit;
            form.appendChild(unitInput);
            
            // 提交表单
            document.body.appendChild(form);
            form.submit();
        }
    </script>
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
    server.on("/plugin_list", std::bind(&WebServerManager::handlePluginList, this));
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
    
    // 替换地理位置相关模板变量
    String autoDetectChecked = geoManager.isAutoDetect() ? "checked" : "";
    html.replace("%AUTO_DETECT_LOCATION%", autoDetectChecked);
    html.replace("%CITY_ID%", geoManager.getCityId());
    html.replace("%CITY_NAME%", geoManager.getCityName());
    html.replace("%LATITUDE%", String(geoManager.getLatitude()));
    html.replace("%LONGITUDE%", String(geoManager.getLongitude()));
    
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
    
    // 处理地理位置设置
    bool autoDetectLocation = server.hasArg("auto_detect_location");
    String cityId = server.arg("city_id");
    String cityName = server.arg("city_name");
    float latitude = server.arg("latitude").toFloat();
    float longitude = server.arg("longitude").toFloat();
    
    // 设置自动检测
    geoManager.setAutoDetect(autoDetectLocation);
    
    // 如果关闭了自动检测或者提供了具体的地理位置信息，则更新地理位置
    if (!autoDetectLocation || (!cityId.isEmpty() || !cityName.isEmpty() || (latitude != 0.0 && longitude != 0.0))) {
        GeoLocation geoInfo;
        geoInfo.cityId = cityId;
        geoInfo.cityName = cityName;
        geoInfo.latitude = latitude;
        geoInfo.longitude = longitude;
        geoInfo.country = "中国"; // 默认值
        geoInfo.region = ""; // 默认值
        geoInfo.autoDetected = false;
        
        geoManager.setLocation(geoInfo);
    }
    
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

void WebServerManager::handlePluginList() {
    DEBUG_PRINTLN("处理推荐插件列表请求");
    
    String html = String(plugin_list_html);
    
    // 发送响应
    server.send(200, "text/html", html);
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
    
    // 公共免密钥二维码生成API列表
    String qrApiUrls[] = {
        "https://api.qrserver.com/v1/create-qr-code/?size=200x200&data=", // 主API（公共免密钥）
        "https://api.qrcode-monkey.com/qr/custom?size=200&data=", // 备用API（公共免密钥）
        "https://qrcode.tec-it.com/API/QRCode?size=200&data=" // 次备用API（公共免密钥）
    };
    
    // 选择主API
    String encodedUrl = qrApiUrls[0];
    
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