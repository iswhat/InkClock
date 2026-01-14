#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <fstream>

// 简化的String类
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

    // 基本方法
    const char* c_str() const {
        return _str.c_str();
    }

    size_t length() const {
        return _str.length();
    }

    bool isEmpty() const {
        return _str.empty();
    }

    int toInt() const {
        try {
            return std::stoi(_str);
        } catch (...) {
            return 0;
        }
    }

    float toFloat() const {
        try {
            return std::stof(_str);
        } catch (...) {
            return 0.0f;
        }
    }

    std::string toStdString() const {
        return _str;
    }

    // 操作符重载
    String& operator=(const char* cstr) {
        _str = cstr;
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
};

// 简化的串口类
class Serial_ {
public:
    void begin(unsigned long baud) {}
    void end() {}
    size_t print(const char* str) {
        std::cout << str;
        return strlen(str);
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
        return strlen(str) + 1;
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

// 显示管理器
class DisplayManager {
private:
    bool isDigitalClock;
    int currentRightPage;

public:
    DisplayManager() : isDigitalClock(true), currentRightPage(0) {}

    bool init() {
        std::cout << "DisplayManager initialized" << std::endl;
        return true;
    }

    void updateDisplay() {
        std::cout << "Display updated" << std::endl;
        exportDisplay();
    }

    void showSplashScreen() {
        std::cout << "Showing splash screen" << std::endl;
    }

    void toggleClockMode() {
        isDigitalClock = !isDigitalClock;
        std::cout << "Clock mode toggled to " << (isDigitalClock ? "digital" : "analog") << std::endl;
    }

    void switchRightPage(int page) {
        currentRightPage = page;
        std::cout << "Switched to page " << page << std::endl;
    }

    void exportDisplay() {
        // 生成HTML预览
        std::ofstream htmlFile("inkclock_preview.html");
        if (htmlFile.is_open()) {
            htmlFile << "<!DOCTYPE html>" << std::endl;
            htmlFile << "<html>" << std::endl;
            htmlFile << "<head>" << std::endl;
            htmlFile << "<title>InkClock Preview</title>" << std::endl;
            htmlFile << "<style>" << std::endl;
            htmlFile << "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }" << std::endl;
            htmlFile << ".clock { background-color: white; border-radius: 10px; padding: 20px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); width: 300px; margin: 0 auto; }" << std::endl;
            htmlFile << ".time { font-size: 36px; font-weight: bold; text-align: center; margin: 20px 0; }" << std::endl;
            htmlFile << ".date { font-size: 18px; text-align: center; color: #666; }" << std::endl;
            htmlFile << ".page { font-size: 14px; text-align: center; color: #999; margin-top: 20px; }" << std::endl;
            htmlFile << "</style>" << std::endl;
            htmlFile << "</head>" << std::endl;
            htmlFile << "<body>" << std::endl;
            htmlFile << "<div class='clock'>" << std::endl;
            htmlFile << "<h2>InkClock Preview</h2>" << std::endl;
            
            // 获取当前时间
            time_t now = time(0);
            struct tm* localtime = localtime(&now);
            char timeBuffer[80];
            char dateBuffer[80];
            strftime(timeBuffer, 80, "%H:%M:%S", localtime);
            strftime(dateBuffer, 80, "%Y-%m-%d %A", localtime);
            
            htmlFile << "<div class='time'>" << timeBuffer << "</div>" << std::endl;
            htmlFile << "<div class='date'>" << dateBuffer << "</div>" << std::endl;
            
            // 显示当前页面
            std::string pageName;
            switch (currentRightPage) {
                case 0: pageName = "Clock";
                    break;
                case 1: pageName = "Calendar";
                    break;
                case 2: pageName = "Stock";
                    break;
                case 3: pageName = "Message";
                    break;
                default: pageName = "Unknown";
                    break;
            }
            htmlFile << "<div class='page'>Current Page: " << pageName << "</div>" << std::endl;
            htmlFile << "<div class='page'>Clock Mode: " << (isDigitalClock ? "Digital" : "Analog") << "</div>" << std::endl;
            htmlFile << "</div>" << std::endl;
            htmlFile << "</body>" << std::endl;
            htmlFile << "</html>" << std::endl;
            htmlFile.close();
            std::cout << "Display exported to inkclock_preview.html" << std::endl;
        }

        // 生成SVG预览
        std::ofstream svgFile("inkclock_preview.svg");
        if (svgFile.is_open()) {
            svgFile << "<svg width='320' height='240' xmlns='http://www.w3.org/2000/svg'>" << std::endl;
            svgFile << "<rect width='320' height='240' fill='white' stroke='black' stroke-width='1'/>" << std::endl;
            svgFile << "<text x='160' y='40' font-family='Arial' font-size='16' text-anchor='middle' fill='black'>InkClock Preview</text>" << std::endl;
            
            // 获取当前时间
            time_t now = time(0);
            struct tm* localtime = localtime(&now);
            char timeBuffer[80];
            char dateBuffer[80];
            strftime(timeBuffer, 80, "%H:%M:%S", localtime);
            strftime(dateBuffer, 80, "%Y-%m-%d", localtime);
            
            svgFile << "<text x='160' y='80' font-family='Arial' font-size='24' text-anchor='middle' fill='black'>" << timeBuffer << "</text>" << std::endl;
            svgFile << "<text x='160' y='110' font-family='Arial' font-size='14' text-anchor='middle' fill='gray'>" << dateBuffer << "</text>" << std::endl;
            
            // 显示当前页面
            std::string pageName;
            switch (currentRightPage) {
                case 0: pageName = "Clock";
                    break;
                case 1: pageName = "Calendar";
                    break;
                case 2: pageName = "Stock";
                    break;
                case 3: pageName = "Message";
                    break;
                default: pageName = "Unknown";
                    break;
            }
            svgFile << "<text x='160' y='150' font-family='Arial' font-size='12' text-anchor='middle' fill='gray'>Current Page: " << pageName << "</text>" << std::endl;
            svgFile << "<text x='160' y='170' font-family='Arial' font-size='12' text-anchor='middle' fill='gray'>Clock Mode: " << (isDigitalClock ? "Digital" : "Analog") << "</text>" << std::endl;
            svgFile << "</svg>" << std::endl;
            svgFile.close();
            std::cout << "Display exported to inkclock_preview.svg" << std::endl;
        }
    }
};

// 核心系统
class CoreSystem {
private:
    static CoreSystem* instance;
    bool initialized;

    CoreSystem() : initialized(false) {}

public:
    static CoreSystem* getInstance() {
        if (!instance) {
            instance = new CoreSystem();
        }
        return instance;
    }

    bool init() {
        if (!initialized) {
            std::cout << "CoreSystem initialized" << std::endl;
            initialized = true;
        }
        return initialized;
    }
};

CoreSystem* CoreSystem::instance = nullptr;

// 主函数
int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "InkClock Complete Simulator" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // 初始化核心系统
        std::cout << "Initializing CoreSystem..." << std::endl;
        CoreSystem* coreSystem = CoreSystem::getInstance();
        if (!coreSystem->init()) {
            std::cerr << "Failed to initialize CoreSystem" << std::endl;
            return 1;
        }
        
        // 初始化显示管理器
        std::cout << "Initializing DisplayManager..." << std::endl;
        DisplayManager displayManager;
        if (!displayManager.init()) {
            std::cerr << "Failed to initialize DisplayManager" << std::endl;
            return 1;
        }
        
        // 显示启动画面
        std::cout << "Showing splash screen..." << std::endl;
        displayManager.showSplashScreen();
        
        // 更新显示
        std::cout << "Updating display..." << std::endl;
        displayManager.updateDisplay();
        
        // 显示操作菜单
        std::cout << "========================================" << std::endl;
        std::cout << "Simulator Menu:" << std::endl;
        std::cout << "1. Update display" << std::endl;
        std::cout << "2. Show splash screen" << std::endl;
        std::cout << "3. Toggle clock mode" << std::endl;
        std::cout << "4. Switch to calendar page" << std::endl;
        std::cout << "5. Switch to stock page" << std::endl;
        std::cout << "6. Switch to message page" << std::endl;
        std::cout << "7. Exit" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 处理用户输入
        int choice;
        do {
            std::cout << "Enter your choice: ";
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    std::cout << "Updating display..." << std::endl;
                    displayManager.updateDisplay();
                    break;
                case 2:
                    std::cout << "Showing splash screen..." << std::endl;
                    displayManager.showSplashScreen();
                    break;
                case 3:
                    std::cout << "Toggling clock mode..." << std::endl;
                    displayManager.toggleClockMode();
                    displayManager.updateDisplay();
                    break;
                case 4:
                    std::cout << "Switching to calendar page..." << std::endl;
                    displayManager.switchRightPage(1);
                    displayManager.updateDisplay();
                    break;
                case 5:
                    std::cout << "Switching to stock page..." << std::endl;
                    displayManager.switchRightPage(2);
                    displayManager.updateDisplay();
                    break;
                case 6:
                    std::cout << "Switching to message page..." << std::endl;
                    displayManager.switchRightPage(3);
                    displayManager.updateDisplay();
                    break;
                case 7:
                    std::cout << "Exiting..." << std::endl;
                    break;
                default:
                    std::cout << "Invalid choice, please try again." << std::endl;
                    break;
            }
            
        } while (choice != 7);
        
        std::cout << "========================================" << std::endl;
        std::cout << "Simulator exited successfully!" << std::endl;
        std::cout << "Preview files generated:" << std::endl;
        std::cout << "- inkclock_preview.html" << std::endl;
        std::cout << "- inkclock_preview.svg" << std::endl;
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
