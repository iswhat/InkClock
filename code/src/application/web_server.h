#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#if defined(ESP32)
#include <WebServer.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WebServer.h>
#define WebServer ESP8266WebServer
#include <ESP8266mDNS.h>
#define ESPmDNS MDNS
#endif
#include "../coresystem/config.h"

// 用户角色枚举
enum UserRole {
    ROLE_ADMIN,    // 管理员 - 所有权限
    ROLE_USER,     // 普通用户 - 有限权限
    ROLE_GUEST     // 访客 - 只读权限
};

// 权限枚举
enum Permission {
    PERMISSION_READ,    // 读取权限
    PERMISSION_WRITE,   // 写入权限
    PERMISSION_ADMIN    // 管理权限
};

class WebServerManager {
private:
    WebServer server;
    bool initialized;
    int port;
    
    // 登录状态管理
    bool isLoggedIn;
    String currentUser;
    UserRole currentUserRole;
    unsigned long lastLoginTime;
    
    // 用户管理
    struct User {
        String username;
        String password;
        UserRole role;
        bool enabled;
    };
    
    // 权限检查
    bool hasPermission(Permission permission);
    UserRole getUserRole(const String& username);
    
    // 用户管理方法
    void handleUserManagement();
    void handleAddUser();
    void handleUpdateUser();
    void handleDeleteUser();
    void handleUserListApi();
    
public:
    static const char* index_html;
  static const char* login_html;
  static const char* settings_html;
  static const char* plugin_html;
  static const char* plugin_list_html;
  static const char* fonts_html;
  static const char* tfcard_html;
  static const char* wifi_config_html;
  static const char* messages_html;
  static const char* stocks_html;
  static const char* style_css;
  
  // 处理函数
  void handleRoot();
  void handleSettings();
  void handlePlugins();
  void handlePluginList();
  void handleFonts();
  void handleTFCard();
  void handleMessages();
  void handleStocks();
  void handleUploadFont();
  void handleUpdateSettings();
  void handleAddPlugin();
  void handleUpdatePlugin();
  void handleDeletePlugin();
  void handleEnablePlugin();
  void handleDisablePlugin();
  void handleNotFound();
  void handleCSS();
  void handleFactoryReset();
  // 传感器数据接口
  void handleSensorData();
  void handleApi();
  // IPv6推送功能API（合并自IPv6Server）
  void handleMessagePush();
  void handleMessageGet();
  void handleMessageDelete();
  void handleMessageMarkRead();
  void handleDeviceStatus();
  // 远程控制API
  void handleRemoteControl();
  void handleDataSync();
  void handleRefreshDisplay();
  // API处理函数
  void handlePluginListApi();
  void handleFontListApi();
  void handleTFCardStatusApi();
  void handleTFCardFilesApi();
  void handleTFCardDeleteApi();
  void handlePluginEnableApi();
  void handlePluginDisableApi();
  void handlePluginDeleteApi();
  void handleFontSetDefaultApi();
  void handleFontDeleteApi();
  // 股票数据API
  void handleStockListApi();
  void handleStockAddApi();
  void handleStockDeleteApi();
  // 登录相关方法
  void handleLogin();
  void handleLogout();
  bool isAuthenticated();
  // 辅助函数
  void sendJsonResponse(const String& json, int statusCode = 200);
  String getCurrentTime();
  
  // 辅助函数
  String getIPAddress();
  String generateQRCodeURL();
  void parseCommandAndParam(String& command, String& param);
  
  // WiFi配置相关方法
  void handleWiFiConfig();
  void handleUpdateWiFiConfig();
  void handleWiFiStatus();
  
  // 用户管理相关方法
  void handleUserManagement();
  void handleAddUser();
  void handleUpdateUser();
  void handleDeleteUser();
  void handleUserListApi();
  void handleForgotPassword();
  
  // 固件更新相关方法
  void handleFirmwareUpdate();
  void handleFirmwareStatus();
  void handleFirmwareRollback();
  
public:
  WebServerManager(int port = 8080);
  ~WebServerManager();
  
  void init();
  void loop();
  
  // 获取Web服务器是否已初始化
  bool isInitialized() { return initialized; }
  // 获取和设置端口
  int getPort() { return port; }
  void setPort(int newPort) { port = newPort; }
};

#endif // WEB_SERVER_H