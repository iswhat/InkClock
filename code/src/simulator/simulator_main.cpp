#include <iostream>
#include <string>
#include <cstdarg>
#include "../coresystem/arduino_compat.h"
#include "../drivers/peripherals/simulator_display.h"
#include "../coresystem/core_system.h"
#include "../coresystem/dependency_injection.h"
#include "../coresystem/plugin_manager.h"
#include "../coresystem/network_manager.h"
#include "../application/display_manager.h"
#include "../plugins/example_plugin.h"

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
    
    // 2. 初始化依赖注入容器
    std::cout << "Initializing Dependency Injection Container..." << std::endl;
    diContainer = DependencyInjectionContainer::getInstance();
    
    // 3. 创建并注册模拟显示驱动
    std::cout << "Creating SimulatorDisplay..." << std::endl;
    SimulatorDisplay* simulatorDisplay = new SimulatorDisplay();
    if (!simulatorDisplay->init()) {
      std::cerr << "Failed to initialize SimulatorDisplay" << std::endl;
      delete simulatorDisplay;
      return 1;
    }
    diContainer->registerInstance<SimulatorDisplay>(simulatorDisplay, "DisplayDriver");
    
    // 4. 初始化显示管理器
    std::cout << "Initializing DisplayManager..." << std::endl;
    DisplayManager* displayManager = new DisplayManager();
    displayManager->setDisplayDriver(simulatorDisplay);
    if (!displayManager->init()) {
      std::cerr << "Failed to initialize DisplayManager" << std::endl;
      delete simulatorDisplay;
      delete displayManager;
      return 1;
    }
    diContainer->registerInstance<DisplayManager>(displayManager, "DisplayManager");
    
    // 5. 初始化插件管理器
    std::cout << "Initializing PluginManager..." << std::endl;
    PluginManager* pluginManager = &PluginManager::getInstance();
    diContainer->registerInstance<PluginManager>(pluginManager, "PluginManager");
    
    // 6. 初始化所有插件
    std::cout << "Initializing plugins..." << std::endl;
    pluginManager->initializeAll();
    
    // 7. 显示已加载的插件
    auto plugins = pluginManager->getAllPlugins();
    std::cout << "Loaded plugins: " << plugins.size() << std::endl;
    for (auto plugin : plugins) {
      std::cout << "- " << plugin->getName() << " v" << plugin->getVersion() << " (" << (plugin->isEnabled() ? "Enabled" : "Disabled") << ")" << std::endl;
      std::cout << "  Description: " << plugin->getDescription() << std::endl;
    }
    
    // 8. 初始化网络管理器
    std::cout << "Initializing NetworkManager..." << std::endl;
    NetworkManager* networkManager = &NetworkManager::getInstance();
    diContainer->registerInstance<NetworkManager>(networkManager, "NetworkManager");
    
    // 9. 启动HTTP和WebSocket服务器
    std::cout << "Starting HTTP Server..." << std::endl;
    networkManager->startHTTPServer(8080);
    std::cout << "Starting WebSocket Server..." << std::endl;
    networkManager->startWebSocketServer(8081);
    
    // 10. 注册API端点
    std::cout << "Registering API endpoints..." << std::endl;
    networkManager->registerAPI("/api/status", [](const std::string& params) {
      return "{\"status\": \"ok\", \"message\": \"InkClock is running\"}";
    });
    
    networkManager->registerAPI("/api/display/update", [](const std::string& params) {
      auto displayManager = DependencyInjectionContainer::getInstance()->getDisplayManager();
      if (displayManager) {
        displayManager->updateDisplay();
        return "{\"status\": \"ok\", \"message\": \"Display updated\"}";
      }
      return "{\"status\": \"error\", \"message\": \"Display manager not found\"}";
    });
    
    networkManager->registerAPI("/api/display/mode", [](const std::string& params) {
      auto displayManager = DependencyInjectionContainer::getInstance()->getDisplayManager();
      if (displayManager) {
        displayManager->toggleClockMode();
        return "{\"status\": \"ok\", \"message\": \"Clock mode toggled\"}";
      }
      return "{\"status\": \"error\", \"message\": \"Display manager not found\"}";
    });
    
    // 5. 显示启动画面
    std::cout << "Showing splash screen..." << std::endl;
    displayManager->showSplashScreen();
    
    // 6. 更新显示
    std::cout << "Updating display..." << std::endl;
    displayManager->updateDisplay();
    
    // 7. 导出显示内容
    std::cout << "Exporting display content..." << std::endl;
    simulatorDisplay->exportToHtml("inkclock_preview.html");
    simulatorDisplay->exportToSvg("inkclock_preview.svg");
    
    // 8. 显示操作菜单
    std::cout << "========================================" << std::endl;
    std::cout << "Simulator Menu:" << std::endl;
    std::cout << "1. Update display" << std::endl;
    std::cout << "2. Show splash screen" << std::endl;
    std::cout << "3. Toggle clock mode" << std::endl;
    std::cout << "4. Switch to calendar page" << std::endl;
    std::cout << "5. Switch to stock page" << std::endl;
    std::cout << "6. Switch to message page" << std::endl;
    std::cout << "7. Export display" << std::endl;
    std::cout << "8. Manage plugins" << std::endl;
    std::cout << "9. Network settings" << std::endl;
    std::cout << "10. Exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 9. 处理用户输入
    int choice;
    do {
      std::cout << "Enter your choice: ";
      std::cin >> choice;
      
      switch (choice) {
        case 1:
          std::cout << "Updating display..." << std::endl;
          displayManager->updateDisplay();
          // 更新插件
          pluginManager->updateAll();
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
          {
            // 插件管理菜单
            std::cout << "========================================" << std::endl;
            std::cout << "Plugin Management:" << std::endl;
            std::cout << "========================================" << std::endl;
            
            auto plugins = pluginManager->getAllPlugins();
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
                pluginManager->disablePlugin(plugin->getName());
                std::cout << "Plugin " << plugin->getName() << " disabled." << std::endl;
              } else {
                pluginManager->enablePlugin(plugin->getName());
                std::cout << "Plugin " << plugin->getName() << " enabled." << std::endl;
              }
            }
          }
          break;
        case 9:
          {
            // 网络设置菜单
            std::cout << "========================================" << std::endl;
            std::cout << "Network Settings:" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "1. Show network status" << std::endl;
            std::cout << "2. Restart HTTP Server" << std::endl;
            std::cout << "3. Restart WebSocket Server" << std::endl;
            std::cout << "4. Test API endpoint" << std::endl;
            std::cout << "0. Back" << std::endl;
            std::cout << "========================================" << std::endl;
            
            int networkChoice;
            std::cout << "Enter choice: ";
            std::cin >> networkChoice;
            
            switch (networkChoice) {
              case 1:
                std::cout << "Network Status:" << std::endl;
                std::cout << "Connected: " << (networkManager->isConnected() ? "Yes" : "No") << std::endl;
                std::cout << "IP Address: " << networkManager->getIPAddress() << std::endl;
                std::cout << "HTTP Server: http://" << networkManager->getIPAddress() << ":8080" << std::endl;
                std::cout << "WebSocket Server: ws://" << networkManager->getIPAddress() << ":8081" << std::endl;
                break;
              case 2:
                std::cout << "Restarting HTTP Server..." << std::endl;
                networkManager->stopHTTPServer();
                networkManager->startHTTPServer(8080);
                break;
              case 3:
                std::cout << "Restarting WebSocket Server..." << std::endl;
                networkManager->stopWebSocketServer();
                networkManager->startWebSocketServer(8081);
                break;
              case 4:
                std::cout << "Testing API endpoint..." << std::endl;
                networkManager->sendMessage("Testing API endpoint: /api/status");
                break;
              case 0:
                break;
              default:
                std::cout << "Invalid choice, please try again." << std::endl;
                break;
            }
          }
          break;
        case 10:
          std::cout << "Exiting..." << std::endl;
          break;
        default:
          std::cout << "Invalid choice, please try again." << std::endl;
          break;
      }
      
    } while (choice != 10);
    
    // 10. 清理资源
    std::cout << "Cleaning up resources..." << std::endl;
    if (pluginManager) {
      pluginManager->shutdownAll();
    }
    if (networkManager) {
      networkManager->shutdown();
    }
    if (diContainer) {
      diContainer->cleanup();
    }
    
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
