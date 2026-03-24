# InkClock 项目快速测试指南

## 🎯 测试内容

本指南帮助你快速测试 InkClock 项目的各个功能模块。

---

## 📋 测试方式

### 方式一：自动化测试（推荐）

运行自动化测试脚本，一次性测试所有 API：

```bash
cd d:\InkClock\webserver
php tests/run_tests.php
```

**测试结果说明：**
- ✓ PASS - 测试通过
- ✗ FAIL - 测试失败

### 方式二：人工测试

1. **启动服务器**
```bash
cd d:\InkClock\webserver
php -S localhost:8080 -t public
```

2. **访问各个页面**
- 登录页面：http://localhost:8080/login.html
- 配置管理：http://localhost:8080/config.html
- 仪表盘：http://localhost:8080/dashboard.html
- 固件 UI 预览：http://localhost:8080/firmware-ui-preview.html
- API 文档：http://localhost:8080/api-docs.html

---

## 🔍 固件 UI 预览

### 查看固件嵌入式 UI

访问 **固件 UI 预览页面**：
```
http://localhost:8080/firmware-ui-preview.html
```

这个页面展示了：
- ESP32 固件中嵌入的所有 UI 界面
- 登录页面预览
- 主页功能说明
- 设置页面功能
- WiFi 配置功能
- 源码查看

### 访问真实设备 UI

**首次配网模式：**
1. 设备上电，创建热点 "InkClock_XXXX"
2. 手机/电脑连接该热点
3. 访问 `http://192.168.4.1`

**已配网模式：**
1. 设备连接 WiFi
2. 从路由器获取设备 IP
3. 访问 `http://[设备 IP]`

---

## 📊 当前测试状态

### ✅ 正常工作的功能
- 静态资源访问（HTML 页面）
- 首页加载
- 登录页面加载
- 配置页面加载

### ❌ 需要修复的问题
- API 返回 500 错误（所有 API 端点）
- 数据库连接问题
- 认证逻辑问题

### 🔧 修复建议

1. **检查数据库文件**
```bash
# 确认数据库文件存在
ls -la db/inkclock.db
```

2. **检查 .env 配置**
```bash
# 查看关键配置
cat .env | grep DB_
cat .env | grep APP_ENV
```

3. **查看详细错误日志**
```bash
# 查看应用日志
tail -f logs/app.log

# 查看错误日志
tail -f logs/error.log
```

4. **重启服务器**
```bash
# 停止当前服务器（Ctrl+C）
# 重新启动
php -S localhost:8080 -t public
```

---

## 🎨 UI 对比

### 固件 UI vs 独立 Web 服务器 UI

| 特性 | 固件 UI | 独立 Web 服务器 |
|------|---------|----------------|
| 位置 | ESP32 内部 Flash | 独立 PHP 服务器 |
| 访问方式 | 设备热点/局域网 IP | localhost:8080 |
| 功能 | 基础配置 | 完整管理功能 |
| UI 框架 | 自定义简单样式 | AdminLTE |
| 用户系统 | 单用户 | 多用户支持 |
| 适用场景 | 设备配网、基础设置 | 日常管理、高级功能 |

---

## 📝 测试清单

### 基础功能测试
- [ ] 服务器启动成功
- [ ] 首页可以访问
- [ ] 登录页面可以访问
- [ ] 静态资源（CSS/JS）加载正常

### API 测试（需要修复后）
- [ ] 健康检查 API 正常
- [ ] 用户登录 API 正常
- [ ] 配置管理 API 正常
- [ ] 设备管理 API 正常

### UI 测试
- [ ] 登录页面显示正常
- [ ] 仪表盘显示正常
- [ ] 配置页面显示正常
- [ ] 固件 UI 预览页面显示正常

---

## 🔗 相关链接

- **独立 Web 服务器**: http://localhost:8080
- **固件 UI 预览**: http://localhost:8080/firmware-ui-preview.html
- **API 文档**: http://localhost:8080/api-docs.html
- **配置管理**: http://localhost:8080/config.html

---

## 💡 提示

1. **测试前确保服务器已启动**
2. **如果 API 测试失败，查看日志文件**
3. **固件 UI 需要在真实设备上测试**
4. **独立 Web 服务器用于日常管理更方便**

---

**最后更新**: 2026-03-17
**版本**: 1.0
