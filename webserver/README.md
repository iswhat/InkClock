# InkClock Web Server

InkClock Web Server是一个为墨水屏时钟设备设计的Web服务端，提供设备管理、消息推送、固件更新、插件管理等功能。

## 项目概述

InkClock Web Server采用现代化的模块化MVC架构，具有良好的可扩展性、可维护性和安全性。系统支持多用户管理，用户可以绑定多个设备，设备通过唯一的设备ID获取消息、插件和OTA信息。

## 系统架构

### 技术栈
- **后端**：PHP 7.0+
- **数据库**：MySQL 5.7+
- **前端**：HTML5 + CSS3 + JavaScript (ES6+)

### 架构设计

系统采用模块化MVC架构，主要包含以下核心组件：

1. **控制器层** (`api/`)
   - 处理HTTP请求
   - 调用业务逻辑
   - 返回统一格式的响应

2. **模型层** (`models/`)
   - 数据访问层
   - 封装数据库操作
   - 业务实体定义

3. **配置层** (`config/`)
   - 集中式配置管理
   - 路由配置
   - 环境变量支持

4. **工具类** (`utils/`)
   - 数据库连接管理
   - 统一响应处理
   - 日志记录

5. **插件系统** (`plugin/`)
   - 支持动态加载插件
   - 插件市场功能

### 核心特性

- ✅ **模块化MVC架构**：清晰的目录结构，便于维护和扩展
- ✅ **RESTful API设计**：遵循RESTful设计原则的API接口
- ✅ **自动路由系统**：基于配置的自动路由，支持参数传递
- ✅ **中间件模式**：统一的身份验证、日志记录和权限检查
- ✅ **单例设计模式**：高效的数据库连接管理
- ✅ **全面的日志和错误处理**：便于调试和监控
- ✅ **CORS处理**：支持跨域请求
- ✅ **集中式配置管理**：便于配置维护
- ✅ **设备唯一ID管理**：设备通过唯一ID获取消息和更新
- ✅ **多用户支持**：用户可以绑定多个设备

## 功能特性

### 1. Web管理界面
- 用户管理页面
- 设备管理页面
- 消息管理页面
- 固件管理页面
- 插件管理页面
- 系统概览仪表盘

### 2. 设备分组和标签
- 设备分组管理
- 设备标签功能
- 按组发送消息
- 按组推送固件

### 3. 消息模板和批量发送
- 消息模板创建和管理
- 批量消息发送
- 定时消息发送

### 4. 固件推送策略
- 基于设备分组的推送
- 按版本号范围推送
- 分批推送
- 定时推送

### 5. 设备数据统计和分析
- 设备在线状态统计
- 固件更新成功率统计
- 消息发送量统计
- 设备使用情况分析

### 6. 插件市场
- 插件上传、审核、发布
- 插件版本管理
- 插件评分和评论

### 7. 通知和告警系统
- 设备离线告警
- 固件更新失败告警
- 系统异常告警
- 多渠道告警通知

### 8. API文档和SDK
- 详细的API文档
- 多语言SDK
- 示例代码和教程

### 9. 安全性增强
- 双因素认证
- API密钥定期轮换
- IP白名单
- 数据加密存储
- 审计日志

### 10. 设备影子功能
- 设备状态同步和持久化
- 设备离线命令缓存
- 设备状态历史记录

### 11. 自动化规则
- 基于条件的自动化规则
- 基于时间的自动化规则
- 设备状态触发规则

## 快速开始

### 环境要求
- PHP 7.0+
- MySQL 5.7+
- Web服务器（Apache/Nginx）

### 安装步骤

1. **克隆项目**
   ```bash
   git clone <repository-url>
   cd InkClock/webserver
   ```

2. **创建数据库**
   ```bash
   mysql -u root -p < init_db.sql
   ```

3. **配置数据库连接**
   修改 `config/config.php` 文件中的数据库配置：
   ```php
   'database' => [
       'host' => 'localhost',
       'port' => 3306,
       'username' => 'root',
       'password' => '',
       'database' => 'inkclock'
   ]
   ```

4. **配置Web服务器**
   - Apache：确保启用了 `mod_rewrite` 模块
   - Nginx：配置伪静态规则

5. **启动服务**
   ```bash
   php -S localhost:8000
   ```

6. **访问系统**
   打开浏览器访问：`http://localhost:8000`

## 目录结构

```
webserver/
├── api/                # API控制器目录
│   ├── BaseController.php
│   ├── DeviceController.php
│   ├── MessageController.php
│   ├── FirmwareController.php
│   └── ...
├── config/            # 配置目录
│   ├── config.php     # 主配置文件
│   └── routes.php     # 路由配置
├── models/            # 数据模型
├── plugin/            # 插件目录
├── utils/             # 工具类
│   ├── Database.php   # 数据库连接管理
│   ├── Response.php   # 统一响应处理
│   └── Logger.php     # 日志系统
├── index.php          # 应用入口
├── index.html         # 前端界面
├── init_db.sql        # 数据库初始化脚本
└── README.md          # 项目说明文档
```

## API文档

### API基础URL
```
http://localhost:8000/api
```

### 主要API端点

#### 用户管理
- `GET /api/user` - 获取用户列表
- `POST /api/user` - 创建用户
- `GET /api/user/{id}` - 获取用户详情
- `PUT /api/user/{id}` - 更新用户
- `DELETE /api/user/{id}` - 删除用户

#### 设备管理
- `GET /api/device` - 获取设备列表
- `POST /api/device` - 注册设备
- `GET /api/device/{deviceId}` - 获取设备详情
- `PUT /api/device/{deviceId}` - 更新设备
- `DELETE /api/device/{deviceId}` - 删除设备

#### 消息管理
- `GET /api/message` - 获取消息列表
- `POST /api/message` - 发送消息
- `GET /api/message/{deviceId}` - 获取设备消息
- `PUT /api/message/{id}` - 更新消息状态

#### 固件管理
- `GET /api/firmware` - 获取固件列表
- `POST /api/firmware` - 上传固件
- `GET /api/firmware/{id}` - 获取固件详情
- `DELETE /api/firmware/{id}` - 删除固件

## 开发指南

### 控制器开发

1. 创建新的控制器类，继承 `BaseController`
2. 在 `config/routes.php` 中配置路由
3. 实现控制器方法

### 模型开发

1. 创建新的模型类
2. 继承或使用 `Database` 类进行数据库操作
3. 实现数据访问方法

### 插件开发

1. 在 `plugin/` 目录下创建插件目录
2. 创建 `index.php` 文件，实现插件功能
3. 创建 `plugin.json` 文件，定义插件元数据

## 部署说明

### 生产环境部署

1. **配置环境变量**
   - 设置 `display_errors = Off`
   - 设置 `log_errors = On`
   - 配置错误日志文件

2. **配置Web服务器**
   - 使用Apache或Nginx作为Web服务器
   - 配置HTTPS
   - 优化服务器性能

3. **数据库优化**
   - 创建索引
   - 优化查询语句
   - 配置数据库连接池

4. **日志管理**
   - 配置日志文件自动旋转
   - 定期清理旧日志

## 贡献指南

1. Fork本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

## 许可证

本项目采用MIT许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

## 更新日志

### 最近更新

#### 架构优化
- 实现了模块化MVC架构
- 引入自动路由系统
- 实现统一的响应处理
- 实现集中式配置管理
- 添加了全面的日志系统

#### 功能增强
- 实现了13个控制器类
- 实现了11个数据模型
- 优化了前端界面和用户体验
- 实现了完整的API端点
- 增强了安全性

#### 文档更新
- 创建了 `FEATURE_GAP_ANALYSIS.md` - 功能差距分析
- 创建了 `CODE_OPTIMIZATION_PLAN.md` - 代码优化计划
- 更新了 `README.md` - 项目说明文档

## 联系方式

- 项目地址：<repository-url>
- 问题反馈：<issue-url>

---

**InkClock Web Server** - 为墨水屏时钟设备提供强大的Web服务端支持
