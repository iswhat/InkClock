#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "core/config.h"

class BluetoothManager {
private:
  BLEServer* pServer;
  BLEService* pService;
  BLECharacteristic* pWiFiSSIDCharacteristic;
  BLECharacteristic* pWiFiPasswordCharacteristic;
  BLECharacteristic* pWiFiStatusCharacteristic;
  BLECharacteristic* pDeviceInfoCharacteristic;
  
  bool deviceConnected;
  bool wifiConfigured;
  String wifiSSID;
  String wifiPassword;
  
  // BLE服务和特征UUID
  static const char* SERVICE_UUID;
  static const char* WIFI_SSID_CHARACTERISTIC_UUID;
  static const char* WIFI_PASSWORD_CHARACTERISTIC_UUID;
  static const char* WIFI_STATUS_CHARACTERISTIC_UUID;
  static const char* DEVICE_INFO_CHARACTERISTIC_UUID;
  
  // 回调类
  class MyServerCallbacks : public BLEServerCallbacks {
    BluetoothManager* manager;
  public:
    MyServerCallbacks(BluetoothManager* mgr) : manager(mgr) {}
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
  };
  
  class MyWiFiSSIDCallbacks : public BLECharacteristicCallbacks {
    BluetoothManager* manager;
  public:
    MyWiFiSSIDCallbacks(BluetoothManager* mgr) : manager(mgr) {}
    void onWrite(BLECharacteristic* pCharacteristic) override;
  };
  
  class MyWiFiPasswordCallbacks : public BLECharacteristicCallbacks {
    BluetoothManager* manager;
  public:
    MyWiFiPasswordCallbacks(BluetoothManager* mgr) : manager(mgr) {}
    void onWrite(BLECharacteristic* pCharacteristic) override;
  };
  
  MyServerCallbacks* serverCallbacks;
  MyWiFiSSIDCallbacks* wifiSSIDCallbacks;
  MyWiFiPasswordCallbacks* wifiPasswordCallbacks;
  
public:
  BluetoothManager();
  ~BluetoothManager();
  
  void init();
  void loop();
  
  // 检查WiFi是否已配置
  bool isWiFiConfigured() { return wifiConfigured; }
  
  // 获取配置的WiFi信息
  String getWiFiSSID() { return wifiSSID; }
  String getWiFiPassword() { return wifiPassword; }
  
  // 设置WiFi配置状态
  void setWiFiConfigStatus(bool success);
  
  // 重置WiFi配置
  void resetWiFiConfig();
};

#endif // BLUETOOTH_MANAGER_H