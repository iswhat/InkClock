#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <string>
#include <functional>

class NetworkManager {
public:
    static NetworkManager& getInstance();
    
    bool initialize();
    void update();
    void shutdown();
    
    bool isConnected() const;
    std::string getIPAddress() const;
    
    // HTTP服务器相关
    bool startHTTPServer(int port = 80);
    bool stopHTTPServer();
    
    // WebSocket服务器相关
    bool startWebSocketServer(int port = 81);
    bool stopWebSocketServer();
    
    // API注册
    using APIFunc = std::function<std::string(const std::string&)>;
    bool registerAPI(const std::string& path, APIFunc handler);
    
    // 事件回调
    using EventCallback = std::function<void(const std::string&, const std::string&)>;
    void setEventCallback(EventCallback callback);
    
    // 发送消息
    bool sendMessage(const std::string& message);
    bool sendWebSocketMessage(const std::string& message);
    
private:
    NetworkManager();
    ~NetworkManager();
    
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    
    struct Impl;
    Impl* impl;
};

#endif // NETWORK_MANAGER_H
