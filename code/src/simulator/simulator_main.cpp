#include <iostream>
#include <string>
#include "../drivers/peripherals/simulator_display.h"
#include "../coresystem/core_system.h"
#include "../application/display_manager.h"

int main(int argc, char* argv[]) {
  std::cout << "========================================" << std::endl;
  std::cout << "InkClock Simulator" << std::endl;
  std::cout << "========================================" << std::endl;
  
  try {
    // 1. 初始化核心系统
    std::cout << "Initializing CoreSystem..." << std::endl;
    CoreSystem* coreSystem = CoreSystem::getInstance();
    if (!coreSystem->init()) {
      std::cerr << "Failed to initialize CoreSystem" << std::endl;
      return 1;
    }
    
    // 2. 创建模拟显示驱动
    std::cout << "Creating SimulatorDisplay..." << std::endl;
    SimulatorDisplay* simulatorDisplay = new SimulatorDisplay();
    if (!simulatorDisplay->init()) {
      std::cerr << "Failed to initialize SimulatorDisplay" << std::endl;
      delete simulatorDisplay;
      return 1;
    }
    
    // 3. 初始化显示管理器
    std::cout << "Initializing DisplayManager..." << std::endl;
    DisplayManager* displayManager = new DisplayManager();
    displayManager->setDisplayDriver(simulatorDisplay);
    if (!displayManager->init()) {
      std::cerr << "Failed to initialize DisplayManager" << std::endl;
      delete simulatorDisplay;
      delete displayManager;
      return 1;
    }
    
    // 4. 显示启动画面
    std::cout << "Showing splash screen..." << std::endl;
    displayManager->showSplashScreen();
    
    // 5. 更新显示
    std::cout << "Updating display..." << std::endl;
    displayManager->updateDisplay();
    
    // 6. 导出显示内容
    std::cout << "Exporting display content..." << std::endl;
    simulatorDisplay->exportToHtml("inkclock_preview.html");
    simulatorDisplay->exportToSvg("inkclock_preview.svg");
    
    // 7. 显示操作菜单
    std::cout << "========================================" << std::endl;
    std::cout << "Simulator Menu:" << std::endl;
    std::cout << "1. Update display" << std::endl;
    std::cout << "2. Show splash screen" << std::endl;
    std::cout << "3. Toggle clock mode" << std::endl;
    std::cout << "4. Switch to calendar page" << std::endl;
    std::cout << "5. Switch to stock page" << std::endl;
    std::cout << "6. Switch to message page" << std::endl;
    std::cout << "7. Export display" << std::endl;
    std::cout << "8. Exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 8. 处理用户输入
    int choice;
    do {
      std::cout << "Enter your choice: ";
      std::cin >> choice;
      
      switch (choice) {
        case 1:
          std::cout << "Updating display..." << std::endl;
          displayManager->updateDisplay();
          break;
        case 2:
          std::cout << "Showing splash screen..." << std::endl;
          displayManager->showSplashScreen();
          break;
        case 3:
          std::cout << "Toggling clock mode..." << std::endl;
          displayManager->toggleClockMode();
          displayManager->updateDisplay();
          break;
        case 4:
          std::cout << "Switching to calendar page..." << std::endl;
          displayManager->switchRightPage(RIGHT_PAGE_CALENDAR);
          displayManager->updateDisplay();
          break;
        case 5:
          std::cout << "Switching to stock page..." << std::endl;
          displayManager->switchRightPage(RIGHT_PAGE_STOCK);
          displayManager->updateDisplay();
          break;
        case 6:
          std::cout << "Switching to message page..." << std::endl;
          displayManager->switchRightPage(RIGHT_PAGE_MESSAGE);
          displayManager->updateDisplay();
          break;
        case 7:
          std::cout << "Exporting display content..." << std::endl;
          simulatorDisplay->exportToHtml("inkclock_preview.html");
          simulatorDisplay->exportToSvg("inkclock_preview.svg");
          std::cout << "Display exported to inkclock_preview.html and inkclock_preview.svg" << std::endl;
          break;
        case 8:
          std::cout << "Exiting..." << std::endl;
          break;
        default:
          std::cout << "Invalid choice, please try again." << std::endl;
          break;
      }
      
    } while (choice != 8);
    
    // 9. 清理资源
    std::cout << "Cleaning up resources..." << std::endl;
    delete displayManager;
    delete simulatorDisplay;
    
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
