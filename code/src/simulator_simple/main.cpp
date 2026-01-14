#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>

// 简化的字符串类
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

    size_t length() const {
        return _str.length();
    }

    char operator[](size_t index) const {
        return _str[index];
    }

    const char* c_str() const {
        return _str.c_str();
    }

    std::string toStdString() const {
        return _str;
    }
};

// 简化的显示驱动
class SimpleDisplay {
private:
    int width;
    int height;

public:
    SimpleDisplay(int w = 800, int h = 480) : width(w), height(h) {
        std::cout << "SimpleDisplay initialized: " << width << "x" << height << std::endl;
    }

    void clear() {
        std::cout << "Display cleared" << std::endl;
    }

    void drawString(int x, int y, const String& text, int color) {
        std::cout << "Draw string at (" << x << ", " << y << "): " << text.c_str() << std::endl;
    }

    void update() {
        std::cout << "Display updated" << std::endl;
        exportToHtml("simple_simulator.html");
        exportToSvg("simple_simulator.svg");
    }

    void exportToHtml(const char* filename) {
        std::ofstream htmlFile(filename);
        if (htmlFile.is_open()) {
            htmlFile << "<!DOCTYPE html>" << std::endl;
            htmlFile << "<html>" << std::endl;
            htmlFile << "<head>" << std::endl;
            htmlFile << "<title>InkClock Simple Simulator</title>" << std::endl;
            htmlFile << "<style>" << std::endl;
            htmlFile << ".display { width: " << width << "px; height: " << height << "px; border: 1px solid #000; background-color: #fff; padding: 20px; }" << std::endl;
            htmlFile << "h1 { font-family: Arial; color: #333; }" << std::endl;
            htmlFile << "p { font-family: Arial; color: #666; }" << std::endl;
            htmlFile << "</style>" << std::endl;
            htmlFile << "</head>" << std::endl;
            htmlFile << "<body>" << std::endl;
            htmlFile << "<div class=\"display\">" << std::endl;
            htmlFile << "<h1>InkClock Simulator</h1>" << std::endl;
            htmlFile << "<p>Display size: " << width << "x" << height << "</p>" << std::endl;
            htmlFile << "<p>This is a simplified simulator for testing purposes.</p>" << std::endl;
            htmlFile << "<p><strong>Current time:</strong> " << __DATE__ << " " << __TIME__ << "</p>" << std::endl;
            htmlFile << "<p><strong>Status:</strong> Running</p>" << std::endl;
            htmlFile << "</div>" << std::endl;
            htmlFile << "</body>" << std::endl;
            htmlFile << "</html>" << std::endl;
            htmlFile.close();
            std::cout << "Exported to " << filename << std::endl;
        }
    }

    void exportToSvg(const char* filename) {
        std::ofstream svgFile(filename);
        if (svgFile.is_open()) {
            svgFile << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;
            svgFile << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>" << std::endl;
            svgFile << "<text x=\"20\" y=\"50\" font-family=\"Arial\" font-size=\"24\" fill=\"black\">InkClock Simulator</text>" << std::endl;
            svgFile << "<text x=\"20\" y=\"80\" font-family=\"Arial\" font-size=\"16\" fill=\"gray\">Display size: " << width << "x" << height << "</text>" << std::endl;
            svgFile << "<text x=\"20\" y=\"110\" font-family=\"Arial\" font-size=\"16\" fill=\"gray\">Simplified version for testing</text>" << std::endl;
            svgFile << "<text x=\"20\" y=\"140\" font-family=\"Arial\" font-size=\"16\" fill=\"gray\">Status: Running</text>" << std::endl;
            svgFile << "</svg>" << std::endl;
            svgFile.close();
            std::cout << "Exported to " << filename << std::endl;
        }
    }
};

// 简化的显示管理器
class SimpleDisplayManager {
private:
    SimpleDisplay* display;

public:
    SimpleDisplayManager() {
        display = new SimpleDisplay();
    }

    ~SimpleDisplayManager() {
        delete display;
    }

    void updateDisplay() {
        std::cout << "Updating display..." << std::endl;
        display->clear();
        display->drawString(10, 10, String("Hello InkClock!"), 0);
        display->drawString(10, 30, String("Current time: " + std::string(__TIME__)), 0);
        display->update();
    }

    void showSplashScreen() {
        std::cout << "Showing splash screen..." << std::endl;
        display->clear();
        display->drawString(10, 10, String("InkClock Booting..."), 0);
        display->update();
    }
};

// 延迟函数
void delay(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "InkClock Simple Simulator" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        // 初始化显示管理器
        SimpleDisplayManager displayManager;

        // 显示启动画面
        displayManager.showSplashScreen();
        delay(2000);

        // 更新显示
        displayManager.updateDisplay();

        // 显示菜单
        std::cout << "========================================" << std::endl;
        std::cout << "Simulator Menu:" << std::endl;
        std::cout << "1. Update display" << std::endl;
        std::cout << "2. Show splash screen" << std::endl;
        std::cout << "3. Exit" << std::endl;
        std::cout << "========================================" << std::endl;

        int choice;
        do {
            std::cout << "Enter your choice: ";
            std::cin >> choice;

            switch (choice) {
                case 1:
                    displayManager.updateDisplay();
                    break;
                case 2:
                    displayManager.showSplashScreen();
                    delay(2000);
                    break;
                case 3:
                    std::cout << "Exiting..." << std::endl;
                    break;
                default:
                    std::cout << "Invalid choice, please try again." << std::endl;
                    break;
            }

        } while (choice != 3);

        std::cout << "========================================" << std::endl;
        std::cout << "Simulator exited successfully!" << std::endl;
        std::cout << "Preview files generated:" << std::endl;
        std::cout << "- simple_simulator.html" << std::endl;
        std::cout << "- simple_simulator.svg" << std::endl;
        std::cout << "========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}