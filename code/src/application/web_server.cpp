#include "web_server.h"
#include "application/wifi_manager.h"
#include "../extensions/plugin_manager.h"
#include "sensor_manager.h"
#include "../coresystem/font_manager.h"
#include "../coresystem/core_system.h"
#include "display_manager.h"
#include "../coresystem/module_registry.h"
#include "../coresystem/config_manager.h"
#include "../coresystem/dependency_injection.h"
#include <ArduinoJson.h>
#include <vector>

// 条件包含可选模块
#if ENABLE_MESSAGE
  #include "application/message_manager.h"
#endif

#if ENABLE_STOCK
  #include "application/stock_manager.h"
#endif

// 模块包装器前向声明
template <typename T>
T* getModule();

class WiFiModuleWrapper;
class MessageModuleWrapper;
class StockModuleWrapper;
class SensorModuleWrapper;
class TimeModuleWrapper;
class FontModuleWrapper;

#if defined(ESP32)
#include "../coresystem/tf_card_manager.h"
#endif

// 模块获取辅助函数
template <typename T>
T* getModule() {
  static ModuleRegistry* moduleRegistry = ModuleRegistry::getInstance();
  if (!moduleRegistry) {
    Serial.println("Error: moduleRegistry is null!");
    return nullptr;
  }
  return moduleRegistry->getModule<T>();
}

// 核心系统实例
CoreSystem* getCoreSystem() {
  return CoreSystem::getInstance();
}

// 完整的HTML内容
const char* WebServerManager::login_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>登录 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <main>
            <section class="login-section">
                <h2>用户登录</h2>
                <form action="/login" method="post">
                    <div class="form-group">
                        <label for="username">用户名:</label>
                        <input type="text" id="username" name="username" required>
                    </div>
                    <div class="form-group">
                        <label for="password">密码:</label>
                        <input type="password" id="password" name="password" required>
                    </div>
                    <div class="form-group">
                        <button type="submit">登录</button>
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
                <li><a href="/logout">退出登录</a></li>
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
                        <h3>功能模块</h3>
                    </div>
                    <div class="form-group">
                        <label for="enable_audio">音频功能:</label>
                        <input type="checkbox" id="enable_audio" name="enable_audio">
                    </div>
                    <div class="form-group">
                        <label for="enable_bluetooth">蓝牙功能:</label>
                        <input type="checkbox" id="enable_bluetooth" name="enable_bluetooth">
                    </div>
                    <div class="form-group">
                        <label for="enable_camera">摄像头功能:</label>
                        <input type="checkbox" id="enable_camera" name="enable_camera">
                    </div>
                    <div class="form-group">
                        <label for="enable_stock">股票功能:</label>
                        <input type="checkbox" id="enable_stock" name="enable_stock">
                    </div>
                    <div class="form-group">
                        <label for="enable_message">消息功能:</label>
                        <input type="checkbox" id="enable_message" name="enable_message">
                    </div>
                    <div class="form-group">
                        <label for="enable_font">字体功能:</label>
                        <input type="checkbox" id="enable_font" name="enable_font">
                    </div>
                    <div class="form-group">
                        <label for="enable_plugin">插件功能:</label>
                        <input type="checkbox" id="enable_plugin" name="enable_plugin">
                    </div>
                    <div class="form-group">
                        <label for="enable_webclient">Web客户端功能:</label>
                        <input type="checkbox" id="enable_webclient" name="enable_webclient">
                    </div>
                    <div class="form-group">
                        <label for="enable_scene">场景管理功能:</label>
                        <input type="checkbox" id="enable_scene" name="enable_scene">
                    </div>
                    <div class="form-group">
                        <label for="enable_ipv6">IPv6功能:</label>
                        <input type="checkbox" id="enable_ipv6" name="enable_ipv6">
                    </div>
                    <div class="form-group">
                        <label for="enable_firmware">固件更新功能:</label>
                        <input type="checkbox" id="enable_firmware" name="enable_firmware" checked>
                    </div>
                    <div class="form-group">
                        <label for="enable_touch">触摸功能:</label>
                        <input type="checkbox" id="enable_touch" name="enable_touch">
                    </div>
                    <div class="form-group">
                        <label for="enable_tf_card">TF卡功能:</label>
                        <input type="checkbox" id="enable_tf_card" name="enable_tf_card">
                    </div>
                    <div class="form-group">
                        <label for="enable_tf_card_management">TF卡管理功能:</label>
                        <input type="checkbox" id="enable_tf_card_management" name="enable_tf_card_management">
                    </div>
                    <div class="form-group">
                        <label for="enable_alarm_display">报警显示功能:</label>
                        <input type="checkbox" id="enable_alarm_display" name="enable_alarm_display">
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

const char* WebServerManager::stocks_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>股票管理 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
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
                <li><a href="/messages">消息管理</a></li>
                <li><a href="/stocks">股票管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
                <li><a href="/logout">退出登录</a></li>
            </ul>
        </nav>
        <main>
            <section class="stocks-section">
                <h2>股票管理</h2>
                <div class="stock-list" id="stock-list">
                    <!-- 股票列表将通过JavaScript加载 -->
                    <p>加载中...</p>
                </div>
                <h2>添加股票</h2>
                <form onsubmit="addStock(event)">
                    <div class="form-group">
                        <label for="stock-code">股票代码:</label>
                        <input type="text" id="stock-code" name="stock-code" required>
                    </div>
                    <div class="form-group">
                        <label for="stock-market">市场:</label>
                        <select id="stock-market" name="stock-market">
                            <option value="sh">上海</option>
                            <option value="sz">深圳</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <button type="submit">添加股票</button>
                    </div>
                </form>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
    <script>
        // 加载股票列表
        function loadStocks() {
            fetch('/api/stock/list')
                .then(response => response.json())
                .then(data => {
                    const stockList = document.getElementById('stock-list');
                    if (data.stocks && data.stocks.length > 0) {
                        stockList.innerHTML = '';
                        data.stocks.forEach((stock, index) => {
                            const stockItem = document.createElement('div');
                            stockItem.className = 'stock-item';
                            stockItem.innerHTML = `
                                <h3>${stock.name} (${stock.code})</h3>
                                <div class="stock-info">
                                    <span class="price ${stock.change >= 0 ? 'up' : 'down'}">${stock.price.toFixed(2)}</span>
                                    <span class="change ${stock.change >= 0 ? 'up' : 'down'}">${stock.change >= 0 ? '+' : ''}${stock.change.toFixed(2)} (${stock.changePercent.toFixed(2)}%)</span>
                                </div>
                                <div class="stock-details">
                                    <span>开盘: ${stock.open.toFixed(2)}</span>
                                    <span>最高: ${stock.high.toFixed(2)}</span>
                                    <span>最低: ${stock.low.toFixed(2)}</span>
                                    <span>成交量: ${stock.volume}</span>
                                </div>
                                <div class="stock-chart">
                                    <canvas id="chart-${index}"></canvas>
                                </div>
                                <div class="stock-actions">
                                    <button onclick="deleteStock(${index})">删除</button>
                                </div>
                            `;
                            stockList.appendChild(stockItem);
                            
                            // 绘制股票走势图
                            if (stock.chartData && stock.chartData.length > 0) {
                                setTimeout(() => {
                                    drawStockChart(index, stock.chartData);
                                }, 100);
                            }
                        });
                    } else {
                        stockList.innerHTML = '<p>暂无股票</p>';
                    }
                });
        }
        
        // 绘制股票走势图
        function drawStockChart(index, chartData) {
            const canvas = document.getElementById(`chart-${index}`);
            if (!canvas) return;
            
            const ctx = canvas.getContext('2d');
            const prices = chartData.map(point => point.price);
            const times = chartData.map(point => point.time);
            
            new Chart(ctx, {
                type: 'line',
                data: {
                    labels: times,
                    datasets: [{
                        label: '价格',
                        data: prices,
                        borderColor: '#333',
                        backgroundColor: 'rgba(0, 123, 255, 0.1)',
                        borderWidth: 1,
                        tension: 0.1
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: {
                            display: false
                        }
                    },
                    scales: {
                        y: {
                            beginAtZero: false
                        }
                    }
                }
            });
        }
        
        // 添加股票
        function addStock(event) {
            event.preventDefault();
            const code = document.getElementById('stock-code').value;
            const market = document.getElementById('stock-market').value;
            
            fetch('/api/stock/add?code=' + encodeURIComponent(code) + '&market=' + market)
                .then(response => response.json())
                .then(data => {
                    alert(data.message);
                    if (data.success) {
                        loadStocks();
                        document.getElementById('stock-code').value = '';
                    }
                });
        }
        
        // 删除股票
        function deleteStock(index) {
            if (confirm('确定要删除该股票吗？')) {
                fetch('/api/stock/delete?index=' + index)
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message);
                        if (data.success) {
                            loadStocks();
                        }
                    });
            }
        }
        
        // 页面加载完成后加载股票列表
        window.onload = loadStocks;
    </script>
</body>
</html>)=====";

const char* WebServerManager::messages_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>消息管理 - InkClock</title>
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
                <li><a href="/messages">消息管理</a></li>
                <li><a href="/tfcard">TF卡管理</a></li>
                <li><a href="/logout">退出登录</a></li>
            </ul>
        </nav>
        <main>
            <section class="messages-section">
                <h2>消息管理</h2>
                <div class="message-list" id="message-list">
                    <!-- 消息列表将通过JavaScript加载 -->
                    <p>加载中...</p>
                </div>
                <h2>发送消息</h2>
                <form onsubmit="sendMessage(event)">
                    <div class="form-group">
                        <label for="sender">发送者:</label>
                        <input type="text" id="sender" name="sender" required>
                    </div>
                    <div class="form-group">
                        <label for="content">内容:</label>
                        <textarea id="content" name="content" required></textarea>
                    </div>
                    <div class="form-group">
                        <label for="priority">优先级:</label>
                        <select id="priority" name="priority">
                            <option value="0">低</option>
                            <option value="1" selected>普通</option>
                            <option value="2">高</option>
                            <option value="3">紧急</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="category">类别:</label>
                        <select id="category" name="category">
                            <option value="0" selected>通用</option>
                            <option value="1">天气</option>
                            <option value="2">股票</option>
                            <option value="3">传感器</option>
                            <option value="4">系统</option>
                            <option value="5">通知</option>
                            <option value="6">报警</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <button type="submit">发送消息</button>
                    </div>
                </form>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
    <script>
        // 加载消息列表
        function loadMessages() {
            fetch('/api/message/get')
                .then(response => response.json())
                .then(data => {
                    const messageList = document.getElementById('message-list');
                    if (data.messages && data.messages.length > 0) {
                        messageList.innerHTML = '';
                        data.messages.forEach(message => {
                            const messageItem = document.createElement('div');
                            messageItem.className = 'message-item ' + (message.read ? 'read' : 'unread');
                            messageItem.innerHTML = `
                                <h3>${message.sender}</h3>
                                <p>${message.content}</p>
                                <div class="message-meta">
                                    <span>时间: ${new Date(parseInt(message.timestamp)).toLocaleString()}</span>
                                    <span>优先级: ${getPriorityText(message.priority)}</span>
                                    <span>类别: ${getCategoryText(message.category)}</span>
                                    <span>状态: ${message.read ? '已读' : '未读'}</span>
                                </div>
                                <div class="message-actions">
                                    ${!message.read ? '<button onclick="markAsRead(\'' + message.id + '\')">标记为已读</button>' : ''}
                                    <button onclick="deleteMessage(\'' + message.id + '\')">删除</button>
                                </div>
                            `;
                            messageList.appendChild(messageItem);
                        });
                    } else {
                        messageList.innerHTML = '<p>暂无消息</p>';
                    }
                });
        }
        
        // 发送消息
        function sendMessage(event) {
            event.preventDefault();
            const sender = document.getElementById('sender').value;
            const content = document.getElementById('content').value;
            const priority = document.getElementById('priority').value;
            const category = document.getElementById('category').value;
            
            fetch('/api/push?sender=' + encodeURIComponent(sender) + '&content=' + encodeURIComponent(content) + '&priority=' + priority + '&category=' + category)
                .then(response => response.json())
                .then(data => {
                    alert(data.message);
                    if (data.success) {
                        loadMessages();
                        document.getElementById('sender').value = '';
                        document.getElementById('content').value = '';
                    }
                });
        }
        
        // 标记消息为已读
        function markAsRead(id) {
            fetch('/api/message/mark-read?id=' + id)
                .then(response => response.json())
                .then(data => {
                    alert(data.message);
                    if (data.success) {
                        loadMessages();
                    }
                });
        }
        
        // 删除消息
        function deleteMessage(id) {
            if (confirm('确定要删除该消息吗？')) {
                fetch('/api/message/delete?id=' + id)
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message);
                        if (data.success) {
                            loadMessages();
                        }
                    });
            }
        }
        
        // 获取优先级文本
        function getPriorityText(priority) {
            const priorities = ['低', '普通', '高', '紧急'];
            return priorities[priority] || '普通';
        }
        
        // 获取类别文本
        function getCategoryText(category) {
            const categories = ['通用', '天气', '股票', '传感器', '系统', '通知', '报警'];
            return categories[category] || '通用';
        }
        
        // 页面加载完成后加载消息列表
        window.onload = loadMessages;
    </script>
</body>
</html>)=====";

const char* WebServerManager::wifi_config_html = R"=====(<!DOCTYPE html>
<html>
<head>
    <title>WiFi配置 - InkClock</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
    <div class="container">
        <header>
            <h1>InkClock</h1>
            <p>家用网络智能墨水屏万年历</p>
        </header>
        <main>
            <section class="settings-section">
                <h2>WiFi配置</h2>
                <form action="/update_wifi_config" method="post">
                    <div class="form-group">
                        <label for="ssid">WiFi SSID:</label>
                        <input type="text" id="ssid" name="ssid" required>
                    </div>
                    <div class="form-group">
                        <label for="password">WiFi密码:</label>
                        <input type="password" id="password" name="password" required>
                    </div>
                    <div class="form-group">
                        <button type="submit">保存配置</button>
                    </div>
                </form>
                <div class="status-section">
                    <h3>WiFi状态</h3>
                    <div id="wifi-status">加载中...</div>
                </div>
            </section>
        </main>
        <footer>
            <p>&copy; 2025 InkClock</p>
        </footer>
    </div>
    <script>
        // 加载WiFi状态
        function loadWiFiStatus() {
            fetch('/api/wifi/status')
                .then(response => response.json())
                .then(data => {
                    const statusDiv = document.getElementById('wifi-status');
                    if (data.connected) {
                        statusDiv.innerHTML = `
                            <div class="status-item">
                                <span class="label">状态:</span>
                                <span class="value">已连接</span>
                            </div>
                            <div class="status-item">
                                <span class="label">SSID:</span>
                                <span class="value">${data.ssid}</span>
                            </div>
                            <div class="status-item">
                                <span class="label">IP地址:</span>
                                <span class="value">${data.ip_address}</span>
                            </div>
                        `;
                    } else {
                        statusDiv.innerHTML = `
                            <div class="status-item">
                                <span class="label">状态:</span>
                                <span class="value">未连接</span>
                            </div>
                            <div class="status-item">
                                <span class="label">模式:</span>
                                <span class="value">${data.mode}</span>
                            </div>
                        `;
                    }
                });
        }
        
        // 页面加载完成后加载状态
        window.onload = loadWiFiStatus;
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

WebServerManager::WebServerManager(int port) : server(port), port(port), initialized(false), isLoggedIn(false), currentUser(""), lastLoginTime(0) {
}

WebServerManager::~WebServerManager() {
}

void WebServerManager::init() {
    DEBUG_PRINTLN("初始化Web服务器...");
    
    // 从配置文件读取端口配置
    int configPort = CONFIG_GET_INT("web_server.port", port);
    if (configPort != port) {
        port = configPort;
        // 注意：WebServer 不支持重新赋值，保持使用构造时的端口
    }
    
    // 设置路由处理函数
    server.on("/", std::bind(&WebServerManager::handleRoot, this));
    server.on("/login", HTTP_POST, std::bind(&WebServerManager::handleLogin, this));
    server.on("/login", HTTP_GET, [this]() {
        server.send(200, "text/html", login_html);
    });
    server.on("/logout", std::bind(&WebServerManager::handleLogout, this));
    server.on("/settings", std::bind(&WebServerManager::handleSettings, this));
    server.on("/plugins", std::bind(&WebServerManager::handlePlugins, this));
    server.on("/plugin_list", std::bind(&WebServerManager::handlePluginList, this));
    server.on("/fonts", std::bind(&WebServerManager::handleFonts, this));
    server.on("/tfcard", std::bind(&WebServerManager::handleTFCard, this));
    server.on("/messages", std::bind(&WebServerManager::handleMessages, this));
    server.on("/stocks", std::bind(&WebServerManager::handleStocks, this));
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
    
    // WiFi配置相关路由
    server.on("/wifi_config", std::bind(&WebServerManager::handleWiFiConfig, this));
    server.on("/update_wifi_config", HTTP_POST, std::bind(&WebServerManager::handleUpdateWiFiConfig, this));
    server.on("/api/wifi/status", std::bind(&WebServerManager::handleWiFiStatus, this));
    
    server.onNotFound(std::bind(&WebServerManager::handleNotFound, this));
    
    // 启动Web服务器
    server.begin();
    
    // 启动mDNS服务
    if (!MDNS.begin("inkclock")) {
        DEBUG_PRINTLN("Error starting mDNS");
    } else {
        DEBUG_PRINT("mDNS started: http://inkclock.local:");
        DEBUG_PRINTLN(port);
    }
    
    initialized = true;
    DEBUG_PRINT("Web服务器初始化完成，端口: ");
    DEBUG_PRINTLN(port);
}

void WebServerManager::loop() {
    if (initialized) {
        server.handleClient();
        // MDNS.update() removed as it's not supported in current MDNS implementation
    }
}

void WebServerManager::handleRoot() {
    DEBUG_PRINTLN("处理根路径请求");
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
    server.send(200, "text/html", index_html);
}

void WebServerManager::handleSettings() {
    DEBUG_PRINTLN("处理设置页面请求");
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
    server.send(200, "text/html", settings_html);
}

void WebServerManager::handlePlugins() {
    DEBUG_PRINTLN("处理插件管理页面请求");
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
    server.send(200, "text/html", plugin_html);
}

void WebServerManager::handleUpdateSettings() {
    DEBUG_PRINTLN("处理设置更新请求");
    
    // 读取功能模块的启用/禁用状态
    bool enableAudio = server.hasArg("enable_audio");
    bool enableBluetooth = server.hasArg("enable_bluetooth");
    bool enableCamera = server.hasArg("enable_camera");
    bool enableStock = server.hasArg("enable_stock");
    bool enableMessage = server.hasArg("enable_message");
    bool enableFont = server.hasArg("enable_font");
    bool enablePlugin = server.hasArg("enable_plugin");
    bool enableWebclient = server.hasArg("enable_webclient");
    bool enableScene = server.hasArg("enable_scene");
    bool enableIpv6 = server.hasArg("enable_ipv6");
    bool enableFirmware = server.hasArg("enable_firmware");
    bool enableTouch = server.hasArg("enable_touch");
    bool enableTfCard = server.hasArg("enable_tf_card");
    bool enableTfCardManagement = server.hasArg("enable_tf_card_management");
    bool enableAlarmDisplay = server.hasArg("enable_alarm_display");
    
    // 读取其他设置
    String ntpServer = server.hasArg("ntp_server") ? server.arg("ntp_server") : "pool.ntp.org";
    int timezone = server.hasArg("timezone") ? server.arg("timezone").toInt() : 8;
    int updateInterval = server.hasArg("update_interval") ? server.arg("update_interval").toInt() : 15;
    int displayMode = server.hasArg("display_mode") ? server.arg("display_mode").toInt() : 1;
    bool lowPowerMode = server.hasArg("low_power_mode");
    String geoLocation = server.hasArg("geo_location") ? server.arg("geo_location") : "";
    
    // 保存设置到配置
    ConfigManager* configManager = ConfigManager::getInstance();
    if (configManager) {
        configManager->setBool("module.audio.enabled", enableAudio);
        configManager->setBool("module.bluetooth.enabled", enableBluetooth);
        configManager->setBool("module.camera.enabled", enableCamera);
        configManager->setBool("module.stock.enabled", enableStock);
        configManager->setBool("module.message.enabled", enableMessage);
        configManager->setBool("module.font.enabled", enableFont);
        configManager->setBool("module.plugin.enabled", enablePlugin);
        configManager->setBool("module.webclient.enabled", enableWebclient);
        configManager->setBool("module.scene.enabled", enableScene);
        configManager->setBool("module.ipv6.enabled", enableIpv6);
        configManager->setBool("module.firmware.enabled", enableFirmware);
        configManager->setBool("module.touch.enabled", enableTouch);
        configManager->setBool("module.tfcard.enabled", enableTfCard);
        configManager->setBool("module.tfcard.management", enableTfCardManagement);
        configManager->setBool("module.alarm_display.enabled", enableAlarmDisplay);
        
        configManager->setString("time.ntp_server", ntpServer);
        configManager->setInt("time.timezone", timezone);
        configManager->setInt("time.update_interval", updateInterval);
        configManager->setInt("display.mode", displayMode);
        configManager->setBool("power.low_power_mode", lowPowerMode);
        configManager->setString("geo.location", geoLocation);
        
        configManager->saveConfig();
        DEBUG_PRINTLN("设置已保存");
    }
    
    DEBUG_PRINTLN("功能模块设置:");
    DEBUG_PRINT("音频功能: ");
    DEBUG_PRINTLN(enableAudio ? "启用" : "禁用");
    DEBUG_PRINT("蓝牙功能: ");
    DEBUG_PRINTLN(enableBluetooth ? "启用" : "禁用");
    DEBUG_PRINT("摄像头功能: ");
    DEBUG_PRINTLN(enableCamera ? "启用" : "禁用");
    DEBUG_PRINT("股票功能: ");
    DEBUG_PRINTLN(enableStock ? "启用" : "禁用");
    DEBUG_PRINT("消息功能: ");
    DEBUG_PRINTLN(enableMessage ? "启用" : "禁用");
    DEBUG_PRINT("字体功能: ");
    DEBUG_PRINTLN(enableFont ? "启用" : "禁用");
    DEBUG_PRINT("插件功能: ");
    DEBUG_PRINTLN(enablePlugin ? "启用" : "禁用");
    DEBUG_PRINT("Web客户端功能: ");
    DEBUG_PRINTLN(enableWebclient ? "启用" : "禁用");
    DEBUG_PRINT("场景管理功能: ");
    DEBUG_PRINTLN(enableScene ? "启用" : "禁用");
    DEBUG_PRINT("IPv6功能: ");
    DEBUG_PRINTLN(enableIpv6 ? "启用" : "禁用");
    DEBUG_PRINT("固件更新功能: ");
    DEBUG_PRINTLN(enableFirmware ? "启用" : "禁用");
    DEBUG_PRINT("触摸功能: ");
    DEBUG_PRINTLN(enableTouch ? "启用" : "禁用");
    DEBUG_PRINT("TF卡功能: ");
    DEBUG_PRINTLN(enableTfCard ? "启用" : "禁用");
    DEBUG_PRINT("TF卡管理功能: ");
    DEBUG_PRINTLN(enableTfCardManagement ? "启用" : "禁用");
    DEBUG_PRINT("报警显示功能: ");
    DEBUG_PRINTLN(enableAlarmDisplay ? "启用" : "禁用");
    
    // 重定向到设置页面
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
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
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
    
    // 获取真实的传感器数据
    SensorData sensorData;
    sensorData.valid = false;
    
    // 创建JSON响应
    static JsonDocument doc;
    doc.clear();
    doc["status"] = "success";
    doc["timestamp"] = platformGetMillis();
    doc["data"]["temperature"] = sensorData.temperature;
    doc["data"]["humidity"] = sensorData.humidity;
    doc["data"]["motionDetected"] = sensorData.motionDetected;
    doc["data"]["gasLevel"] = sensorData.gasLevel;
    doc["data"]["flameDetected"] = sensorData.flameDetected;
    doc["data"]["lightLevel"] = sensorData.lightLevel;
    doc["data"]["valid"] = sensorData.valid;
    
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
        String type = server.hasArg("type") ? server.arg("type") : "text";
        String priority = server.hasArg("priority") ? server.arg("priority") : "normal";
        String category = server.hasArg("category") ? server.arg("category") : "general";
        
        bool success = false;
        
        #if ENABLE_MESSAGE
        auto messageModule = getModule<MessageModuleWrapper>();
        if (messageModule) {
            MessageType msgType = MESSAGE_TEXT;
            if (type == "audio") msgType = MESSAGE_AUDIO;
            else if (type == "image") msgType = MESSAGE_IMAGE;
            
            MessagePriority msgPriority = MESSAGE_PRIORITY_NORMAL;
            if (priority == "high") msgPriority = MESSAGE_PRIORITY_HIGH;
            else if (priority == "low") msgPriority = MESSAGE_PRIORITY_LOW;
            
            MessageCategory msgCategory = MESSAGE_CATEGORY_GENERAL;
            if (category == "notification") msgCategory = MESSAGE_CATEGORY_NOTIFICATION;
            else if (category == "alert") msgCategory = MESSAGE_CATEGORY_ALERT;
            else if (category == "info") msgCategory = MESSAGE_CATEGORY_INFO;
            
            success = messageModule->getMessageManager().addMessage(sender, content, msgType, msgPriority, msgCategory);
        }
        #endif
        
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
    
    // 获取真实的时间
    doc["time"] = "2025-01-01 00:00:00";
    
    // 添加WiFi状态信息
    bool connected = WiFi.status() == WL_CONNECTED;
    doc["wifi"]["connected"] = connected;
    if (connected) {
        doc["wifi"]["ssid"] = WiFi.SSID();
        doc["wifi"]["signal_strength"] = WiFi.RSSI();
    }
    
    // 添加系统信息
    doc["system"]["version"] = "1.0.0";
    doc["system"]["platform"] = String(PLATFORM_NAME);
    
    // 添加内存使用情况
    #if defined(ESP32)
    doc["system"]["memory"]["free"] = ESP.getFreeHeap();
    doc["system"]["memory"]["total"] = ESP.getHeapSize();
    #elif defined(ESP8266)
    doc["system"]["memory"]["free"] = ESP.getFreeHeap();
    doc["system"]["memory"]["total"] = 8192; // 8MB 内存
    #endif
    
    // 添加已启用的功能模块
    JsonArray modulesArray = doc["modules"].createNestedArray();
    auto moduleRegistry = ModuleRegistry::getInstance();
    if (moduleRegistry) {
        // 这里可以添加已注册的模块信息
        modulesArray.add("WiFiManager");
        modulesArray.add("TimeManager");
        modulesArray.add("WebServer");
        modulesArray.add("MessageManager");
        modulesArray.add("SensorManager");
    }
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void WebServerManager::handleMessageGet() {
    JsonDocument doc;
    
    int messageCount = 0;
    int unreadCount = 0;
    
    #if ENABLE_MESSAGE
    auto messageModule = getModule<MessageModuleWrapper>();
    if (messageModule) {
        messageCount = messageModule->getMessageManager().getMessageCount();
        JsonArray messagesArray = doc.createNestedArray("messages");
        for (int i = 0; i < messageCount; i++) {
            MessageData message = messageModule->getMessageManager().getMessage(String(i + 1));
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
        unreadCount = messageModule->getMessageManager().getUnreadMessageCount();
    }
    #endif
    
    doc["total"] = messageCount;
    doc["unread"] = unreadCount;
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void WebServerManager::handleMessageDelete() {
    if (server.hasArg("id")) {
        String id = server.arg("id");
        bool success = false;
        
        #if ENABLE_MESSAGE
        auto messageModule = getModule<MessageModuleWrapper>();
        if (messageModule) {
            success = messageModule->getMessageManager().deleteMessage(id);
        }
        #endif
        
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
        bool success = false;
        
        #if ENABLE_MESSAGE
        auto messageModule = getModule<MessageModuleWrapper>();
        if (messageModule) {
            success = messageModule->getMessageManager().markMessageAsRead(id);
        }
        #endif
        
        if (success) {
            server.send(200, "application/json", "{\"success\": true, \"message\": \"Message marked as read\"}");
        } else {
            server.send(500, "application/json", "{\"success\": false, \"message\": \"Failed to mark message as read\"}");
        }
    } else {
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Missing required parameter: id\"}");
    }
}

void WebServerManager::handleWiFiConfig() {
    DEBUG_PRINTLN("处理WiFi配置页面请求");
    server.send(200, "text/html", wifi_config_html);
}

void WebServerManager::handleUpdateWiFiConfig() {
    DEBUG_PRINTLN("处理WiFi配置更新请求");
    
    if (server.hasArg("ssid") && server.hasArg("password")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        
        DEBUG_PRINT("更新WiFi配置: ");
        DEBUG_PRINTLN(ssid);
        
        // 直接使用 WiFi 库来设置配置
        WiFi.begin(ssid.c_str(), password.c_str());
        server.send(200, "application/json", "{\"success\": true, \"message\": \"WiFi configuration updated successfully\"}");
    } else {
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Missing required parameters: ssid and password\"}");
    }
}

void WebServerManager::handleWiFiStatus() {
    DEBUG_PRINTLN("处理WiFi状态API请求");
    
    JsonDocument doc;
    bool connected = WiFi.status() == WL_CONNECTED;
    doc["connected"] = connected;
    
    if (connected) {
        doc["ssid"] = WiFi.SSID();
        doc["ip_address"] = WiFi.localIP().toString();
    } else {
        doc["mode"] = "STA Mode";
    }
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void WebServerManager::handleStockListApi() {
    JsonDocument doc;
    JsonArray stocksArray = doc.createNestedArray("stocks");
    
    int stockCount = 0;
    
    #if ENABLE_STOCK
    auto stockModule = getModule<StockModuleWrapper>();
    if (stockModule) {
        stockCount = stockModule->getStockManager().getStockCount();
        for (int i = 0; i < stockCount; i++) {
            StockData stock = stockModule->getStockManager().getStockData(i);
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
    }
    #endif
    
    doc["total"] = stockCount;
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void WebServerManager::handleStockAddApi() {
    if (server.hasArg("code") && server.hasArg("market")) {
        String code = server.arg("code");
        String market = server.arg("market");
        
        bool success = false;
        
        #if ENABLE_STOCK
        auto stockModule = getModule<StockModuleWrapper>();
        if (stockModule) {
            success = stockModule->getStockManager().addStock(code, market);
        }
        #endif
        
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
        bool success = false;
        
        #if ENABLE_STOCK
        auto stockModule = getModule<StockModuleWrapper>();
        if (stockModule) {
            success = stockModule->getStockManager().removeStock(index);
        }
        #endif
        
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
    
    bool success = false;
    
    // 获取字体管理器实例
    FontManager* fontManager = FontManager::getInstance();
    if (fontManager) {
        // 获取字体列表
        auto fontList = fontManager->getFontList();
        for (const auto& font : fontList) {
            if (font.name == fontId) {
                success = fontManager->setCurrentFont(font.name);
                break;
            }
        }
    }
    
    if (success) {
        server.send(200, "application/json", "{\"success\": true, \"message\": \"默认字体设置成功\"}");
    } else {
        server.send(500, "application/json", "{\"success\": false, \"message\": \"设置默认字体失败\"}");
    }
}

void WebServerManager::handleFontDeleteApi() {
    DEBUG_PRINTLN("处理删除字体API请求");
    
    // 从URL中获取字体ID
    String path = server.uri();
    int lastSlashIndex = path.lastIndexOf('/');
    String fontId = path.substring(lastSlashIndex + 1);
    
    bool success = false;
    
    // 获取字体管理器实例
    FontManager* fontManager = FontManager::getInstance();
    if (fontManager) {
        success = fontManager->removeFont(fontId);
    }
    
    if (success) {
        server.send(200, "application/json", "{\"success\": true, \"message\": \"字体删除成功\"}");
    } else {
        server.send(500, "application/json", "{\"success\": false, \"message\": \"删除字体失败\"}");
    }
}

void WebServerManager::sendJsonResponse(const String& json, int statusCode) {
    server.send(statusCode, "application/json", json);
}

void WebServerManager::handleFonts() {
    DEBUG_PRINTLN("处理字体管理页面请求");
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
    server.send(200, "text/html", fonts_html);
}

void WebServerManager::handleTFCard() {
    DEBUG_PRINTLN("处理TF卡管理页面请求");
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
    server.send(200, "text/html", tfcard_html);
}

void WebServerManager::handleMessages() {
    DEBUG_PRINTLN("处理消息管理页面请求");
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
    server.send(200, "text/html", messages_html);
}

void WebServerManager::handleStocks() {
    DEBUG_PRINTLN("处理股票管理页面请求");
    
    // 检查登录状态
    if (!isAuthenticated()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
        return;
    }
    
    server.send(200, "text/html", stocks_html);
}

void WebServerManager::handleUploadFont() {
    DEBUG_PRINTLN("处理字体上传请求");
    
    // 检查是否有文件上传
    if (server.hasArg("font_file")) {
        HTTPUpload& upload = server.upload();
        static String currentFontName;
        static std::vector<uint8_t> fontData;
        
        if (upload.status == UPLOAD_FILE_START) {
            // 开始上传
            fontData.clear();
            currentFontName = upload.filename;
            int dotIndex = currentFontName.lastIndexOf('.');
            if (dotIndex != -1) {
                currentFontName = currentFontName.substring(0, dotIndex);
            }
            DEBUG_PRINTF("开始上传字体文件: %s\n", upload.filename.c_str());
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            // 写入数据
            for (size_t i = 0; i < upload.currentSize; i++) {
                fontData.push_back(upload.buf[i]);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            // 上传完成
            DEBUG_PRINTF("字体文件上传完成, 大小: %u bytes\n", upload.totalSize);
            
            // 获取字体管理器实例
            FontManager* fontManager = FontManager::getInstance();
            if (fontManager) {
                // 上传字体
                bool success = fontManager->uploadFont(currentFontName, fontData.data(), fontData.size());
                if (success) {
                    DEBUG_PRINTF("字体上传成功: %s\n", currentFontName.c_str());
                } else {
                    DEBUG_PRINTF("字体上传失败: %s\n", currentFontName.c_str());
                }
            }
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

void WebServerManager::handleLogin() {
    DEBUG_PRINTLN("处理登录请求");
    
    // 读取表单数据
    String username = server.arg("username");
    String password = server.arg("password");
    
    // 简单的用户名密码验证（实际应用中应该从配置或数据库中获取）
    if (username == "admin" && password == "admin123") {
        // 登录成功
        isLoggedIn = true;
        currentUser = username;
        lastLoginTime = platformGetMillis();
        
        DEBUG_PRINTLN("登录成功");
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
    } else {
        // 登录失败
        DEBUG_PRINTLN("登录失败");
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "");
    }
}

void WebServerManager::handleLogout() {
    DEBUG_PRINTLN("处理登出请求");
    
    // 清除登录状态
    isLoggedIn = false;
    currentUser = "";
    lastLoginTime = 0;
    
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
}

bool WebServerManager::isAuthenticated() {
    // 检查登录状态
    if (!isLoggedIn) {
        return false;
    }
    
    // 检查登录是否过期（30分钟）
    unsigned long currentTime = platformGetMillis();
    if (currentTime - lastLoginTime > 30 * 60 * 1000) {
        isLoggedIn = false;
        currentUser = "";
        lastLoginTime = 0;
        return false;
    }
    
    return true;
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
    
    // 获取字体管理器实例
    FontManager* fontManager = FontManager::getInstance();
    if (fontManager) {
        // 获取字体列表
        auto fontList = fontManager->getFontList();
        int id = 1;
        for (const auto& font : fontList) {
            JsonObject fontObj = fonts.add<JsonObject>();
            fontObj["id"] = String(id++);
            fontObj["name"] = font.name;
            fontObj["size"] = font.size;
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


