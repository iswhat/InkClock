# InkClock Webserver 测试指南

## 🚀 快速测试

### 1. 启动服务器
```bash
cd d:\InkClock\webserver
php -S localhost:8080 -t public
```

### 2. 访问页面
- **登录页面**: http://localhost:8080/login.html
- **配置管理**: http://localhost:8080/config.html
- **仪表盘**: http://localhost:8080/dashboard.html
- **API 文档**: http://localhost:8080/api-docs.html

### 3. 运行自动化测试
```bash
php tests/run_tests.php
```

---

## 📱 固件前端 UI 预览

### 方法一：查看嵌入式 Web 服务器代码
ESP32 固件的 Web 界面代码在：
- **主文件**: `d:\InkClock\code\src\application\web_server.cpp`
- 所有 HTML/CSS 都嵌入在 C++ 代码中

### 方法二：使用 Web 服务器前端（推荐）
独立 Web 服务器的前端更现代化，功能更完整：
- 位于：`d:\InkClock\webserver\public\`
- 使用 AdminLTE 模板
- 支持 PWA

### 主要页面说明：

#### 1. 登录页面 (login.html)
- 完整的用户认证
- 支持记住密码
- PWA 离线支持

#### 2. 仪表盘 (dashboard.html)
- 设备状态概览
- 实时统计信息
- 快捷操作入口

#### 3. 设备管理 (devices.html)
- 设备列表
- 设备配置
- 固件升级

#### 4. 消息管理 (messages.html)
- 消息推送
- 消息历史
- 定时消息

#### 5. 插件管理 (plugins.html)
- 插件安装/卸载
- 插件配置
- 插件市场

#### 6. 系统设置 (settings.html)
- 系统配置
- 网络设置
- 显示设置

#### 7. 配置管理 (config.html)
- 环境变量管理
- 数据库配置
- 日志配置

---

## 🔧 测试功能清单

### ✅ 用户认证
- [ ] 用户登录
- [ ] 用户注册
- [ ] 密码重置
- [ ] API Key 认证

### ✅ 设备管理
- [ ] 添加设备
- [ ] 编辑设备
- [ ] 删除设备
- [ ] 设备分组
- [ ] 设备标签

### ✅ 消息推送
- [ ] 发送消息
- [ ] 消息历史
- [ ] 消息已读/未读
- [ ] 消息删除

### ✅ 固件管理
- [ ] 上传固件
- [ ] 固件版本管理
- [ ] OTA 升级
- [ ] 固件推送任务

### ✅ 插件系统
- [ ] 插件安装
- [ ] 插件启用/禁用
- [ ] 插件配置
- [ ] 插件卸载

### ✅ 系统管理
- [ ] 系统信息查看
- [ ] 日志查看
- [ ] 数据库备份
- [ ] 系统重启

---

## 🐛 常见问题排查

### 1. 服务器无法启动
```bash
# 检查 PHP 版本
php -v

# 检查端口占用
netstat -ano | findstr :8080
```

### 2. API 返回 500 错误
- 检查 `logs/error.log`
- 检查 `.env` 配置
- 检查数据库连接

### 3. 页面无法访问
- 确认服务器已启动
- 检查防火墙设置
- 清除浏览器缓存

---

## 📊 性能测试

### 使用 Apache Bench (ab)
```bash
# 测试登录 API
ab -n 1000 -c 10 -p login_data.json -T application/json http://localhost:8080/api/user/login

# 测试配置 API
ab -n 1000 -c 10 http://localhost:8080/api/config
```

---

## 📝 测试数据

### 默认管理员账号
- 用户名：`admin`
- 密码：`admin123`

### 测试用户账号
- 用户名：`testuser`
- 密码：`TestUser123!`

---

## 🔗 相关链接

- **API 文档**: http://localhost:8080/api-docs.html
- **Swagger UI**: http://localhost:8080/api-docs.json
- **GitHub**: https://github.com/your-repo/InkClock
