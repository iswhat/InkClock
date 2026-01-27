#include "bluetooth_manager.h"

// 定义BLE服务和特征UUID
const char* BluetoothManager::SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* BluetoothManager::WIFI_SSID_CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
const char* BluetoothManager::WIFI_PASSWORD_CHARACTERISTIC_UUID = "5b5c15a0-4a37-4415-8c41-f42e1922c4a9";
const char* BluetoothManager::WIFI_STATUS_CHARACTERISTIC_UUID = "81372a9d-7c01-4377-99b2-3593332b0d2c";
const char* BluetoothManager::DEVICE_INFO_CHARACTERISTIC_UUID = "2a24b65b-5566-4477-8899-aabbccddeeff";

// 回调实现
void BluetoothManager::MyServerCallbacks::onConnect(BLEServer* pServer) {
  manager->deviceConnected = true;
  DEBUG_PRINTLN("BLE device connected");
}

void BluetoothManager::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
  manager->deviceConnected = false;
  DEBUG_PRINTLN("BLE device disconnected");
  // 重新启动广播
  pServer->startAdvertising();
}

void BluetoothManager::MyWiFiSSIDCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
  std::string value = pCharacteristic->getValue();
  if (value.length() > 0) {
    manager->wifiSSID = String(value.c_str());
    DEBUG_PRINT("WiFi SSID received: ");
    DEBUG_PRINTLN(manager->wifiSSID);
  }
}

void BluetoothManager::MyWiFiPasswordCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
  std::string value = pCharacteristic->getValue();
  if (value.length() > 0) {
    manager->wifiPassword = String(value.c_str());
    DEBUG_PRINT("WiFi Password received: ");
    DEBUG_PRINTLN(manager->wifiPassword);
    
    // 当同时收到SSID和密码时，标记为已配置并尝试连接
    if (manager->wifiSSID.length() > 0 && manager->wifiPassword.length() > 0) {
      manager->wifiConfigured = true;
      manager->setWiFiConfigStatus(true);
      DEBUG_PRINTLN("WiFi configuration completed");
      
      // 注意：WiFi管理器的使用需要进一步实现，暂时注释掉
      // 目前无法直接访问全局WiFiManager实例
      // WiFiManager wifiManager;
      // wifiManager.setConfiguredWiFi(manager->wifiSSID, manager->wifiPassword);
      // wifiManager.connect(manager->wifiSSID, manager->wifiPassword);
      
      // 发送连接状态通知
      // 暂时使用固定状态，因为WiFi库的使用存在问题
      String status = "Connecting";
      if (manager->pWiFiStatusCharacteristic != nullptr) {
        manager->pWiFiStatusCharacteristic->setValue(status.c_str());
        manager->pWiFiStatusCharacteristic->notify();
      }
    }
  }
}

BluetoothManager::BluetoothManager() {
  pServer = nullptr;
  pService = nullptr;
  pWiFiSSIDCharacteristic = nullptr;
  pWiFiPasswordCharacteristic = nullptr;
  pWiFiStatusCharacteristic = nullptr;
  pDeviceInfoCharacteristic = nullptr;
  
  deviceConnected = false;
  wifiConfigured = false;
  wifiSSID = "";
  wifiPassword = "";
  
  serverCallbacks = new MyServerCallbacks(this);
  wifiSSIDCallbacks = new MyWiFiSSIDCallbacks(this);
  wifiPasswordCallbacks = new MyWiFiPasswordCallbacks(this);
}

BluetoothManager::~BluetoothManager() {
  delete serverCallbacks;
  delete wifiSSIDCallbacks;
  delete wifiPasswordCallbacks;
  
  if (pServer != nullptr) {
    delete pServer;
  }
}

void BluetoothManager::init() {
  DEBUG_PRINTLN("初始化蓝牙管理器...");
  
  // 初始化BLE设备
  BLEDevice::init("InkClock Config");
  
  // 创建BLE服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(serverCallbacks);
  
  // 创建BLE服务
  pService = pServer->createService(SERVICE_UUID);
  
  // 创建WiFi SSID特征（可写）
  pWiFiSSIDCharacteristic = pService->createCharacteristic(
    WIFI_SSID_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pWiFiSSIDCharacteristic->setCallbacks(wifiSSIDCallbacks);
  
  // 创建WiFi密码特征（可写）
  pWiFiPasswordCharacteristic = pService->createCharacteristic(
    WIFI_PASSWORD_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pWiFiPasswordCharacteristic->setCallbacks(wifiPasswordCallbacks);
  
  // 创建WiFi状态特征（可读）
  pWiFiStatusCharacteristic = pService->createCharacteristic(
    WIFI_STATUS_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pWiFiStatusCharacteristic->addDescriptor(new BLE2902());
  
  // 创建设备信息特征（可读）
  pDeviceInfoCharacteristic = pService->createCharacteristic(
    DEVICE_INFO_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  pDeviceInfoCharacteristic->setValue("InkClock v1.0");
  
  // 启动服务
  pService->start();
  
  // 开始广播
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // 表示最小BLE 5.0连接间隔
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  DEBUG_PRINTLN("蓝牙管理器初始化完成，开始广播");
}

void BluetoothManager::loop() {
  // 检查是否有设备连接
  if (deviceConnected) {
    // 处理WiFi配置请求
    if (pWiFiStatusCharacteristic != nullptr) {
      // 暂时使用固定状态，因为WiFi库的使用存在问题
      String status = "Disconnected";
      pWiFiStatusCharacteristic->setValue(status.c_str());
      pWiFiStatusCharacteristic->notify();
    }
  }
}

void BluetoothManager::setWiFiConfigStatus(bool success) {
  if (pWiFiStatusCharacteristic != nullptr) {
    String status = success ? "Configured" : "Failed";
    pWiFiStatusCharacteristic->setValue(status.c_str());
    pWiFiStatusCharacteristic->notify();
    DEBUG_PRINT("WiFi configuration status: ");
    DEBUG_PRINTLN(status);
  }
}

void BluetoothManager::resetWiFiConfig() {
  wifiConfigured = false;
  wifiSSID = "";
  wifiPassword = "";
  DEBUG_PRINTLN("WiFi configuration reset");
  
  // 发送重置状态通知
  if (pWiFiStatusCharacteristic != nullptr) {
    pWiFiStatusCharacteristic->setValue("Reset");
    pWiFiStatusCharacteristic->notify();
  }
}