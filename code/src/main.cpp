/**
 * @file main.cpp
 * @brief 家用网络智能墨水屏万年历主程序
 * @author iswhat
 * @date 2025-12-26
 * @version 1.1
 * 
 * 该文件是家用网络智能墨水屏万年历的主入口文件，负责初始化所有模块、
 * 调度各模块的循环任务，并处理模块间的交互。
 * 
 * 主要功能：
 * 1. 系统初始化和模块管理
 * 2. 任务调度和循环执行
 * 3. 异常处理和系统容错
 * 4. 系统安全模式保障
 */

#include <Arduino.h>
#include "coresystem/config.h"
#include "coresystem/core_system.h"
#include "coresystem/platform_abstraction.h"
#include "application/display_manager.h"
#include "drivers/peripherals/display_driver.h"
#include "drivers/peripherals/eink_driver.h"
#include "application/wifi_manager.h"
#include "application/time_manager.h"
#include "application/weather_manager.h"
#include "application/sensor_manager.h"
#include "button_manager.h"
#include "application/web_server.h"
#include "application/power_manager.h"
#include "application/lunar_manager.h"
#include "application/api_manager.h"
#include "application/geo_manager.h"
#include "coresystem/spiffs_manager.h"
#include "application/feedback_manager.h"
#include "coresystem/hardware_detector.h"
#include "coresystem/feature_manager.h"
#include "coresystem/performance_monitor.h"
#include "coresystem/storage_manager.h"

// 条件包含可选模块 - 可通过修改这些宏来启用或禁用相应功能
// 音频功能 - 用于音频录制和播放
#define ENABLE_AUDIO true      

// 蓝牙功能 - 用于首次WiFi配置和蓝牙通信
#define ENABLE_BLUETOOTH true   

// 摄像头功能 - 用于图像识别和监控
#define ENABLE_CAMERA false     

// 股票功能 - 用于获取和显示股票数据
#define ENABLE_STOCK true       

// 消息功能 - 用于接收和显示消息
#define ENABLE_MESSAGE true     

// 字体功能 - 用于字体管理和选择
#define ENABLE_FONT true        

// 插件功能 - 用于扩展功能
#define ENABLE_PLUGIN true      

// Web客户端功能 - 用于与Web服务器通信
#define ENABLE_WEBCLIENT true   

// 场景管理功能 - 用于场景模式管理
#define ENABLE_SCENE true       

// IPv6功能 - 用于IPv6网络支持
#define ENABLE_IPV6 false       

// 固件更新功能 - 用于OTA更新
#define ENABLE_FIRMWARE true    

// 触摸功能 - 用于触摸屏幕支持
#define ENABLE_TOUCH false      

// TF卡功能 - 用于存储音频、图片、视频
#define ENABLE_TF_CARD false     

// TF卡管理功能 - 用于管理TF卡内容
#define ENABLE_TF_CARD_MANAGEMENT false 

// 报警显示功能 - 用于显示报警信息和闪烁效果
#define ENABLE_ALARM_DISPLAY true

#if ENABLE_AUDIO
  #include "audio_manager.h"
#endif

#if ENABLE_BLUETOOTH
  #include "bluetooth_manager.h"
#endif

#if ENABLE_CAMERA
  #include "camera_manager.h"
#endif

#if ENABLE_STOCK
  #include "application/stock_manager.h"
#endif

#if ENABLE_MESSAGE
  #include "application/message_manager.h"
#endif

#if ENABLE_PLUGIN
  #include "extensions/plugin_manager.h"
#endif

#if ENABLE_WEBCLIENT
  #include "application/web_client.h"
#endif

#if ENABLE_SCENE
  #include "application/scene_manager.h"
#endif

#if ENABLE_FONT
  #include "coresystem/font_manager.h"
#endif

#if ENABLE_IPV6
  #include "ipv6_server.h"
#endif

#if ENABLE_FIRMWARE
  #include "firmware_manager.h"
#endif

#if ENABLE_TOUCH
  #include "touch_manager.h"
#endif

/**
 * @brief 全局对象实例
 * 
 * 所有功能模块的全局实例，用于在setup和loop函数中访问各个模块
 */// 模块注册中心实例
ModuleRegistry* moduleRegistry;

// 模块获取辅助函数
template <typename T>
T* getModule() {
  return moduleRegistry->getModule<T>();
}

// 条件注册可选模块
#if ENABLE_AUDIO
  class AudioModuleWrapper : public IModule {
  public:
    AudioModuleWrapper() : audioManager() {}
    
    void init() override {
      audioManager.init();
    }
    
    void loop() override {
      audioManager.loop();
    }
    
    String getName() const override {
      return "AudioManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_AUDIO;
    }
    
    AudioManager& getAudioManager() {
      return audioManager;
    }
    
  private:
    AudioManager audioManager;
  };
#endif

#if ENABLE_SCENE
  class SceneModuleWrapper : public IModule {
  public:
    SceneModuleWrapper() : sceneManager() {}
    
    void init() override {
      sceneManager.init();
    }
    
    void loop() override {
      sceneManager.loop();
    }
    
    String getName() const override {
      return "SceneManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_SCENE;
    }
    
    SceneManager& getSceneManager() {
      return sceneManager;
    }
    
  private:
    SceneManager sceneManager;
  };
#endif

#if ENABLE_BLUETOOTH
  class BluetoothModuleWrapper : public IModule {
  public:
    BluetoothModuleWrapper() : bluetoothManager() {}
    
    void init() override {
      bluetoothManager.init();
    }
    
    void loop() override {
      bluetoothManager.loop();
    }
    
    String getName() const override {
      return "BluetoothManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_BLUETOOTH;
    }
    
    BluetoothManager& getBluetoothManager() {
      return bluetoothManager;
    }
    
  private:
    BluetoothManager bluetoothManager;
  };
#endif

#if ENABLE_CAMERA
  class CameraModuleWrapper : public IModule {
  public:
    CameraModuleWrapper() : cameraManager() {}
    
    void init() override {
      cameraManager.init();
    }
    
    void loop() override {
      cameraManager.loop();
    }
    
    String getName() const override {
      return "CameraManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_CAMERA;
    }
    
    CameraManager& getCameraManager() {
      return cameraManager;
    }
    
  private:
    CameraManager cameraManager;
  };
#endif

#if ENABLE_STOCK
  class StockModuleWrapper : public IModule {
  public:
    StockModuleWrapper() : stockManager() {}
    
    void init() override {
      stockManager.init();
    }
    
    void loop() override {
      stockManager.loop();
    }
    
    String getName() const override {
      return "StockManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_STOCK;
    }
    
    StockManager& getStockManager() {
      return stockManager;
    }
    
  private:
    StockManager stockManager;
  };
#endif

#if ENABLE_MESSAGE
  class MessageModuleWrapper : public IModule {
  public:
    MessageModuleWrapper() : messageManager() {}
    
    void init() override {
      messageManager.init();
    }
    
    void loop() override {
      messageManager.loop();
    }
    
    String getName() const override {
      return "MessageManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_MESSAGE;
    }
    
    MessageManager& getMessageManager() {
      return messageManager;
    }
    
  private:
    MessageManager messageManager;
  };
#endif

#if ENABLE_PLUGIN
  class PluginModuleWrapper : public IModule {
  public:
    PluginModuleWrapper() : pluginManager() {}
    
    void init() override {
      pluginManager.init();
    }
    
    void loop() override {
      pluginManager.loop();
    }
    
    String getName() const override {
      return "PluginManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_PLUGIN;
    }
    
    PluginManager& getPluginManager() {
      return pluginManager;
    }
    
  private:
    PluginManager pluginManager;
  };
#endif

#if ENABLE_WEBCLIENT
  class WebClientModuleWrapper : public IModule {
  public:
    WebClientModuleWrapper() : webClient() {}
    
    void init() override {
      webClient.init();
    }
    
    void loop() override {
      webClient.loop();
    }
    
    String getName() const override {
      return "WebClient";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_WEBCLIENT;
    }
    
    WebClient& getWebClient() {
      return webClient;
    }
    
  private:
    WebClient webClient;
  };
#endif

#if ENABLE_FONT
  class FontModuleWrapper : public IModule {
  public:
    FontModuleWrapper() : fontManager() {}
    
    void init() override {
      fontManager.init();
    }
    
    void loop() override {
      fontManager.loop();
    }
    
    String getName() const override {
      return "FontManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_FONT;
    }
    
    FontManager& getFontManager() {
      return fontManager;
    }
    
  private:
    FontManager fontManager;
  };
#endif

#if ENABLE_FIRMWARE
  class FirmwareModuleWrapper : public IModule {
  public:
    FirmwareModuleWrapper() : firmwareManager() {}
    
    void init() override {
      firmwareManager.init();
    }
    
    void loop() override {
      firmwareManager.loop();
    }
    
    String getName() const override {
      return "FirmwareManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_FIRMWARE;
    }
    
    FirmwareManager& getFirmwareManager() {
      return firmwareManager;
    }
    
  private:
    FirmwareManager firmwareManager;
  };
#endif

#if ENABLE_TOUCH
  class TouchModuleWrapper : public IModule {
  public:
    TouchModuleWrapper() : touchManager() {}
    
    void init() override {
      touchManager.init();
    }
    
    void loop() override {
      touchManager.loop();
    }
    
    String getName() const override {
      return "TouchManager";
    }
    
    ModuleType getModuleType() const override {
      return MODULE_TYPE_TOUCH;
    }
    
    TouchManager& getTouchManager() {
      return touchManager;
    }
    
  private:
    TouchManager touchManager;
  };
#endif

// 核心模块包装器
class DisplayModuleWrapper : public IModule {
public:
  DisplayModuleWrapper() : displayManager() {}
  
  void init() override {
    displayManager.init();
  }
  
  void loop() override {
    displayManager.loop();
  }
  
  String getName() const override {
    return "DisplayManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_DISPLAY;
  }
  
  DisplayManager& getDisplayManager() {
    return displayManager;
  }
  
private:
  DisplayManager displayManager;
};

class WiFiModuleWrapper : public IModule {
public:
  WiFiModuleWrapper() : wifiManager() {}
  
  void init() override {
    wifiManager.init();
  }
  
  void loop() override {
    wifiManager.loop();
  }
  
  String getName() const override {
    return "WiFiManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_WIFI;
  }
  
  WiFiManager& getWiFiManager() {
    return wifiManager;
  }
  
private:
  WiFiManager wifiManager;
};

class TimeModuleWrapper : public IModule {
public:
  TimeModuleWrapper() : timeManager() {}
  
  void init() override {
    timeManager.init();
  }
  
  void loop() override {
    timeManager.loop();
  }
  
  String getName() const override {
    return "TimeManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_TIME;
  }
  
  TimeManager& getTimeManager() {
    return timeManager;
  }
  
private:
  TimeManager timeManager;
};

class LunarModuleWrapper : public IModule {
public:
  LunarModuleWrapper() : lunarManager() {}
  
  void init() override {
    lunarManager.init();
  }
  
  void loop() override {
    lunarManager.loop();
  }
  
  String getName() const override {
    return "LunarManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_LUNAR;
  }
  
  LunarManager& getLunarManager() {
    return lunarManager;
  }
  
private:
  LunarManager lunarManager;
};

class WeatherModuleWrapper : public IModule {
public:
  WeatherModuleWrapper() : weatherManager() {}
  
  void init() override {
    weatherManager.init();
  }
  
  void loop() override {
    weatherManager.loop();
  }
  
  String getName() const override {
    return "WeatherManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_WEATHER;
  }
  
  WeatherManager& getWeatherManager() {
    return weatherManager;
  }
  
private:
  WeatherManager weatherManager;
};

class SensorModuleWrapper : public IModule {
public:
  SensorModuleWrapper() : sensorManager() {}
  
  void init() override {
    sensorManager.init();
  }
  
  void loop() override {
    sensorManager.loop();
  }
  
  String getName() const override {
    return "SensorManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_SENSOR;
  }
  
  SensorManager& getSensorManager() {
    return sensorManager;
  }
  
private:
  SensorManager sensorManager;
};

class ButtonModuleWrapper : public IModule {
public:
  ButtonModuleWrapper() : buttonManager() {}
  
  void init() override {
    buttonManager.init();
  }
  
  void loop() override {
    buttonManager.loop();
  }
  
  String getName() const override {
    return "ButtonManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_BUTTON;
  }
  
  ButtonManager& getButtonManager() {
    return buttonManager;
  }
  
private:
  ButtonManager buttonManager;
};

class FeedbackModuleWrapper : public IModule {
public:
  FeedbackModuleWrapper() : feedbackManager() {}
  
  void init() override {
    feedbackManager.init();
  }
  
  void loop() override {
    feedbackManager.update();
  }
  
  String getName() const override {
    return "FeedbackManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_FEEDBACK;
  }
  
  FeedbackManager& getFeedbackManager() {
    return feedbackManager;
  }
  
private:
  FeedbackManager feedbackManager;
};

class PowerModuleWrapper : public IModule {
public:
  PowerModuleWrapper() : powerManager() {}
  
  void init() override {
    powerManager.init();
  }
  
  void loop() override {
    powerManager.loop();
  }
  
  String getName() const override {
    return "PowerManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_POWER;
  }
  
  PowerManager& getPowerManager() {
    return powerManager;
  }
  
private:
  PowerManager powerManager;
};

class WebServerModuleWrapper : public IModule {
public:
  WebServerModuleWrapper() : webServerManager() {}
  
  void init() override {
    webServerManager.init();
  }
  
  void loop() override {
    webServerManager.loop();
  }
  
  String getName() const override {
    return "WebServerManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_WEB_SERVER;
  }
  
  WebServerManager& getWebServerManager() {
    return webServerManager;
  }
  
private:
  WebServerManager webServerManager;
};

class APIModuleWrapper : public IModule {
public:
  APIModuleWrapper() : apiManager() {}
  
  void init() override {
    apiManager.init();
  }
  
  void loop() override {
    apiManager.loop();
  }
  
  String getName() const override {
    return "APIManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_API;
  }
  
  APIManager& getAPIManager() {
    return apiManager;
  }
  
private:
  APIManager apiManager;
};

class GeoModuleWrapper : public IModule {
public:
  GeoModuleWrapper() : geoManager() {}
  
  void init() override {
    geoManager.init();
  }
  
  void loop() override {
    geoManager.loop();
  }
  
  String getName() const override {
    return "GeoManager";
  }
  
  ModuleType getModuleType() const override {
    return MODULE_TYPE_GEO;
  }
  
  GeoManager& getGeoManager() {
    return geoManager;
  }
  
private:
  GeoManager geoManager;
};

/**
 * @brief 初始化函数
 * 
 * 设备启动时执行的初始化函数，负责初始化所有功能模块
 * 执行顺序：
 * 1. 串口初始化
 * 2. 固件管理器初始化（硬件自动检测）
 * 3. 显示驱动初始化
 * 4. 蓝牙管理模块初始化
 * 5. 传感器和其他不需要WiFi的模块初始化
 * 6. Web客户端初始化
 * 7. WiFi连接（如果未配置则等待蓝牙配置）
 * 8. 需要WiFi的模块初始化
 * 9. Web服务器和IPv6服务器初始化
 * 10. 初始数据更新
 * 11. 初始页面显示
 */
// 初始化基础系统组件
void initSystem() {
  // 初始化串口通信，用于调试输出
  Serial.begin(115200);
  platformDelay(1000);
  
  // 打印启动信息
  Serial.println("===== 家用网络智能墨水屏万年历 ====");
  
  // 初始化SPIFFS，确保在其他模块之前初始化
  try {
    initSPIFFS();
    Serial.println("SPIFFS初始化完成");
  } catch (const std::exception& e) {
    Serial.print("SPIFFS初始化异常: ");
    Serial.println(e.what());
  }
  
  // 初始化硬件检测器，用于硬件资源评估
  try {
    HardwareDetector* hardwareDetector = HardwareDetector::getInstance();
    if (hardwareDetector->init()) {
      // 执行硬件资源检测
      hardwareDetector->detectResources();
      hardwareDetector->evaluateCapabilities();
      
      // 获取硬件评估结果
      HardwareEvaluationResult result = hardwareDetector->getEvaluationResult();
      Serial.printf("硬件评估完成: 得分=%.2f, 级别=%d\n", result.overallScore, result.overallLevel);
      Serial.printf("硬件平台: %s\n", result.platform.c_str());
      Serial.printf("总内存: %.2f KB\n", result.totalMemory);
      Serial.printf("总存储: %.2f KB\n", result.totalStorage);
      
      Serial.println("硬件检测器初始化完成");
    } else {
      Serial.println("硬件检测器初始化失败");
    }
  } catch (const std::exception& e) {
    Serial.print("硬件检测器初始化异常: ");
    Serial.println(e.what());
  }
  
  // 初始化功能管理器，用于功能降级策略
  try {
    FeatureManager* featureManager = FeatureManager::getInstance();
    if (featureManager->init()) {
      // 评估功能级别
      featureManager->evaluateFeatures();
      
      // 打印功能状态
      std::vector<FeatureConfig> featureConfigs = featureManager->getAllFeatureConfigs();
      Serial.println("功能状态:");
      for (const auto& config : featureConfigs) {
        if (featureManager->isFeatureEnabled(config.name)) {
          Serial.printf("%s: 级别=%d, 状态=启用\n", config.name.c_str(), config.currentLevel);
        } else {
          Serial.printf("%s: 级别=%d, 状态=禁用\n", config.name.c_str(), config.currentLevel);
        }
      }
      
      Serial.println("功能管理器初始化完成");
    } else {
      Serial.println("功能管理器初始化失败");
    }
  } catch (const std::exception& e) {
    Serial.print("功能管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  // 初始化性能监控器，用于实时性能监控和预警
  try {
    PerformanceMonitor* performanceMonitor = PerformanceMonitor::getInstance();
    if (performanceMonitor->init()) {
      // 运行基准测试
      float benchmarkTime = performanceMonitor->runBenchmark();
      Serial.printf("性能基准测试完成: %.2f ms\n", benchmarkTime);
      
      // 检查系统健康状态
      performanceMonitor->checkSystemHealth();
      
      Serial.println("性能监控器初始化完成");
    } else {
      Serial.println("性能监控器初始化失败");
    }
  } catch (const std::exception& e) {
    Serial.print("性能监控器初始化异常: ");
    Serial.println(e.what());
  }
  
  // 初始化存储管理器，用于分层存储管理
  try {
    StorageManager* storageManager = StorageManager::getInstance();
    if (storageManager->init()) {
      // 检查存储系统健康状态
      if (storageManager->checkHealth()) {
        Serial.println("存储系统健康状态良好");
      } else {
        Serial.println("存储系统健康状态异常");
      }
      
      // 打印存储介质信息
      std::vector<StorageMediumInfo> storageInfos = storageManager->getAllStorageMediumInfo();
      for (const auto& info : storageInfos) {
        if (info.available) {
          Serial.printf("存储介质: %s, 总容量: %.2f MB, 可用容量: %.2f MB\n", 
                      info.name.c_str(), 
                      (float)info.totalSize / (1024 * 1024), 
                      (float)info.availableSize / (1024 * 1024));
        }
      }
      
      Serial.println("存储管理器初始化完成");
    } else {
      Serial.println("存储管理器初始化失败");
    }
  } catch (const std::exception& e) {
    Serial.print("存储管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  // 初始化电源管理，影响系统运行模式
  try {
    powerManager.init();        // 电源管理和低功耗控制
    Serial.println("电源管理初始化完成");
  } catch (const std::exception& e) {
    Serial.print("电源管理初始化异常: ");
    Serial.println(e.what());
  }
}

// 注册硬件驱动
void registerHardwareDrivers() {
  try {
    Serial.println("注册硬件驱动...");
    
    // 注册墨水屏驱动
    registerDisplayDriver<EinkDriver>();
    
    // 注册所有传感器驱动到驱动注册表
    Serial.println("注册传感器驱动...");
    
    // 温湿度传感器驱动
    registerSensorDriver<DHT22Driver>();
    registerSensorDriver<SHT30Driver>();
    registerSensorDriver<AM2302Driver>();
    registerSensorDriver<SHT20Driver>();
    registerSensorDriver<SHT21Driver>();
    registerSensorDriver<SHT40Driver>();
    registerSensorDriver<HDC1080Driver>();
    registerSensorDriver<HTU21DDriver>();
    registerSensorDriver<SI7021Driver>();
    registerSensorDriver<BME280Driver>();
    registerSensorDriver<BME680Driver>();
    registerSensorDriver<LPS25HBDriver>();
    registerSensorDriver<BMP388Driver>();
    
    // 人体感应传感器驱动
    registerSensorDriver<HC_SR501Driver>();
    registerSensorDriver<HC_SR505Driver>();
    registerSensorDriver<RE200BDriver>();
    registerSensorDriver<LD2410Driver>();
    
    // 光照传感器驱动
    registerSensorDriver<BH1750Driver>();
    registerSensorDriver<TSL2561Driver>();
    registerSensorDriver<GY30Driver>();
    registerSensorDriver<SI1145Driver>();
    
    // 气体传感器驱动
    registerSensorDriver<MQ2Driver>();
    registerSensorDriver<MQ5Driver>();
    registerSensorDriver<MQ7Driver>();
    registerSensorDriver<MQ135Driver>();
    registerSensorDriver<TGS2600Driver>();
    registerSensorDriver<SGP30Driver>();
    
    // 火焰传感器驱动
    registerSensorDriver<IRFlameDriver>();
    
    Serial.println("传感器驱动注册完成");
  } catch (const std::exception& e) {
    Serial.print("驱动注册异常: ");
    Serial.println(e.what());
  }
}

// 初始化显示系统
void initDisplaySystem() {
  // 使用驱动注册表自动检测并获取显示驱动
  try {
    DriverRegistry* registry = DriverRegistry::getInstance();
    IDisplayDriver* displayDriver = nullptr;
    
    // 如果配置了固定的显示类型，则直接获取对应驱动
    #if DISPLAY_TYPE != EINK_75_INCH
      displayDriver = registry->getDisplayDriver(DISPLAY_TYPE);
      Serial.print("使用配置的显示驱动: ");
    #else
      // 否则自动检测显示驱动
      displayDriver = registry->autoDetectDisplayDriver();
      Serial.print("自动检测显示驱动: ");
    #endif
    
    if (displayDriver == nullptr) {
      // 如果无法获取显示驱动，使用默认的墨水屏驱动
      Serial.println("未找到匹配的显示驱动，使用默认墨水屏驱动");
      displayDriver = new EinkDriver();
    } else {
      Serial.println("成功获取显示驱动");
    }
    
    // 设置显示驱动
    displayManager.setDisplayDriver(displayDriver);
    
    // 初始化显示管理器，显示启动画面
    displayManager.init();
    displayManager.showSplashScreen();
    Serial.println("显示管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("显示驱动初始化异常: ");
    Serial.println(e.what());
  }
}

// 初始化输入设备
void initInputDevices() {
  // 初始化输入设备，用于用户交互
  try {
    buttonManager.init();       // 按键事件处理
    
    // 初始化状态反馈管理器
    feedbackManager.init();
    // 设置LED引脚（根据实际硬件连接修改）
    feedbackManager.setLEDPins(13, 12, 14); // 电源LED、WiFi LED、蓝牙LED
    // 设置屏幕驱动
    feedbackManager.setDisplayDriver(displayManager.getDisplayDriver());
    
    // 设置按键回调函数，用于处理按键事件，包括报警状态下切换回主界面
  buttonManager.setCallback([](int buttonIndex, ButtonEvent event) {
    if (displayManager.isAlarmShowing()) {
      // 如果处于报警状态，任何按键都切换回主界面
      displayManager.hideAlarm();
      return;
    }
    
    // 单按钮操作逻辑
    switch (event) {
      case BUTTON_CLICK:
        // 单击：切换右侧页面（日历/股票/消息）
        {
          static RightPageType currentPage = RIGHT_PAGE_CALENDAR;
          switch (currentPage) {
            case RIGHT_PAGE_CALENDAR:
              currentPage = RIGHT_PAGE_STOCK;
              break;
            case RIGHT_PAGE_STOCK:
              currentPage = RIGHT_PAGE_MESSAGE;
              break;
            case RIGHT_PAGE_MESSAGE:
              currentPage = RIGHT_PAGE_CALENDAR;
              break;
            default:
              currentPage = RIGHT_PAGE_CALENDAR;
              break;
          }
          displayManager.switchRightPage(currentPage);
          displayManager.updateDisplay();
          // 触发反馈
          feedbackManager.triggerFeedback(FEEDBACK_CLICK);
        }
        break;
        
      case BUTTON_DOUBLE_CLICK:
        // 双击：切换显示模式（数字/模拟/文字时钟）
        displayManager.toggleClockMode();
        displayManager.updateDisplay();
        // 触发反馈
        feedbackManager.triggerFeedback(FEEDBACK_DOUBLE_CLICK);
        break;
        
      case BUTTON_TRIPLE_CLICK:
        // 三连击：切换到下一个场景
        #if ENABLE_SCENE
          SceneManager& sceneManager = getModule<SceneModuleWrapper>()->getSceneManager();
          sceneManager.switchToNextScene();
          // 显示当前场景名称
          displayManager.showToastMessage("场景: " + sceneManager.getSceneConfig(sceneManager.getCurrentScene()).name, 2000);
        #endif
        // 触发反馈
        feedbackManager.triggerFeedback(FEEDBACK_TRIPLE_CLICK);
        break;
        
      case BUTTON_LONG_PRESS:
        // 长按：开机/关机
        DEBUG_PRINTLN("长按：开机/关机");
        // 触发反馈
        feedbackManager.triggerFeedback(FEEDBACK_LONG_PRESS);
        break;
        
      case BUTTON_POWER_OFF:
        // 长按5秒以上：关机
        DEBUG_PRINTLN("长按5秒以上：关机");
        // 触发反馈
        feedbackManager.triggerFeedback(FEEDBACK_POWER_OFF);
        // 这里可以添加关机逻辑
        // 例如：进入深度睡眠模式
        platformDeepSleep(0); // 永久睡眠
        break;
        
      default:
        break;
    }
  });
    Serial.println("按键管理器初始化完成");
    Serial.println("状态反馈管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("按键管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  #if ENABLE_TOUCH
    try {
      touchManager.init();        // 触摸事件处理
      Serial.println("触摸管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("触摸管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif
}

// 初始化本地模块（不需要网络）
void initLocalModules() {
  // 初始化蓝牙管理模块，用于首次WiFi配置
  #if ENABLE_BLUETOOTH
    try {
      bluetoothManager.init();
      Serial.println("蓝牙管理初始化完成");
    } catch (const std::exception& e) {
      Serial.print("蓝牙管理初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  // 初始化本地传感器和不需要网络的模块
  try {
    sensorManager.init();       // 温湿度传感器管理
    Serial.println("传感器管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("传感器管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  #if ENABLE_AUDIO
    try {
      audioManager.init();        // 音频录制和播放管理
      Serial.println("音频管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("音频管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  #if ENABLE_MESSAGE
    try {
      messageManager.init();      // 消息接收和存储
      Serial.println("消息管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("消息管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif

  #if ENABLE_SCENE
    try {
      sceneManager.init();        // 场景模式管理
      Serial.println("场景管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("场景管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  #if ENABLE_PLUGIN
    try {
      pluginManager.init();       // 插件加载和管理
      Serial.println("插件管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("插件管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  #if ENABLE_TF_CARD
    try {
      #include "coresystem/tf_card_manager.h"
      initTFCard(SD_CS);          // TF卡初始化
      Serial.println("TF卡初始化完成");
    } catch (const std::exception& e) {
      Serial.print("TF卡初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  #if ENABLE_FONT
    try {
      fontManager.init();         // 字体管理初始化
      Serial.println("字体管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("字体管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  #if ENABLE_CAMERA
    try {
      cameraManager.init();       // 摄像头管理
      Serial.println("摄像头管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("摄像头管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif
}

// 初始化网络相关模块
void initNetworkModules() {
  // 初始化网络通信相关模块
  #if ENABLE_WEBCLIENT
    try {
      webClient.init();           // Web客户端，用于与云端服务器通信
      Serial.println("Web客户端初始化完成");
    } catch (const std::exception& e) {
      Serial.print("Web客户端初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  try {
    apiManager.init();          // API管理器，用于统一处理所有外部API请求
    Serial.println("API管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("API管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  try {
    geoManager.init();          // 地理位置管理器，用于自动检测和管理地理位置
    Serial.println("地理位置管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("地理位置管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  // 初始化WiFi模块
  try {
    wifiManager.init();
    Serial.println("WiFi管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("WiFi管理器初始化异常: ");
    Serial.println(e.what());
  }
}

// 初始化需要网络的模块
void initNetworkDependentModules() {
  // 初始化需要WiFi的模块
  try {
    timeManager.init();         // NTP时间同步
    Serial.println("时间管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("时间管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  try {
    weatherManager.init();      // 天气数据获取
    Serial.println("天气管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("天气管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  #if ENABLE_STOCK
    try {
      stockManager.init();        // 股票数据获取和显示
      Serial.println("股票管理器初始化完成");
    } catch (const std::exception& e) {
      Serial.print("股票管理器初始化异常: ");
      Serial.println(e.what());
    }
  #endif
  
  try {
    lunarManager.init();        // 农历管理模块
    Serial.println("农历管理器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("农历管理器初始化异常: ");
    Serial.println(e.what());
  }
  
  // 初始化Web服务器
  try {
    // 设置web_server.cpp中的coreSystem指针
    extern CoreSystem* coreSystem;
    coreSystem = CoreSystem::getInstance();
    
    webServerManager.init();
    Serial.println("Web服务器初始化完成");
  } catch (const std::exception& e) {
    Serial.print("Web服务器初始化异常: ");
    Serial.println(e.what());
  }
}

// 更新初始数据
void updateInitialData() {
  // 更新初始数据
  try {
    timeManager.update();       // 更新时间
    Serial.println("时间数据更新完成");
  } catch (const std::exception& e) {
    Serial.print("时间数据更新异常: ");
    Serial.println(e.what());
  }
  
  try {
    weatherManager.update();    // 更新天气
    Serial.println("天气数据更新完成");
  } catch (const std::exception& e) {
    Serial.print("天气数据更新异常: ");
    Serial.println(e.what());
  }
  
  try {
    sensorManager.update();     // 更新传感器数据
    Serial.println("传感器数据更新完成");
  } catch (const std::exception& e) {
    Serial.print("传感器数据更新异常: ");
    Serial.println(e.what());
  }
  
  #if ENABLE_STOCK
    try {
      stockManager.update();      // 更新股票数据
      Serial.println("股票数据更新完成");
    } catch (const std::exception& e) {
      Serial.print("股票数据更新异常: ");
      Serial.println(e.what());
    }
  #endif
  
  try {
    lunarManager.update();      // 更新农历数据
    Serial.println("农历数据更新完成");
  } catch (const std::exception& e) {
    Serial.print("农历数据更新异常: ");
    Serial.println(e.what());
  }
  
  // 显示初始页面
  try {
    displayManager.updateDisplay();
    Serial.println("初始页面显示完成");
  } catch (const std::exception& e) {
    Serial.print("初始页面显示异常: ");
    Serial.println(e.what());
  }
}

void setup() {
  // 初始化核心系统（底层操作系统）
  CoreSystem* coreSystem = CoreSystem::getInstance();
  if (!coreSystem->init()) {
    Serial.println("核心系统初始化失败，进入安全模式");
    // 安全模式下只初始化必要功能
    initSystem();
    registerHardwareDrivers();
    initDisplaySystem();
    return;
  }
  
  // 初始化应用层模块
  initInputDevices();
  initLocalModules();
  initNetworkModules();
  initNetworkDependentModules();
  updateInitialData();
  
  // 打印初始化完成信息
  Serial.println("===== 初始化完成 =====");
}

/**
 * @brief 主循环函数
 * 
 * 设备启动后持续执行的主循环，负责调度各模块的循环任务
 * 执行顺序：
 * 1. 各模块的循环任务处理
 * 2. 根据电源管理建议更新显示
 * 3. 定期更新数据
 */
void loop() {
  static unsigned long lastWatchdogReset = 0;
  static const unsigned long WATCHDOG_TIMEOUT = 60000; // 看门狗超时时间，60秒
  
  // 获取核心系统实例
  CoreSystem* coreSystem = CoreSystem::getInstance();
  
  try {
    // 重置软件看门狗
    lastWatchdogReset = platformGetMillis();
    
    // 运行核心系统（底层操作系统）
    coreSystem->run();
    
    // 处理各个模块的循环任务，每个模块都用try-catch包裹，确保单个模块崩溃不会影响整个系统
  // 核心功能模块 - 优先执行
  try {
    wifiManager.loop();           // WiFi状态监测和重连（核心网络功能）
  } catch (const std::exception& e) {
    coreSystem->sendError("WiFi模块异常", 1001, "WiFiManager");
  }
  
  try {
    powerManager.loop();          // 电源状态监测和低功耗控制（影响系统运行模式）
  } catch (const std::exception& e) {
    coreSystem->sendError("电源模块异常", 1002, "PowerManager");
  }
  
  // 实时交互模块 - 需要快速响应
  try {
    buttonManager.loop();         // 按键事件检测（实时交互）
  } catch (const std::exception& e) {
    coreSystem->sendError("按键模块异常", 1003, "ButtonManager");
  }
  
  try {
    feedbackManager.update();      // 操作反馈系统（实时响应）
  } catch (const std::exception& e) {
    coreSystem->sendError("反馈模块异常", 1013, "FeedbackManager");
  }
  
  #if ENABLE_TOUCH
    try {
      touchManager.loop();          // 触摸事件检测（实时交互）
    } catch (const std::exception& e) {
      coreSystem->sendError("触摸模块异常", 1004, "TouchManager");
    }
  #endif
  
  // 时间基准模块 - 影响其他模块的时间戳
  try {
    timeManager.loop();           // 时间更新和同步（系统时间基准）
  } catch (const std::exception& e) {
    coreSystem->sendError("时间模块异常", 1005, "TimeManager");
  }
  
  // 地理位置模块 - 影响天气等数据
  try {
    geoManager.loop();            // 地理位置更新和管理
  } catch (const std::exception& e) {
    coreSystem->sendError("地理位置模块异常", 1006, "GeoManager");
  }
  
  // 数据获取模块 - 顺序执行，避免并发网络请求
  try {
    sensorManager.loop();         // 传感器数据采集（本地传感器，响应快）
  } catch (const std::exception& e) {
    coreSystem->sendError("传感器模块异常", 1007, "SensorManager");
  }
  
  try {
    lunarManager.loop();          // 农历管理模块循环
  } catch (const std::exception& e) {
    coreSystem->sendError("农历模块异常", 1008, "LunarManager");
  }
  
  try {
    weatherManager.loop();        // 天气数据管理
  } catch (const std::exception& e) {
    coreSystem->sendError("天气模块异常", 1009, "WeatherManager");
  }
  
  #if ENABLE_STOCK
    try {
      stockManager.loop();          // 股票数据管理
    } catch (const std::exception& e) {
      coreSystem->sendError("股票模块异常", 1010, "StockManager");
    }
  #endif
  
  // 扩展功能模块 - 资源消耗较大，放在后面
  #if ENABLE_AUDIO
    try {
      audioManager.loop();          // 音频播放控制
    } catch (const std::exception& e) {
      coreSystem->sendError("音频模块异常", 1011, "AudioManager");
    }
  #endif
  
  #if ENABLE_BLUETOOTH
    try {
      bluetoothManager.loop();      // 蓝牙连接管理
    } catch (const std::exception& e) {
      coreSystem->sendError("蓝牙模块异常", 1012, "BluetoothManager");
    }
  #endif
  
  #if ENABLE_CAMERA
    try {
      cameraManager.loop();         // 摄像头状态管理
    } catch (const std::exception& e) {
      coreSystem->sendError("摄像头模块异常", 1013, "CameraManager");
    }
  #endif
  
  // 网络服务模块
  try {
    webServerManager.loop();      // Web服务器请求处理
  } catch (const std::exception& e) {
    coreSystem->sendError("Web服务器模块异常", 1014, "WebServerManager");
  }
  
  #if ENABLE_WEBCLIENT
    try {
      webClient.loop();             // 与云端服务器通信
    } catch (const std::exception& e) {
      coreSystem->sendError("Web客户端模块异常", 1015, "WebClient");
    }
  #endif
  
  // 后台功能模块
  #if ENABLE_MESSAGE
    try {
      messageManager.loop();       // 消息处理和通知
    } catch (const std::exception& e) {
      coreSystem->sendError("消息模块异常", 1011, "MessageManager");
    }
  #endif

  #if ENABLE_SCENE
    try {
      sceneManager.loop();         // 场景模式管理
    } catch (const std::exception& e) {
      coreSystem->sendError("场景模块异常", 1014, "SceneManager");
    }
  #endif
  
  #if ENABLE_PLUGIN
    try {
      pluginManager.loop();         // 插件更新和显示
    } catch (const std::exception& e) {
      coreSystem->sendError("插件模块异常", 1017, "PluginManager");
    }
  #endif
  
  #if ENABLE_FIRMWARE
    try {
      firmwareManager.loop();       // 固件更新检查
    } catch (const std::exception& e) {
      coreSystem->sendError("固件管理模块异常", 1018, "FirmwareManager");
    }
  #endif
  
  // 根据power_manager的建议更新显示，减少不必要的刷新
  try {
    if (powerManager.shouldUpdateDisplay()) {
      // 使用局部刷新更新显示，降低功耗
      displayManager.updateDisplayPartial();
    }
  } catch (const std::exception& e) {
    coreSystem->sendError("显示更新异常", 1019, "DisplayManager");
  }
  
  // 定期更新数据 - 为每个模块使用独立的更新间隔，避免不必要的数据刷新
  static unsigned long lastTimeUpdate = 0;
  static unsigned long lastWeatherUpdate = 0;
  static unsigned long lastSensorUpdate = 0;
  static unsigned long lastLunarUpdate = 0;
  
  unsigned long now = platformGetMillis();
  
  // 更新时间（根据time_manager的配置）
  try {
    if (now - lastTimeUpdate > CLOCK_REFRESH_INTERVAL) {
      lastTimeUpdate = now;
      timeManager.update();       // 更新时间
    }
  } catch (const std::exception& e) {
    coreSystem->sendError("时间更新异常", 1020, "TimeManager");
  }
  
  // 更新天气（根据weather_manager的配置）
  try {
    if (now - lastWeatherUpdate > WEATHER_REFRESH_INTERVAL) {
      lastWeatherUpdate = now;
      weatherManager.update();    // 更新天气
    }
  } catch (const std::exception& e) {
    coreSystem->sendError("天气更新异常", 1021, "WeatherManager");
  }
  
  // 更新传感器数据（根据sensor_manager的配置）
  try {
    if (now - lastSensorUpdate > SENSOR_REFRESH_INTERVAL) {
      lastSensorUpdate = now;
      sensorManager.update();     // 更新传感器数据
    }
  } catch (const std::exception& e) {
    coreSystem->sendError("传感器更新异常", 1022, "SensorManager");
  }
  
  #if ENABLE_STOCK
    static unsigned long lastStockUpdate = 0;
    // 更新股票数据（根据stock_manager的配置）
    try {
      if (now - lastStockUpdate > STOCK_REFRESH_INTERVAL) {
        lastStockUpdate = now;
        stockManager.update();      // 更新股票数据
      }
    } catch (const std::exception& e) {
      coreSystem->sendError("股票更新异常", 1023, "StockManager");
    }
  #endif
  
  // 更新农历数据（根据lunar_manager的配置，每天更新一次）
  try {
    if (now - lastLunarUpdate > 86400000) { // 24小时
      lastLunarUpdate = now;
      lunarManager.update();      // 更新农历数据
    }
  } catch (const std::exception& e) {
    coreSystem->sendError("农历更新异常", 1024, "LunarManager");
  }
} catch (const std::exception& e) {
  // 捕获所有未处理的异常，确保系统不会完全崩溃
  coreSystem->sendError("主循环异常", 1025, "MainLoop");
  
  // 尝试恢复基本功能
  try {
    // 仅保留核心功能运行
    webServerManager.loop();
    #if ENABLE_FIRMWARE
      firmwareManager.loop();
    #endif
  } catch (...) {
    // 忽略恢复过程中的异常
  }
} catch (...) {
  // 捕获所有其他类型的异常
  coreSystem->sendError("未知异常", 1026, "MainLoop");
}
  
  // 软件看门狗检查
  if (platformGetMillis() - lastWatchdogReset > WATCHDOG_TIMEOUT) {
    Serial.println("软件看门狗超时，系统将重启");
    Serial.flush();
    platformDelay(1000);
    platformReset(); // 重启系统
  }
  
  // 定期监控硬件资源使用情况
  static unsigned long lastHardwareMonitor = 0;
  static const unsigned long HARDWARE_MONITOR_INTERVAL = 60000; // 60秒
  
  if (now - lastHardwareMonitor > HARDWARE_MONITOR_INTERVAL) {
    lastHardwareMonitor = now;
    try {
      HardwareDetector* hardwareDetector = HardwareDetector::getInstance();
      hardwareDetector->monitorResources();
      
      // 监控功能状态
      FeatureManager* featureManager = FeatureManager::getInstance();
      featureManager->monitorFeatures();
      
      // 运行性能监控周期
      PerformanceMonitor* performanceMonitor = PerformanceMonitor::getInstance();
      performanceMonitor->runMonitoringCycle();
      
      // 执行存储系统清理和优化
      StorageManager* storageManager = StorageManager::getInstance();
      storageManager->cleanup();
      storageManager->optimize();
      
      // 每5分钟打印一次性能报告
      static unsigned long lastReportTime = 0;
      if (now - lastReportTime > 300000) { // 5分钟
        lastReportTime = now;
        String performanceReport = performanceMonitor->getPerformanceReport();
        Serial.println(performanceReport);
        
        // 打印存储使用情况
        auto storageUsage = storageManager->getStorageUsage();
        Serial.println("存储使用情况:");
        for (const auto& pair : storageUsage) {
          StorageMediumInfo info = storageManager->getStorageMediumInfo(pair.first);
          Serial.printf("%s: %.2f MB 已使用\n", info.name.c_str(), (float)pair.second / (1024 * 1024));
        }
      }
    } catch (const std::exception& e) {
      coreSystem->sendError("硬件监控异常", 1026, "HardwareDetector");
    }
  }
  
  // 短暂延迟，降低CPU占用
  platformDelay(10);
}