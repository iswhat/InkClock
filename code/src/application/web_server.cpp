#include "web_server.h"
#include "application/wifi_manager.h"
#include "../extensions/plugin_manager.h"
#include "sensor_manager.h"
#include "../coresystem/font_manager.h"
#include "../coresystem/core_system.h"
#include "display_manager.h"
#include <ArduinoJson.h>

#if defined(ESP32)
#include "../coresystem/tf_card_manager.h"
#endif

// 外部全局对象
CoreSystem* coreSystem;

// 外部全局对象
extern WiFiManager wifiManager;
extern PluginManager pluginManager;
extern SensorManager sensorManager;
extern MessageManager messageManager;

extern DisplayManager displayManager;

// 完整的HTML内容
const char* WebServerManager::index_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <nav>
            <ul>
                <li><a href="/">首页</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
                <li><a href="/fonts">字体管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
            </ul>
        </nav>
        <main>
            <section class="status-section">
                <h2>设备状态</h2>
                <div class="status-item">
                    <span class="label">IP地址:</span>
                    <span class="value" id="ip-address">加载中...</span>
                </div>
                <div class="status-item">
                    <span class="label">MAC地址:</span>
                    <span class="value" id="mac-address">加载中...</span>
                </div>
                <div class="status-item">
                    <span class="label">当前时间:</span>
                    <span class="value" id="current-time">加载中...</span>
                </div>
                <div class="status-item">
                    <span class="label">系统状态:</span>
                    <span class="value" id="system-status">加载中...</span>
                </div>
            </section>
            <section class="quick-actions">
                <h2>快速操作</h2>
                <button onclick="refreshDisplay()">刷新显示</button>
                <button onclick="factoryReset()">恢复出厂设置</button>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
    <script>
        // 加载设备状态
        function loadStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('ip-address').textContent = data.ip_address;
                    document.getElementById('mac-address').textContent = data.mac_address;
                    document.getElementById('current-time').textContent = data.time;
                    document.getElementById('system-status').textContent = data.status;
                });
        }
        
        // 刷新显示
        function refreshDisplay() {
            fetch('/api/refresh')
                .then(response => response.json())
                .then(data => {
                    alert(data.message);
                });
        }
        
        // 恢复出厂设置
        function factoryReset() {
            if (confirm('确定要恢复出厂设置吗？')) {
                fetch('/factory_reset')
                    .then(() => {
                        alert('恢复出厂设置成功，设备将重启');
                    });
            }
        }
        
        // 页面加载完成后加载状态
        window.onload = loadStatus;
    </script>
</body>
</html>)=====";

const char* WebServerManager::settings_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>设置 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <nav>
            <ul>
                <li><a href="/">首页</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
                <li><a href="/fonts">字体管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
            </ul>
        </nav>
        <main>
            <section class="settings-section">
                <h2>系统设置</h2>
                <form action="/update_settings" method="post">
                    <div class="form-group">
                        <label for="ntp_server">NTP服务器:</label>
                        <input type="text" id="ntp_server" name="ntp_server" value="pool.ntp.org">
                    </div>
                    <div class="form-group">
                        <label for="timezone">时区:</label>
                        <select id="timezone" name="timezone">
                            <option value="8">UTC+8 (中国)</option>
                            <option value="0">UTC+0 (格林威治)</option>
                            <option value="-5">UTC-5 (美国东部)</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="update_interval">更新间隔 (分钟):</label>
                        <input type="number" id="update_interval" name="update_interval" value="15" min="1" max="60">
                    </div>
                    <div class="form-group">
                        <label for="display_mode">显示模式:</label>
                        <select id="display_mode" name="display_mode">
                            <option value="1">时钟模式</option>
                            <option value="2">日历模式</option>
                            <option value="3">天气模式</option>
                            <option value="4">混合模式</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="low_power_mode">低功耗模式:</label>
                        <input type="checkbox" id="low_power_mode" name="low_power_mode">
                    </div>
                    <div class="form-group">
                        <label for="geo_location">地理位置:</label>
                        <input type="text" id="geo_location" name="geo_location" placeholder="城市名称">
                    </div>
                    <div class="form-group">
                        <button type="submit">保存设置</button>
                    </div>
                </form>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
</body>
</html>)=====";

const char* WebServerManager::plugin_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>插件管理 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <nav>
            <ul>
                <li><a href="/">首页</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
                <li><a href="/fonts">字体管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
            </ul>
        </nav>
        <main>
            <section class="plugins-section">
                <h2>已安装插件</h2>
                <div class="plugin-list" id="plugin-list">
                    <!-- 插件列表将通过JavaScript加载 -->
                    <p>加载中...</p>
                </div>
                <h2>添加插件</h2>
                <form action="/add_plugin" method="post" enctype="multipart/form-data">
                    <div class="form-group">
                        <label for="plugin_file">选择插件文件:</label>
                        <input type="file" id="plugin_file" name="plugin_file" accept=".bin">
                    </div>
                    <div class="form-group">
                        <button type="submit">上传插件</button>
                    </div>
                </form>
                <a href="/plugin_list">浏览推荐插件</a>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
    <script>
        // 加载插件列表
        function loadPlugins() {
            fetch('/api/plugins')
                .then(response => response.json())
                .then(data => {
                    const pluginList = document.getElementById('plugin-list');
                    if (data.plugins && data.plugins.length > 0) {
                        pluginList.innerHTML = '';
                        data.plugins.forEach(plugin => {
                            const pluginItem = document.createElement('div');
                            pluginItem.className = 'plugin-item';
                            pluginItem.innerHTML = `
                                <h3>${plugin.name}</h3>
                                <p>版本: ${plugin.version}</p>
                                <p>描述: ${plugin.description}</p>
                                <p>状态: ${plugin.enabled ? '已启用' : '已禁用'}</p>
                                <button onclick="togglePlugin('${plugin.id}', ${!plugin.enabled})">
                                    ${plugin.enabled ? '禁用' : '启用'}
                                </button>
                                <button onclick="deletePlugin('${plugin.id}')">删除</button>
                            `;
                            pluginList.appendChild(pluginItem);
                        });
                    } else {
                        pluginList.innerHTML = '<p>暂无已安装插件</p>';
                    }
                });
        }
        
        // 切换插件状态
        function togglePlugin(id, enable) {
            fetch(`/api/plugin/${id}/${enable ? 'enable' : 'disable'}`)
                .then(response => response.json())
                .then(data => {
                    alert(data.message);
                    loadPlugins();
                });
        }
        
        // 删除插件
        function deletePlugin(id) {
            if (confirm('确定要删除该插件吗？')) {
                fetch(`/api/plugin/${id}/delete`)
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message);
                        loadPlugins();
                    });
            }
        }
        
        // 页面加载完成后加载插件列表
        window.onload = loadPlugins;
    </script>
</body>
</html>)=====";

const char* WebServerManager::plugin_list_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>推荐插件 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <nav>
            <ul>
                <li><a href="/">首页</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
                <li><a href="/fonts">字体管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
            </ul>
        </nav>
        <main>
            <section class="plugin-list-section">
                <h2>推荐插件</h2>
                <div class="plugin-grid">
                    <div class="plugin-card">
                        <h3>天气插件</h3>
                        <p>显示实时天气和天气预报</p>
                        <p>版本: 1.0.0</p>
                        <button>安装</button>
                    </div>
                    <div class="plugin-card">
                        <h3>股票插件</h3>
                        <p>显示股票市场数据</p>
                        <p>版本: 1.0.0</p>
                        <button>安装</button>
                    </div>
                    <div class="plugin-card">
                        <h3>消息推送插件</h3>
                        <p>接收和显示消息通知</p>
                        <p>版本: 1.0.0</p>
                        <button>安装</button>
                    </div>
                    <div class="plugin-card">
                        <h3>智能家居插件</h3>
                        <p>控制智能家居设备</p>
                        <p>版本: 1.0.0</p>
                        <button>安装</button>
                    </div>
                </div>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
</body>
</html>)=====";

const char* WebServerManager::fonts_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>字体管理 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <nav>
            <ul>
                <li><a href="/">首页</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
                <li><a href="/fonts">字体管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
            </ul>
        </nav>
        <main>
            <section class="fonts-section">
                <h2>已安装字体</h2>
                <div class="font-list" id="font-list">
                    <!-- 字体列表将通过JavaScript加载 -->
                    <p>加载中...</p>
                </div>
                <h2>上传字体</h2>
                <form action="/upload_font" method="post" enctype="multipart/form-data">
                    <div class="form-group">
                        <label for="font_file">选择字体文件:</label>
                        <input type="file" id="font_file" name="font_file" accept=".ttf,.otf">
                    </div>
                    <div class="form-group">
                        <button type="submit">上传字体</button>
                    </div>
                </form>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
    <script>
        // 加载字体列表
        function loadFonts() {
            fetch('/api/fonts')
                .then(response => response.json())
                .then(data => {
                    const fontList = document.getElementById('font-list');
                    if (data.fonts && data.fonts.length > 0) {
                        fontList.innerHTML = '';
                        data.fonts.forEach(font => {
                            const fontItem = document.createElement('div');
                            fontItem.className = 'font-item';
                            fontItem.innerHTML = `
                                <h3>${font.name}</h3>
                                <p>大小: ${font.size}KB</p>
                                <button onclick="setDefaultFont('${font.id}')">设为默认</button>
                                <button onclick="deleteFont('${font.id}')">删除</button>
                            `;
                            fontList.appendChild(fontItem);
                        });
                    } else {
                        fontList.innerHTML = '<p>暂无已安装字体</p>';
                    }
                });
        }
        
        // 设置默认字体
        function setDefaultFont(id) {
            fetch(`/api/font/${id}/setdefault`)
                .then(response => response.json())
                .then(data => {
                    alert(data.message);
                });
        }
        
        // 删除字体
        function deleteFont(id) {
            if (confirm('确定要删除该字体吗？')) {
                fetch(`/api/font/${id}/delete`)
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message);
                        loadFonts();
                    });
            }
        }
        
        // 页面加载完成后加载字体列表
        window.onload = loadFonts;
    </script>
</body>
</html>)=====";

const char* WebServerManager::tfcard_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>TF卡管理 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <nav>
            <ul>
                <li><a href="/">首页</a></li>
                <li><a href="/settings">设置</a></li>
                <li><a href="/plugins">插件管理</a></li>
                <li><a href="/fonts">字体管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
            </ul>
        </nav>
        <main>
            <section class="tfcard-section">
                <h2>TF卡状态</h2>
                <div class="tfcard-status" id="tfcard-status">
                    <!-- TF卡状态将通过JavaScript加载 -->
                    <p>加载中...</p>
                </div>
                <h2>文件管理</h2>
                <div class="file-list" id="file-list">
                    <!-- 文件列表将通过JavaScript加载 -->
                    <p>加载中...</p>
                </div>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
    <script>
        // 加载TF卡状态
        function loadTFCardStatus() {
            fetch('/api/tfcard/status')
                .then(response => response.json())
                .then(data => {
                    const tfcardStatus = document.getElementById('tfcard-status');
                    if (data.exists) {
                        tfcardStatus.innerHTML = `
                            <div class="status-item">
                                <span class="label">状态:</span>
                                <span class="value">已挂载</span>
                            </div>
                            <div class="status-item">
                                <span class="label">总容量:</span>
                                <span class="value">${data.total_space}MB</span>
                            </div>
                            <div class="status-item">
                                <span class="label">可用容量:</span>
                                <span class="value">${data.free_space}MB</span>
                            </div>
                        `;
                    } else {
                        tfcardStatus.innerHTML = '<p>未检测到TF卡</p>';
                    }
                });
        }
        
        // 加载文件列表
        function loadFileList() {
            fetch('/api/tfcard/files')
                .then(response => response.json())
                .then(data => {
                    const fileList = document.getElementById('file-list');
                    if (data.files && data.files.length > 0) {
                        fileList.innerHTML = '';
                        data.files.forEach(file => {
                            const fileItem = document.createElement('div');
                            fileItem.className = 'file-item';
                            fileItem.innerHTML = `
                                <span>${file.name}</span>
                                <span>${file.size}KB</span>
                                <button onclick="deleteFile('${file.name}')">删除</button>
                            `;
                            fileList.appendChild(fileItem);
                        });
                    } else {
                        fileList.innerHTML = '<p>TF卡为空</p>';
                    }
                });
        }
        
        // 删除文件
        function deleteFile(filename) {
            if (confirm(`确定要删除文件 ${filename} 吗？`)) {
                fetch(`/api/tfcard/delete?file=${filename}`)
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message);
                        loadFileList();
                    });
            }
        }
        
        // 页面加载完成后加载状态
        window.onload = function() {
            loadTFCardStatus();
            loadFileList();
        };
    </script>
</body>
</html>)=====";

const char* WebServerManager::style_css = R"=====(/* 全局样式 */
* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

body {
    font-family: Arial, sans-serif;
    line-height: 1.6;
    color: #333;
    background-color: #f4f4f4;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 20px;
}

/* 头部样式 */
header {
    background-color: #333;
    color: #fff;
    padding: 20px;
    text-align: center;
    margin-bottom: 20px;
    border-radius: 5px;
}

header h1 {
    margin-bottom: 10px;
}

/* 导航样式 */
nav {
    background-color: #444;
    margin-bottom: 20px;
    border-radius: 5px;
}

nav ul {
    list-style: none;
    display: flex;
    flex-wrap: wrap;
}

nav ul li {
    flex: 1;
    text-align: center;
}

nav ul li a {
    display: block;
    padding: 15px;
    color: #fff;
    text-decoration: none;
    transition: background-color 0.3s;
}

nav ul li a:hover {
    background-color: #555;
}

/* 主内容样式 */
main {
    background-color: #fff;
    padding: 20px;
    border-radius: 5px;
    box-shadow: 0 2px 5px rgba(0,0,0,0.1);
}

section {
    margin-bottom: 30px;
}

section h2 {
    margin-bottom: 20px;
    color: #333;
    border-bottom: 2px solid #333;
    padding-bottom: 10px;
}

/* 状态项目样式 */
.status-item {
    display: flex;
    justify-content: space-between;
    margin-bottom: 10px;
    padding: 10px;
    background-color: #f9f9f9;
    border-radius: 3px;
}

.status-item .label {
    font-weight: bold;
}

/* 表单样式 */
.form-group {
    margin-bottom: 15px;
}

.form-group label {
    display: block;
    margin-bottom: 5px;
    font-weight: bold;
}

.form-group input[type="text"],
.form-group input[type="number"],
.form-group select {
    width: 100%;
    padding: 8px;
    border: 1px solid #ddd;
    border-radius: 3px;
}

.form-group button {
    padding: 10px 20px;
    background-color: #333;
    color: #fff;
    border: none;
    border-radius: 3px;
    cursor: pointer;
    transition: background-color 0.3s;
}

.form-group button:hover {
    background-color: #555;
}

/* 快速操作按钮 */
.quick-actions {
    display: flex;
    gap: 10px;
    margin-top: 20px;
}

.quick-actions button {
    padding: 10px 20px;
    background-color: #333;
    color: #fff;
    border: none;
    border-radius: 3px;
    cursor: pointer;
    transition: background-color 0.3s;
}

.quick-actions button:hover {
    background-color: #555;
}

/* 插件列表样式 */
.plugin-list {
    margin-top: 20px;
}

.plugin-item {
    background-color: #f9f9f9;
    padding: 15px;
    margin-bottom: 10px;
    border-radius: 3px;
    border-left: 4px solid #333;
}

.plugin-item h3 {
    margin-bottom: 10px;
}

.plugin-item button {
    margin-right: 10px;
    padding: 5px 10px;
    background-color: #333;
    color: #fff;
    border: none;
    border-radius: 3px;
    cursor: pointer;
    transition: background-color 0.3s;
}

.plugin-item button:hover {
    background-color: #555;
}

/* 插件网格样式 */
.plugin-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 20px;
    margin-top: 20px;
}

.plugin-card {
    background-color: #f9f9f9;
    padding: 20px;
    border-radius: 5px;
    text-align: center;
    box-shadow: 0 2px 5px rgba(0,0,0,0.1);
}

.plugin-card h3 {
    margin-bottom: 10px;
}

.plugin-card button {
    margin-top: 15px;
    padding: 8px 15px;
    background-color: #333;
    color: #fff;
    border: none;
    border-radius: 3px;
    cursor: pointer;
    transition: background-color 0.3s;
}

.plugin-card button:hover {
    background-color: #555;
}

/* 字体列表样式 */
.font-list {
    margin-top: 20px;
}

.font-item {
    background-color: #f9f9f9;
    padding: 15px;
    margin-bottom: 10px;
    border-radius: 3px;
    border-left: 4px solid #333;
}

.font-item h3 {
    margin-bottom: 10px;
}

.font-item button {
    margin-right: 10px;
    padding: 5px 10px;
    background-color: #333;
    color: #fff;
    border: none;
    border-radius: 3px;
    cursor: pointer;
    transition: background-color 0.3s;
}

.font-item button:hover {
    background-color: #555;
}

/* 文件列表样式 */
.file-list {
    margin-top: 20px;
}

.file-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    background-color: #f9f9f9;
    padding: 10px;
    margin-bottom: 5px;
    border-radius: 3px;
}

.file-item button {
    padding: 5px 10px;
    background-color: #d9534f;
    color: #fff;
    border: none;
    border-radius: 3px;
    cursor: pointer;
    transition: background-color 0.3s;
}

.file-item button:hover {
    background-color: #c9302c;
}

/* 底部样式 */
footer {
    background-color: #333;
    color: #fff;
    text-align: center;
    padding: 15px;
    margin-top: 20px;
    border-radius: 5px;
}

/* 响应式设计 */
@media (max-width: 768px) {
    nav ul {
        flex-direction: column;
    }
    
    nav ul li {
        text-align: left;
    }
    
    .quick-actions {
        flex-direction: column;
    }
    
    .plugin-grid {
        grid-template-columns: 1fr;
    }
}
)=====";

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
    server.on("/fonts", std::bind(&WebServerManager::handleFonts, this));
    server.on("/tfcard", std::bind(&WebServerManager::handleTFCard, this));
    server.on("/style.css", std::bind(&WebServerManager::handleCSS, this));
    server.on("/factory_reset", std::bind(&WebServerManager::handleFactoryReset, this));
    
    // API路由
    server.on("/api", std::bind(&WebServerManager::handleApi, this));
    server.on("/api/sensor", std::bind(&WebServerManager::handleSensorData, this));
    server.on("/api/plugins", std::bind(&WebServerManager::handlePluginListApi, this));
    server.on("/api/fonts", std::bind(&WebServerManager::handleFontListApi, this));
    server.on("/api/tfcard/status", std::bind(&WebServerManager::handleTFCardStatusApi, this));
    server.on("/api/tfcard/files", std::bind(&WebServerManager::handleTFCardFilesApi, this));
    server.on("/api/tfcard/delete", std::bind(&WebServerManager::handleTFCardDeleteApi, this));
    
    #if defined(ESP32)
    server.on("/upload_font", HTTP_POST, std::bind(&WebServerManager::handleUploadFont, this));
    server.on("/update_settings", HTTP_POST, std::bind(&WebServerManager::handleUpdateSettings, this));
    server.on("/add_plugin", HTTP_POST, std::bind(&WebServerManager::handleAddPlugin, this));
    server.on("/update_plugin", HTTP_POST, std::bind(&WebServerManager::handleUpdatePlugin, this));
    server.on("/delete_plugin", HTTP_POST, std::bind(&WebServerManager::handleDeletePlugin, this));
    server.on("/enable_plugin", HTTP_POST, std::bind(&WebServerManager::handleEnablePlugin, this));
    server.on("/disable_plugin", HTTP_POST, std::bind(&WebServerManager::handleDisablePlugin, this));
    server.on("/api/control", HTTP_GET, std::bind(&WebServerManager::handleRemoteControl, this));
    server.on("/api/control", HTTP_POST, std::bind(&WebServerManager::handleRemoteControl, this));
    server.on("/api/sync", HTTP_GET, std::bind(&WebServerManager::handleDataSync, this));
    server.on("/api/sync", HTTP_POST, std::bind(&WebServerManager::handleDataSync, this));
    server.on("/api/refresh", HTTP_GET, std::bind(&WebServerManager::handleRefreshDisplay, this));
    server.on("/api/refresh", HTTP_POST, std::bind(&WebServerManager::handleRefreshDisplay, this));
    server.on("/api/push", std::bind(&WebServerManager::handleMessagePush, this));
    server.on("/api/message/get", HTTP_GET, std::bind(&WebServerManager::handleMessageGet, this));
    server.on("/api/message/delete", HTTP_POST, std::bind(&WebServerManager::handleMessageDelete, this));
    server.on("/api/message/mark-read", HTTP_POST, std::bind(&WebServerManager::handleMessageMarkRead, this));
    server.on("/api/stock/list", HTTP_GET, std::bind(&WebServerManager::handleStockListApi, this));
    server.on("/api/stock/add", HTTP_POST, std::bind(&WebServerManager::handleStockAddApi, this));
    server.on("/api/stock/delete", HTTP_POST, std::bind(&WebServerManager::handleStockDeleteApi, this));
    server.on("/api/plugin/\d+/enable", std::bind(&WebServerManager::handlePluginEnableApi, this));
    server.on("/api/plugin/\d+/disable", std::bind(&WebServerManager::handlePluginDisableApi, this));
    server.on("/api/plugin/\d+/delete", std::bind(&WebServerManager::handlePluginDeleteApi, this));
    server.on("/api/font/\d+/setdefault", std::bind(&WebServerManager::handleFontSetDefaultApi, this));
    server.on("/api/font/\d+/delete", std::bind(&WebServerManager::handleFontDeleteApi, this));
    #elif defined(ESP8266)
    server.on("/upload_font", std::bind(&WebServerManager::handleUploadFont, this));
    server.on("/update_settings", std::bind(&WebServerManager::handleUpdateSettings, this));
    server.on("/add_plugin", std::bind(&WebServerManager::handleAddPlugin, this));
    server.on("/update_plugin", std::bind(&WebServerManager::handleUpdatePlugin, this));
    server.on("/delete_plugin", std::bind(&WebServerManager::handleDeletePlugin, this));
    server.on("/enable_plugin", std::bind(&WebServerManager::handleEnablePlugin, this));
    server.on("/disable_plugin", std::bind(&WebServerManager::handleDisablePlugin, this));
    server.on("/api/control", std::bind(&WebServerManager::handleRemoteControl, this));
    server.on("/api/sync", std::bind(&WebServerManager::handleDataSync, this));
    server.on("/api/refresh", std::bind(&WebServerManager::handleRefreshDisplay, this));
    server.on("/api/push", std::bind(&WebServerManager::handleMessagePush, this));
    server.on("/api/message/get", HTTP_GET, std::bind(&WebServerManager::handleMessageGet, this));
    server.on("/api/message/delete", HTTP_POST, std::bind(&WebServerManager::handleMessageDelete, this));
    server.on("/api/message/mark-read", HTTP_POST, std::bind(&WebServerManager::handleMessageMarkRead, this));
    server.on("/api/stock/list", HTTP_GET, std::bind(&WebServerManager::handleStockListApi, this));
    server.on("/api/stock/add", HTTP_POST, std::bind(&WebServerManager::handleStockAddApi, this));
    server.on("/api/stock/delete", HTTP_POST, std::bind(&WebServerManager::handleStockDeleteApi, this));
    server.on("/api/plugin/\d+/enable", std::bind(&WebServerManager::handlePluginEnableApi, this));
    server.on("/api/plugin/\d+/disable", std::bind(&WebServerManager::handlePluginDisableApi, this));
    server.on("/api/plugin/\d+/delete", std::bind(&WebServerManager::handlePluginDeleteApi, this));
    server.on("/api/font/\d+/setdefault", std::bind(&WebServerManager::handleFontSetDefaultApi, this));
    server.on("/api/font/\d+/delete", std::bind(&WebServerManager::handleFontDeleteApi, this));
    #endif
    #if defined(ESP32)
    server.on("/api/status", HTTP_GET, std::bind(&WebServerManager::handleDeviceStatus, this));
    #elif defined(ESP8266)
    server.on("/api/status", std::bind(&WebServerManager::handleDeviceStatus, this));
    #endif
    
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
        // MDNS.update() removed as it's not supported in current MDNS implementation
    }
}

void WebServerManager::handleRoot() {
    DEBUG_PRINTLN("处理根路径请求");
    server.send(200, "text/html", index_html);
}

void WebServerManager::handleSettings() {
    DEBUG_PRINTLN("处理设置页面请求");
    server.send(200, "text/html", settings_html);
}

void WebServerManager::handlePlugins() {
    DEBUG_PRINTLN("处理插件管理页面请求");
    server.send(200, "text/html", plugin_html);
}

void WebServerManager::handleUpdateSettings() {
    DEBUG_PRINTLN("处理设置更新请求");
    server.sendHeader("Location", "/settings");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleAddPlugin() {
    DEBUG_PRINTLN("处理添加插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleUpdatePlugin() {
    DEBUG_PRINTLN("处理更新插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleDeletePlugin() {
    DEBUG_PRINTLN("处理删除插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleEnablePlugin() {
    DEBUG_PRINTLN("处理启用插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleDisablePlugin() {
    DEBUG_PRINTLN("处理禁用插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handlePluginList() {
    DEBUG_PRINTLN("处理推荐插件列表请求");
    server.send(200, "text/html", plugin_list_html);
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

void WebServerManager::handleSensorData() {
    DEBUG_PRINTLN("处理传感器数据API请求");
    
    // 创建JSON响应
    static JsonDocument doc;
    doc.clear();
    doc["status"] = "success";
    doc["timestamp"] = platformGetMillis();
    doc["data"]["temperature"] = 23.5;
    doc["data"]["humidity"] = 45.2;
    doc["data"]["motionDetected"] = false;
    doc["data"]["gasLevel"] = 300;
    doc["data"]["flameDetected"] = false;
    doc["data"]["lightLevel"] = 200;
    doc["data"]["valid"] = true;
    
    // 发送响应
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleApi() {
    DEBUG_PRINTLN("处理API根请求");
    
    // 创建JSON响应
    static JsonDocument doc;
    doc.clear();
    doc["status"] = "success";
    doc["name"] = "InkClock API";
    doc["version"] = "1.0";
    doc["description"] = "家用网络智能墨水屏万年历API";
    
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
    return "https://api.qrserver.com/v1/create-qr-code/?size=200x200&data=" + url;
}

void WebServerManager::handleMessagePush() {
    if (server.hasArg("sender") && server.hasArg("content")) {
        String sender = server.arg("sender");
        String content = server.arg("content");
        MessageType type = MESSAGE_TEXT;
        MessagePriority priority = MESSAGE_PRIORITY_NORMAL;
        MessageCategory category = MESSAGE_CATEGORY_GENERAL;
        
        if (server.hasArg("type")) {
            type = (MessageType)server.arg("type").toInt();
        }
        
        if (server.hasArg("priority")) {
            priority = (MessagePriority)server.arg("priority").toInt();
        }
        
        if (server.hasArg("category")) {
            category = (MessageCategory)server.arg("category").toInt();
        }
        
        bool success = messageManager.addMessage(sender, content, type, priority, category);
        
        if (success) {
            server.send(200, "application/json", "{\"success\": true, \"message\": \"Message pushed successfully\"}");
        } else {
            server.send(500, "application/json", "{\"success\": false, \"message\": \"Failed to push message\"}");
        }
    } else {
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Missing required parameters\"}");
    }
}

void WebServerManager::handleDeviceStatus() {
    JsonDocument doc;
    
    doc["status"] = "online";
    doc["ip_address"] = getIPAddress();
    doc["mac_address"] = WiFi.macAddress();
    doc["time"] = "2025-01-01 00:00:00";
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void WebServerManager::handleMessageGet() {
    JsonDocument doc;
    JsonArray messagesArray = doc.createNestedArray("messages");
    
    int messageCount = messageManager.getMessageCount();
    for (int i = 0; i < messageCount; i++) {
        MessageData message = messageManager.getMessage(String(i + 1));
        if (message.valid) {
            JsonObject messageObj = messagesArray.createNestedObject();
            messageObj["id"] = message.id;
            messageObj["sender"] = message.sender;
            messageObj["content"] = message.content;
            messageObj["timestamp"] = message.timestamp;
            messageObj["priority"] = message.priority;
            messageObj["category"] = message.category;
            messageObj["read"] = message.read;
            messageObj["archived"] = message.archived;
        }
    }
    
    doc["total"] = messageCount;
    doc["unread"] = messageManager.getUnreadMessageCount();
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void WebServerManager::handleMessageDelete() {
    if (server.hasArg("id")) {
        String id = server.arg("id");
        bool success = messageManager.deleteMessage(id);
        
        if (success) {
            server.send(200, "application/json", "{\"success\": true, \"message\": \"Message deleted successfully\"}");
        } else {
            server.send(500, "application/json", "{\"success\": false, \"message\": \"Failed to delete message\"}");
        }
    } else {
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Missing required parameter: id\"}");
    }
}

void WebServerManager::handleMessageMarkRead() {
    if (server.hasArg("id")) {
        String id = server.arg("id");
        bool success = messageManager.markMessageAsRead(id);
        
        if (success) {
            server.send(200, "application/json", "{\"success\": true, \"message\": \"Message marked as read\"}");
        } else {
            server.send(500, "application/json", "{\"success\": false, \"message\": \"Failed to mark message as read\"}");
        }
    } else {
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Missing required parameter: id\"}");
    }
}

void WebServerManager::handleStockListApi() {
    JsonDocument doc;
    JsonArray stocksArray = doc.createNestedArray("stocks");
    
    int stockCount = stockManager.getStockCount();
    for (int i = 0; i < stockCount; i++) {
        StockData stock = stockManager.getStockData(i);
        if (stock.valid) {
            JsonObject stockObj = stocksArray.createNestedObject();
            stockObj["code"] = stock.code;
            stockObj["name"] = stock.name;
            stockObj["market"] = stock.market;
            stockObj["price"] = stock.price;
            stockObj["change"] = stock.change;
            stockObj["changePercent"] = stock.changePercent;
            stockObj["open"] = stock.open;
            stockObj["high"] = stock.high;
            stockObj["low"] = stock.low;
            stockObj["close"] = stock.close;
            stockObj["volume"] = stock.volume;
            stockObj["amount"] = stock.amount;
            stockObj["time"] = stock.time;
            
            // 添加曲线数据
            JsonArray chartDataArray = stockObj.createNestedArray("chartData");
            for (int j = 0; j < stock.chartDataCount; j++) {
                JsonObject chartPoint = chartDataArray.createNestedObject();
                chartPoint["price"] = stock.chartData[j].price;
                chartPoint["time"] = stock.chartData[j].time;
            }
        }
    }
    
    doc["total"] = stockCount;
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void WebServerManager::handleStockAddApi() {
    if (server.hasArg("code") && server.hasArg("market")) {
        String code = server.arg("code");
        String market = server.arg("market");
        
        bool success = stockManager.addStock(code, market);
        
        if (success) {
            server.send(200, "application/json", "{\"success\": true, \"message\": \"Stock added successfully\"}");
        } else {
            server.send(500, "application/json", "{\"success\": false, \"message\": \"Failed to add stock\"}");
        }
    } else {
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Missing required parameters: code, market\"}");
    }
}

void WebServerManager::handleStockDeleteApi() {
    if (server.hasArg("index")) {
        int index = server.arg("index").toInt();
        bool success = stockManager.removeStock(index);
        
        if (success) {
            server.send(200, "application/json", "{\"success\": true, \"message\": \"Stock deleted successfully\"}");
        } else {
            server.send(500, "application/json", "{\"success\": false, \"message\": \"Failed to delete stock\"}");
        }
    } else {
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Missing required parameter: index\"}");
    }
}

void WebServerManager::handleFontSetDefaultApi() {
    DEBUG_PRINTLN("处理设置默认字体API请求");
    
    // 从URL中获取字体ID
    String path = server.uri();
    int lastSlashIndex = path.lastIndexOf('/');
    String fontId = path.substring(lastSlashIndex + 1);
    
    // 获取字体列表
    auto fontManager = DependencyInjectionContainer::getInstance()->getFontManager();
    if (fontManager) {
        std::vector<FontInfo> fontList = fontManager->getFontList();
        int index = fontId.toInt() - 1;
        if (index >= 0 && index < fontList.size()) {
            String fontName = fontList[index].name;
            if (fontManager->setCurrentFont(fontName)) {
                server.send(200, "application/json", "{\"success\": true, \"message\": \"字体设置成功\"}");
                return;
            }
        }
    }
    
    server.send(500, "application/json", "{\"success\": false, \"message\": \"设置字体失败\"}");
}

void WebServerManager::handleFontDeleteApi() {
    DEBUG_PRINTLN("处理删除字体API请求");
    
    // 从URL中获取字体ID
    String path = server.uri();
    int lastSlashIndex = path.lastIndexOf('/');
    String fontId = path.substring(lastSlashIndex + 1);
    
    // 获取字体列表
    auto fontManager = DependencyInjectionContainer::getInstance()->getFontManager();
    if (fontManager) {
        std::vector<FontInfo> fontList = fontManager->getFontList();
        int index = fontId.toInt() - 1;
        if (index >= 0 && index < fontList.size()) {
            String fontName = fontList[index].name;
            if (fontManager->removeFont(fontName)) {
                server.send(200, "application/json", "{\"success\": true, \"message\": \"字体删除成功\"}");
                return;
            }
        }
    }
    
    server.send(500, "application/json", "{\"success\": false, \"message\": \"删除字体失败\"}");
}

void WebServerManager::sendJsonResponse(const String& json, int statusCode) {
    server.send(statusCode, "application/json", json);
}

void WebServerManager::handleFonts() {
    DEBUG_PRINTLN("处理字体管理页面请求");
    server.send(200, "text/html", fonts_html);
}

void WebServerManager::handleTFCard() {
    DEBUG_PRINTLN("处理TF卡管理页面请求");
    server.send(200, "text/html", tfcard_html);
}

void WebServerManager::handleUploadFont() {
    DEBUG_PRINTLN("处理字体上传请求");
    
    // 检查是否有文件上传
    if (server.hasArg("font_file")) {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            DEBUG_PRINTF("开始上传字体文件: %s\n", upload.filename.c_str());
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            // 保存上传的字体文件
            if (upload.filename.length() > 0) {
                // 获取字体名称（去掉文件扩展名）
                String fontName = upload.filename;
                int dotIndex = fontName.lastIndexOf('.');
                if (dotIndex != -1) {
                    fontName = fontName.substring(0, dotIndex);
                }
                
                // 使用FontManager上传字体
                auto fontManager = DependencyInjectionContainer::getInstance()->getFontManager();
                if (fontManager) {
                    bool success = fontManager->uploadFont(fontName, (const uint8_t*)upload.buf, upload.currentSize);
                    if (success) {
                        DEBUG_PRINTF("字体上传成功: %s\n", fontName.c_str());
                    } else {
                        DEBUG_PRINTF("字体上传失败: %s\n", fontName.c_str());
                    }
                }
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            DEBUG_PRINTF("字体文件上传完成, 大小: %u bytes\n", upload.totalSize);
        }
    }
    
    server.sendHeader("Location", "/fonts");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleFactoryReset() {
    DEBUG_PRINTLN("处理工厂重置请求");
    server.sendHeader("Location", "/settings");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleRemoteControl() {
    DEBUG_PRINTLN("处理远程控制请求");
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Command executed\"}");
}

void WebServerManager::handleDataSync() {
    DEBUG_PRINTLN("处理数据同步请求");
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Data synced\"}");
}

void WebServerManager::handleRefreshDisplay() {
    DEBUG_PRINTLN("处理显示刷新请求");
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Display refreshed\"}");
}

void WebServerManager::handlePluginListApi() {
    DEBUG_PRINTLN("处理插件列表API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    JsonArray plugins = doc["plugins"].to<JsonArray>();
    
    // 模拟插件数据
    JsonObject plugin1 = plugins.add<JsonObject>();
    plugin1["id"] = "1";
    plugin1["name"] = "天气插件";
    plugin1["version"] = "1.0.0";
    plugin1["description"] = "显示实时天气和天气预报";
    plugin1["enabled"] = true;
    
    JsonObject plugin2 = plugins.add<JsonObject>();
    plugin2["id"] = "2";
    plugin2["name"] = "股票插件";
    plugin2["version"] = "1.0.0";
    plugin2["description"] = "显示股票市场数据";
    plugin2["enabled"] = false;
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleFontListApi() {
    DEBUG_PRINTLN("处理字体列表API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    JsonArray fonts = doc["fonts"].to<JsonArray>();
    
    // 使用FontManager获取真实字体列表
    auto fontManager = DependencyInjectionContainer::getInstance()->getFontManager();
    if (fontManager) {
        std::vector<FontInfo> fontList = fontManager->getFontList();
        for (int i = 0; i < fontList.size(); i++) {
            const FontInfo& font = fontList[i];
            JsonObject fontObj = fonts.add<JsonObject>();
            fontObj["id"] = String(i + 1);
            fontObj["name"] = font.name;
            fontObj["size"] = 0; // 字体大小，实际应用中可以从字体文件获取
            fontObj["isBuiltIn"] = font.isBuiltIn;
            fontObj["isDefault"] = font.isDefault;
        }
    }
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleTFCardStatusApi() {
    DEBUG_PRINTLN("处理TF卡状态API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    doc["exists"] = true;
    doc["total_space"] = 1024;
    doc["free_space"] = 800;
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleTFCardFilesApi() {
    DEBUG_PRINTLN("处理TF卡文件列表API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    JsonArray files = doc["files"].to<JsonArray>();
    
    // 模拟文件数据
    JsonObject file1 = files.add<JsonObject>();
    file1["name"] = "firmware.bin";
    file1["size"] = 512;
    
    JsonObject file2 = files.add<JsonObject>();
    file2["name"] = "config.json";
    file2["size"] = 4;
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleTFCardDeleteApi() {
    DEBUG_PRINTLN("处理TF卡文件删除API请求");
    
    String filename = server.arg("file");
    
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "文件删除成功";
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handlePluginEnableApi() {
    DEBUG_PRINTLN("处理插件启用API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "插件启用成功";
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handlePluginDisableApi() {
    DEBUG_PRINTLN("处理插件禁用API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "插件禁用成功";
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handlePluginDeleteApi() {
    DEBUG_PRINTLN("处理插件删除API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "插件删除成功";
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleFontSetDefaultApi() {
    DEBUG_PRINTLN("处理字体设置默认API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "字体设置成功";
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleFontDeleteApi() {
    DEBUG_PRINTLN("处理字体删除API请求");
    
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "字体删除成功";
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}
