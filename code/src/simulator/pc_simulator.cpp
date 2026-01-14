#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>

// 基础类型定义
typedef unsigned char byte;
typedef unsigned short word;

// 简单的String类实现
class String {
private:
    std::string _str;

public:
    String() : _str() {}
    String(const char* cstr) : _str(cstr) {}
    String(const std::string& str) : _str(str) {}
    String(int value) : _str(std::to_string(value)) {}
    String(long value) : _str(std::to_string(value)) {}
    String(float value) : _str(std::to_string(value)) {}
    String(double value) : _str(std::to_string(value)) {}

    // 操作符重载
    String& operator=(const char* cstr) {
        _str = cstr;
        return *this;
    }

    String& operator=(const std::string& str) {
        _str = str;
        return *this;
    }

    String& operator+=(const char* cstr) {
        _str += cstr;
        return *this;
    }

    String& operator+=(const String& other) {
        _str += other._str;
        return *this;
    }

    String& operator+=(char c) {
        _str += c;
        return *this;
    }

    String& operator+=(int value) {
        _str += std::to_string(value);
        return *this;
    }

    // 方法
    const char* c_str() const {
        return _str.c_str();
    }

    size_t length() const {
        return _str.length();
    }

    bool isEmpty() const {
        return _str.empty();
    }

    String substring(size_t beginIndex) {
        if (beginIndex >= _str.length()) {
            return String();
        }
        return String(_str.substr(beginIndex));
    }

    String substring(size_t beginIndex, size_t endIndex) {
        if (beginIndex >= _str.length()) {
            return String();
        }
        endIndex = std::min(endIndex, _str.length());
        return String(_str.substr(beginIndex, endIndex - beginIndex));
    }

    int toInt() {
        try {
            return std::stoi(_str);
        } catch (...) {
            return 0;
        }
    }

    float toFloat() {
        try {
            return std::stof(_str);
        } catch (...) {
            return 0.0f;
        }
    }

    // 转换为std::string
    std::string toStdString() {
        return _str;
    }

    // 比较操作符
    bool operator==(const char* cstr) {
        return _str == cstr;
    }

    bool operator!=(const char* cstr) {
        return _str != cstr;
    }

    bool operator==(const String& other) {
        return _str == other._str;
    }

    bool operator!=(const String& other) {
        return _str != other._str;
    }
};

// 运算符重载：String + char
String operator+(const String& lhs, char rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：String + const char*
String operator+(const String& lhs, const char* rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：String + String
String operator+(const String& lhs, const String& rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：const char* + String
String operator+(const char* lhs, const String& rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：char + String
String operator+(char lhs, const String& rhs) {
    String result;
    result += lhs;
    result += rhs;
    return result;
}

// 基本函数
void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

unsigned long millis() {
    static auto start_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return duration.count();
}

unsigned long micros() {
    static auto start_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
    return duration.count();
}

void yield() {
    // 空实现，用于兼容Arduino
}

// 随机数函数
int random(int max) {
    return rand() % max;
}

int random(int min, int max) {
    return min + rand() % (max - min);
}

void randomSeed(unsigned long seed) {
    srand(seed);
}

// 串口类
class Serial_ {
public:
    void begin(unsigned long baud) {
        // 空实现
    }

    void end() {
        // 空实现
    }

    size_t print(const char* str) {
        std::cout << str;
        return std::string(str).length();
    }

    size_t print(const String& str) {
        std::cout << str.c_str();
        return str.length();
    }

    size_t print(int value, int base = 10) {
        std::cout << value;
        return std::to_string(value).length();
    }

    size_t print(long value, int base = 10) {
        std::cout << value;
        return std::to_string(value).length();
    }

    size_t print(float value, int digits = 2) {
        std::cout.precision(digits);
        std::cout << value;
        return std::to_string(value).length();
    }

    size_t println(const char* str) {
        std::cout << str << std::endl;
        return std::string(str).length() + 1;
    }

    size_t println(const String& str) {
        std::cout << str.c_str() << std::endl;
        return str.length() + 1;
    }

    size_t println(int value, int base = 10) {
        std::cout << value << std::endl;
        return std::to_string(value).length() + 1;
    }

    size_t println(long value, int base = 10) {
        std::cout << value << std::endl;
        return std::to_string(value).length() + 1;
    }

    size_t println(float value, int digits = 2) {
        std::cout.precision(digits);
        std::cout << value << std::endl;
        return std::to_string(value).length() + 1;
    }

    size_t println() {
        std::cout << std::endl;
        return 1;
    }

    bool available() {
        return false;
    }

    int read() {
        return -1;
    }
};

// 全局串口对象
Serial_ Serial;

// 显示驱动接口
class IDisplayDriver {
public:
    virtual ~IDisplayDriver() = default;
    virtual bool init() = 0;
    virtual void clear() = 0;
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) = 0;
    virtual void drawString(int16_t x, int16_t y, const String& text, uint16_t color, uint16_t bg, uint8_t size) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) = 0;
    virtual void update() = 0;
    virtual void update(int16_t x, int16_t y, int16_t w, int16_t h) = 0;
    virtual int16_t getWidth() const = 0;
    virtual int16_t getHeight() const = 0;
    virtual int16_t measureTextWidth(const String& text, uint8_t size) const = 0;
    virtual int16_t measureTextHeight(const String& text, uint8_t size) const = 0;
    virtual void sleep() = 0;
    virtual void wakeup() = 0;
};

// 模拟显示驱动
class SimulatorDisplay : public IDisplayDriver {
private:
    int16_t width;
    int16_t height;
    uint16_t* frameBuffer;

public:
    SimulatorDisplay() : width(800), height(480), frameBuffer(nullptr) {
        initFrameBuffer();
    }

    ~SimulatorDisplay() {
        cleanupFrameBuffer();
    }

    bool init() override {
        std::cout << "SimulatorDisplay initialized successfully" << std::endl;
        return true;
    }

    void clear() override {
        if (frameBuffer) {
            for (int i = 0; i < width * height; i++) {
                frameBuffer[i] = 0xFFFF; // 白色
            }
        }
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (x >= 0 && x < width && y >= 0 && y < height && frameBuffer) {
            frameBuffer[y * width + x] = color;
        }
    }

    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) override {
        // 简单的字符绘制实现
        for (uint8_t i = 0; i < 5; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                if (c >= 'A' && c <= 'Z') {
                    // 简单的字母绘制
                    for (uint8_t sx = 0; sx < size; sx++) {
                        for (uint8_t sy = 0; sy < size; sy++) {
                            drawPixel(x + i * size + sx, y + j * size + sy, color);
                        }
                    }
                } else if (c >= '0' && c <= '9') {
                    // 简单的数字绘制
                    for (uint8_t sx = 0; sx < size; sx++) {
                        for (uint8_t sy = 0; sy < size; sy++) {
                            drawPixel(x + i * size + sx, y + j * size + sy, color);
                        }
                    }
                }
            }
        }
    }

    void drawString(int16_t x, int16_t y, const String& text, uint16_t color, uint16_t bg, uint8_t size) override {
        int16_t currentX = x;
        for (size_t i = 0; i < text.length(); i++) {
            drawChar(currentX, y, text.c_str()[i], color, bg, size);
            currentX += 6 * size;
        }
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        // 绘制矩形的四条边
        drawLine(x, y, x + w - 1, y, color);
        drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
        drawLine(x, y, x, y + h - 1, color);
        drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        for (int16_t i = x; i < x + w; i++) {
            for (int16_t j = y; j < y + h; j++) {
                drawPixel(i, j, color);
            }
        }
    }

    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override {
        // 简单的直线绘制
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1;
        int sy = y1 < y2 ? 1 : -1;
        int err = dx - dy;

        int x = x1;
        int y = y1;

        while (x != x2 || y != y2) {
            drawPixel(x, y, color);
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }
        drawPixel(x2, y2, color);
    }

    void update() override {
        std::cout << "SimulatorDisplay update" << std::endl;
        exportToHtml("simulator_display.html");
    }

    void update(int16_t x, int16_t y, int16_t w, int16_t h) override {
        std::cout << "SimulatorDisplay partial update at (" << x << ", " << y << ") size (" << w << ", " << h << ")" << std::endl;
    }

    int16_t getWidth() const override {
        return width;
    }

    int16_t getHeight() const override {
        return height;
    }

    int16_t measureTextWidth(const String& text, uint8_t size) const override {
        return text.length() * 8 * size;
    }

    int16_t measureTextHeight(const String& text, uint8_t size) const override {
        return 16 * size;
    }

    void sleep() override {
        std::cout << "SimulatorDisplay sleep" << std::endl;
    }

    void wakeup() override {
        std::cout << "SimulatorDisplay wakeup" << std::endl;
    }

    // 导出显示内容为HTML
    void exportToHtml(const char* filename) {
        std::ofstream htmlFile(filename);
        if (htmlFile.is_open()) {
            htmlFile << "<!DOCTYPE html>" << std::endl;
            htmlFile << "<html>" << std::endl;
            htmlFile << "<head>" << std::endl;
            htmlFile << "<title>Simulator Display</title>" << std::endl;
            htmlFile << "<style>" << std::endl;
            htmlFile << ".display { width: " << width << "px; height: " << height << "px; border: 1px solid #000; background-color: #fff; }" << std::endl;
            htmlFile << ".pixel { width: 1px; height: 1px; float: left; }" << std::endl;
            htmlFile << "</style>" << std::endl;
            htmlFile << "</head>" << std::endl;
            htmlFile << "<body>" << std::endl;
            htmlFile << "<h1>Simulator Display</h1>" << std::endl;
            htmlFile << "<div class=\"display\">" << std::endl;

            if (frameBuffer) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        uint16_t color = frameBuffer[y * width + x];
                        int r = (color >> 11) & 0x1F;
                        int g = (color >> 5) & 0x3F;
                        int b = color & 0x1F;
                        r = (r * 255) / 31;
                        g = (g * 255) / 63;
                        b = (b * 255) / 31;
                        htmlFile << "<div class=\"pixel\" style=\"background-color: rgb(" << r << ", " << g << ", " << b << ");\"></div>" << std::endl;
                    }
                    htmlFile << "<br style=\"clear: both;\">" << std::endl;
                }
            }

            htmlFile << "</div>" << std::endl;
            htmlFile << "</body>" << std::endl;
            htmlFile << "</html>" << std::endl;
            htmlFile.close();
            std::cout << "Display exported to " << filename << std::endl;
        }
    }

private:
    void initFrameBuffer() {
        frameBuffer = new uint16_t[width * height];
        clear();
    }

    void cleanupFrameBuffer() {
        if (frameBuffer) {
            delete[] frameBuffer;
            frameBuffer = nullptr;
        }
    }
};

// 显示管理器
class DisplayManager {
private:
    IDisplayDriver* displayDriver;
    int16_t width;
    int16_t height;

public:
    DisplayManager() : displayDriver(nullptr), width(0), height(0) {}

    ~DisplayManager() {}

    bool init() {
        if (displayDriver == nullptr) {
            std::cout << "Error: Display driver not set" << std::endl;
            return false;
        }

        if (!displayDriver->init()) {
            std::cout << "Display driver initialization failed" << std::endl;
            return false;
        }

        width = displayDriver->getWidth();
        height = displayDriver->getHeight();

        std::cout << "DisplayManager initialized successfully" << std::endl;
        return true;
    }

    void setDisplayDriver(IDisplayDriver* driver) {
        displayDriver = driver;
    }

    void showSplashScreen() {
        if (displayDriver == nullptr) {
            return;
        }

        displayDriver->clear();
        displayDriver->drawString(300, 200, "InkClock Simulator", 0x0000, 0xFFFF, 4);
        displayDriver->drawString(320, 250, "v1.0", 0x0000, 0xFFFF, 2);
        displayDriver->update();
    }

    void updateDisplay() {
        if (displayDriver == nullptr) {
            return;
        }

        displayDriver->clear();

        // 绘制时钟界面
        displayDriver->drawString(350, 100, "12:34:56", 0x0000, 0xFFFF, 4);
        displayDriver->drawString(320, 160, "2026-01-14", 0x0000, 0xFFFF, 2);
        displayDriver->drawString(300, 200, "Monday", 0x0000, 0xFFFF, 2);

        // 绘制温度和湿度
        displayDriver->drawString(300, 250, "Temperature: 22.5°C", 0x0000, 0xFFFF, 2);
        displayDriver->drawString(300, 280, "Humidity: 45%", 0x0000, 0xFFFF, 2);

        // 绘制电池状态
        displayDriver->drawString(300, 320, "Battery: 85%", 0x0000, 0xFFFF, 2);

        displayDriver->update();
    }

    void toggleClockMode() {
        std::cout << "Clock mode toggled" << std::endl;
        updateDisplay();
    }
};

// 网络管理器
class NetworkManager {
private:
    bool connected;
    std::string ipAddress;

public:
    NetworkManager() : connected(true), ipAddress("127.0.0.1") {}

    bool initialize() {
        std::cout << "NetworkManager initialized" << std::endl;
        std::cout << "IP Address: " << ipAddress << std::endl;
        return true;
    }

    bool startHTTPServer(int port) {
        std::cout << "HTTP Server started on port " << port << std::endl;
        std::cout << "Access URL: http://" << ipAddress << ":" << port << std::endl;
        return true;
    }

    bool startWebSocketServer(int port) {
        std::cout << "WebSocket Server started on port " << port << std::endl;
        return true;
    }

    bool stopHTTPServer() {
        std::cout << "HTTP Server stopped" << std::endl;
        return true;
    }

    bool stopWebSocketServer() {
        std::cout << "WebSocket Server stopped" << std::endl;
        return true;
    }

    bool isConnected() const {
        return connected;
    }

    std::string getIPAddress() const {
        return ipAddress;
    }

    void shutdown() {
        stopHTTPServer();
        stopWebSocketServer();
        std::cout << "NetworkManager shutdown" << std::endl;
    }
};

// 插件接口
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool initialize() = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

// 示例插件
class ExamplePlugin : public IPlugin {
private:
    bool enabled;

public:
    ExamplePlugin() : enabled(true) {}

    std::string getName() const override {
        return "ExamplePlugin";
    }

    std::string getVersion() const override {
        return "1.0";
    }

    std::string getDescription() const override {
        return "Example plugin for InkClock";
    }

    bool initialize() override {
        if (enabled) {
            std::cout << "ExamplePlugin initialized" << std::endl;
        }
        return true;
    }

    void update() override {
        if (enabled) {
            // 插件更新逻辑
        }
    }

    void shutdown() override {
        if (enabled) {
            std::cout << "ExamplePlugin shutdown" << std::endl;
        }
    }

    bool isEnabled() const override {
        return enabled;
    }

    void setEnabled(bool enabled) override {
        this->enabled = enabled;
    }
};

// 插件管理器
class PluginManager {
private:
    std::vector<std::unique_ptr<IPlugin>> plugins;

public:
    PluginManager() {
        // 注册默认插件
        plugins.push_back(std::make_unique<ExamplePlugin>());
    }

    void initializeAll() {
        for (auto& plugin : plugins) {
            if (plugin->isEnabled()) {
                plugin->initialize();
            }
        }
    }

    void updateAll() {
        for (auto& plugin : plugins) {
            if (plugin->isEnabled()) {
                plugin->update();
            }
        }
    }

    void shutdownAll() {
        for (auto& plugin : plugins) {
            plugin->shutdown();
        }
    }

    std::vector<IPlugin*> getAllPlugins() {
        std::vector<IPlugin*> result;
        for (auto& plugin : plugins) {
            result.push_back(plugin.get());
        }
        return result;
    }

    bool enablePlugin(const std::string& name) {
        for (auto& plugin : plugins) {
            if (plugin->getName() == name) {
                plugin->setEnabled(true);
                plugin->initialize();
                return true;
            }
        }
        return false;
    }

    bool disablePlugin(const std::string& name) {
        for (auto& plugin : plugins) {
            if (plugin->getName() == name) {
                plugin->shutdown();
                plugin->setEnabled(false);
                return true;
            }
        }
        return false;
    }
};

// 核心系统
class CoreSystem {
private:
    bool initialized;

public:
    CoreSystem() : initialized(false) {}

    bool init() {
        std::cout << "Initializing CoreSystem..." << std::endl;
        // 核心系统初始化逻辑
        initialized = true;
        std::cout << "CoreSystem initialized successfully" << std::endl;
        return true;
    }

    bool isInitialized() const {
        return initialized;
    }
};

// 主函数
int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "InkClock PC Simulator" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        // 1. 初始化核心系统
        std::cout << "Initializing CoreSystem..." << std::endl;
        CoreSystem coreSystem;
        if (!coreSystem.init()) {
            std::cerr << "Failed to initialize CoreSystem" << std::endl;
            return 1;
        }

        // 2. 创建并初始化模拟显示驱动
        std::cout << "Creating SimulatorDisplay..." << std::endl;
        SimulatorDisplay simulatorDisplay;
        if (!simulatorDisplay.init()) {
            std::cerr << "Failed to initialize SimulatorDisplay" << std::endl;
            return 1;
        }

        // 3. 初始化显示管理器
        std::cout << "Initializing DisplayManager..." << std::endl;
        DisplayManager displayManager;
        displayManager.setDisplayDriver(&simulatorDisplay);
        if (!displayManager.init()) {
            std::cerr << "Failed to initialize DisplayManager" << std::endl;
            return 1;
        }

        // 4. 初始化插件管理器
        std::cout << "Initializing PluginManager..." << std::endl;
        PluginManager pluginManager;
        pluginManager.initializeAll();

        // 5. 初始化网络管理器
        std::cout << "Initializing NetworkManager..." << std::endl;
        NetworkManager networkManager;
        networkManager.initialize();
        networkManager.startHTTPServer(8080);
        networkManager.startWebSocketServer(8081);

        // 6. 显示启动画面
        std::cout << "Showing splash screen..." << std::endl;
        displayManager.showSplashScreen();
        delay(2000);

        // 7. 更新显示
        std::cout << "Updating display..." << std::endl;
        displayManager.updateDisplay();

        // 8. 显示操作菜单
        std::cout << "========================================" << std::endl;
        std::cout << "Simulator Menu:" << std::endl;
        std::cout << "1. Update display" << std::endl;
        std::cout << "2. Show splash screen" << std::endl;
        std::cout << "3. Toggle clock mode" << std::endl;
        std::cout << "4. Manage plugins" << std::endl;
        std::cout << "5. Network settings" << std::endl;
        std::cout << "6. Exit" << std::endl;
        std::cout << "========================================" << std::endl;

        // 9. 处理用户输入
        int choice;
        do {
            std::cout << "Enter your choice: ";
            std::cin >> choice;

            switch (choice) {
                case 1:
                    std::cout << "Updating display..." << std::endl;
                    displayManager.updateDisplay();
                    pluginManager.updateAll();
                    break;
                case 2:
                    std::cout << "Showing splash screen..." << std::endl;
                    displayManager.showSplashScreen();
                    break;
                case 3:
                    std::cout << "Toggling clock mode..." << std::endl;
                    displayManager.toggleClockMode();
                    break;
                case 4:
                    {
                        // 插件管理菜单
                        std::cout << "========================================" << std::endl;
                        std::cout << "Plugin Management:" << std::endl;
                        std::cout << "========================================" << std::endl;

                        auto plugins = pluginManager.getAllPlugins();
                        for (size_t i = 0; i < plugins.size(); ++i) {
                            auto plugin = plugins[i];
                            std::cout << (i + 1) << ". " << plugin->getName() << " v" << plugin->getVersion() << " (" << (plugin->isEnabled() ? "Enabled" : "Disabled") << ")" << std::endl;
                        }
                        std::cout << "0. Back" << std::endl;
                        std::cout << "========================================" << std::endl;

                        int pluginChoice;
                        std::cout << "Enter plugin number: ";
                        std::cin >> pluginChoice;

                        if (pluginChoice > 0 && pluginChoice <= static_cast<int>(plugins.size())) {
                            auto plugin = plugins[pluginChoice - 1];
                            if (plugin->isEnabled()) {
                                pluginManager.disablePlugin(plugin->getName());
                                std::cout << "Plugin " << plugin->getName() << " disabled." << std::endl;
                            } else {
                                pluginManager.enablePlugin(plugin->getName());
                                std::cout << "Plugin " << plugin->getName() << " enabled." << std::endl;
                            }
                        }
                    }
                    break;
                case 5:
                    {
                        // 网络设置菜单
                        std::cout << "========================================" << std::endl;
                        std::cout << "Network Settings:" << std::endl;
                        std::cout << "========================================" << std::endl;
                        std::cout << "1. Show network status" << std::endl;
                        std::cout << "2. Restart HTTP Server" << std::endl;
                        std::cout << "3. Restart WebSocket Server" << std::endl;
                        std::cout << "0. Back" << std::endl;
                        std::cout << "========================================" << std::endl;

                        int networkChoice;
                        std::cout << "Enter choice: ";
                        std::cin >> networkChoice;

                        switch (networkChoice) {
                            case 1:
                                std::cout << "Network Status:" << std::endl;
                                std::cout << "Connected: " << (networkManager.isConnected() ? "Yes" : "No") << std::endl;
                                std::cout << "IP Address: " << networkManager.getIPAddress() << std::endl;
                                std::cout << "HTTP Server: http://" << networkManager.getIPAddress() << ":8080" << std::endl;
                                std::cout << "WebSocket Server: ws://" << networkManager.getIPAddress() << ":8081" << std::endl;
                                break;
                            case 2:
                                std::cout << "Restarting HTTP Server..." << std::endl;
                                networkManager.stopHTTPServer();
                                networkManager.startHTTPServer(8080);
                                break;
                            case 3:
                                std::cout << "Restarting WebSocket Server..." << std::endl;
                                networkManager.stopWebSocketServer();
                                networkManager.startWebSocketServer(8081);
                                break;
                            case 0:
                                break;
                            default:
                                std::cout << "Invalid choice, please try again." << std::endl;
                                break;
                        }
                    }
                    break;
                case 6:
                    std::cout << "Exiting..." << std::endl;
                    break;
                default:
                    std::cout << "Invalid choice, please try again." << std::endl;
                    break;
            }

        } while (choice != 6);

        // 7. 清理资源
        std::cout << "Cleaning up resources..." << std::endl;
        pluginManager.shutdownAll();
        networkManager.shutdown();

        std::cout << "========================================" << std::endl;
        std::cout << "Simulator exited successfully!" << std::endl;
        std::cout << "Preview files generated:" << std::endl;
        std::cout << "- simulator_display.html" << std::endl;
        std::cout << "========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }

    return 0;
}
