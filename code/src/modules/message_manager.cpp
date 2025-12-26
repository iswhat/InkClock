#include "message_manager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "time_manager.h"

// 外部全局对象
extern TimeManager timeManager;

MessageManager::MessageManager() {
  // 初始化消息数组
  for (int i = 0; i < MAX_MESSAGES; i++) {
    messages[i].id = -1;
    messages[i].sender = "";
    messages[i].content = "";
    messages[i].type = MESSAGE_TEXT;
    messages[i].status = MESSAGE_UNREAD;
    messages[i].time = "";
    messages[i].valid = false;
  }
  
  // 初始化消息计数
  messageCount = 0;
  nextId = 1;
  
  // 初始化更新标志
  lastUpdate = 0;
  dataUpdated = false;
}

MessageManager::~MessageManager() {
  // 清理资源，保存消息
  saveMessages();
}

void MessageManager::init() {
  DEBUG_PRINTLN("初始化消息管理器...");
  
  // 初始化SPIFFS文件系统（如果未初始化）
  if (!SPIFFS.begin(false)) {
    DEBUG_PRINTLN("SPIFFS初始化失败");
    return;
  }
  
  // 加载保存的消息
  if (!loadMessages()) {
    DEBUG_PRINTLN("加载消息失败，将创建新的消息存储");
    saveMessages();
  }
  
  DEBUG_PRINTLN("消息管理器初始化完成");
  DEBUG_PRINT("当前消息数: ");
  DEBUG_PRINTLN(messageCount);
}

void MessageManager::update() {
  // 定期保存消息
  if (dataUpdated) {
    saveMessages();
    dataUpdated = false;
  }
}

void MessageManager::loop() {
  // 定期更新消息
  static unsigned long lastUpdateCheck = 0;
  if (millis() - lastUpdateCheck > MESSAGE_UPDATE_INTERVAL) {
    lastUpdateCheck = millis();
    update();
  }
}

bool MessageManager::addMessage(String sender, String content, MessageType type) {
  DEBUG_PRINTLN("添加新消息...");
  
  // 检查消息数组是否已满
  if (messageCount >= MAX_MESSAGES) {
    // 删除最旧的消息
    deleteMessage(messages[MAX_MESSAGES - 1].id);
  }
  
  // 获取当前时间
  String timeStr = timeManager.getDateTimeString();
  
  // 创建新消息
  MessageData newMessage;
  newMessage.id = nextId++;
  newMessage.sender = sender;
  newMessage.content = content;
  newMessage.type = type;
  newMessage.status = MESSAGE_UNREAD;
  newMessage.time = timeStr;
  newMessage.valid = true;
  
  // 将新消息添加到数组开头
  for (int i = messageCount; i > 0; i--) {
    messages[i] = messages[i - 1];
  }
  messages[0] = newMessage;
  
  // 更新消息计数
  messageCount++;
  
  // 设置数据更新标志
  dataUpdated = true;
  
  DEBUG_PRINT("消息添加成功，ID: ");
  DEBUG_PRINTLN(newMessage.id);
  
  return true;
}

bool MessageManager::deleteMessage(int id) {
  DEBUG_PRINT("删除消息，ID: ");
  DEBUG_PRINTLN(id);
  
  // 查找消息索引
  int index = findMessageIndex(id);
  if (index == -1) {
    DEBUG_PRINTLN("消息不存在");
    return false;
  }
  
  // 删除消息
  for (int i = index; i < messageCount - 1; i++) {
    messages[i] = messages[i + 1];
  }
  
  // 清空最后一个消息
  messages[messageCount - 1].id = -1;
  messages[messageCount - 1].sender = "";
  messages[messageCount - 1].content = "";
  messages[messageCount - 1].type = MESSAGE_TEXT;
  messages[messageCount - 1].status = MESSAGE_UNREAD;
  messages[messageCount - 1].time = "";
  messages[messageCount - 1].valid = false;
  
  // 更新消息计数
  messageCount--;
  
  // 设置数据更新标志
  dataUpdated = true;
  
  DEBUG_PRINTLN("消息删除成功");
  
  return true;
}

bool MessageManager::markMessageAsRead(int id) {
  DEBUG_PRINT("标记消息为已读，ID: ");
  DEBUG_PRINTLN(id);
  
  // 查找消息索引
  int index = findMessageIndex(id);
  if (index == -1) {
    DEBUG_PRINTLN("消息不存在");
    return false;
  }
  
  // 更新消息状态
  messages[index].status = MESSAGE_READ;
  
  // 设置数据更新标志
  dataUpdated = true;
  
  DEBUG_PRINTLN("消息标记为已读");
  
  return true;
}

bool MessageManager::hasNewMessage() {
  // 检查是否有未读消息
  for (int i = 0; i < messageCount; i++) {
    if (messages[i].status == MESSAGE_UNREAD) {
      return true;
    }
  }
  return false;
}

MessageData MessageManager::getMessage(int id) {
  // 查找消息
  int index = findMessageIndex(id);
  if (index != -1) {
    return messages[index];
  }
  
  // 返回无效消息
  MessageData invalidMessage;
  invalidMessage.id = -1;
  invalidMessage.sender = "";
  invalidMessage.content = "";
  invalidMessage.type = MESSAGE_TEXT;
  invalidMessage.status = MESSAGE_UNREAD;
  invalidMessage.time = "";
  invalidMessage.valid = false;
  
  return invalidMessage;
}

MessageData MessageManager::getLatestMessage() {
  if (messageCount > 0) {
    return messages[0];
  }
  
  // 返回无效消息
  MessageData invalidMessage;
  invalidMessage.id = -1;
  invalidMessage.sender = "";
  invalidMessage.content = "";
  invalidMessage.type = MESSAGE_TEXT;
  invalidMessage.status = MESSAGE_UNREAD;
  invalidMessage.time = "";
  invalidMessage.valid = false;
  
  return invalidMessage;
}

int MessageManager::getMessageCount() {
  return messageCount;
}

int MessageManager::getUnreadMessageCount() {
  int count = 0;
  for (int i = 0; i < messageCount; i++) {
    if (messages[i].status == MESSAGE_UNREAD) {
      count++;
    }
  }
  return count;
}

bool MessageManager::saveMessages() {
  DEBUG_PRINTLN("保存消息到文件...");
  
  // 创建JSON文档
  DynamicJsonDocument doc(4096);
  
  // 添加消息数组
  JsonArray messageArray = doc.createNestedArray("messages");
  
  for (int i = 0; i < messageCount; i++) {
    JsonObject messageObj = messageArray.createNestedObject();
    messageObj["id"] = messages[i].id;
    messageObj["sender"] = messages[i].sender;
    messageObj["content"] = messages[i].content;
    messageObj["type"] = messages[i].type;
    messageObj["status"] = messages[i].status;
    messageObj["time"] = messages[i].time;
  }
  
  // 添加元数据
  doc["nextId"] = nextId;
  doc["messageCount"] = messageCount;
  
  // 打开文件
  File file = SPIFFS.open("/messages.json", FILE_WRITE);
  if (!file) {
    DEBUG_PRINTLN("无法打开消息文件进行写入");
    return false;
  }
  
  // 序列化JSON到文件
  if (serializeJson(doc, file) == 0) {
    DEBUG_PRINTLN("JSON序列化失败");
    file.close();
    return false;
  }
  
  // 关闭文件
  file.close();
  
  DEBUG_PRINTLN("消息保存成功");
  return true;
}

bool MessageManager::loadMessages() {
  DEBUG_PRINTLN("从文件加载消息...");
  
  // 检查文件是否存在
  if (!SPIFFS.exists("/messages.json")) {
    DEBUG_PRINTLN("消息文件不存在");
    return false;
  }
  
  // 打开文件
  File file = SPIFFS.open("/messages.json", FILE_READ);
  if (!file) {
    DEBUG_PRINTLN("无法打开消息文件进行读取");
    return false;
  }
  
  // 创建JSON文档
  DynamicJsonDocument doc(4096);
  
  // 从文件反序列化JSON
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    DEBUG_PRINT("JSON反序列化失败: ");
    DEBUG_PRINTLN(error.c_str());
    file.close();
    return false;
  }
  
  // 关闭文件
  file.close();
  
  // 加载消息数组
  JsonArray messageArray = doc["messages"];
  
  for (JsonObject messageObj : messageArray) {
    // 检查消息数组是否已满
    if (messageCount >= MAX_MESSAGES) {
      break;
    }
    
    // 解析消息
    MessageData message;
    message.id = messageObj["id"];
    message.sender = messageObj["sender"].as<String>();
    message.content = messageObj["content"].as<String>();
    message.type = (MessageType)messageObj["type"];
    message.status = (MessageStatus)messageObj["status"];
    message.time = messageObj["time"].as<String>();
    message.valid = true;
    
    // 添加到消息数组
    messages[messageCount] = message;
    messageCount++;
  }
  
  // 加载元数据
  nextId = doc["nextId"];
  
  DEBUG_PRINT("消息加载成功，共加载 ");
  DEBUG_PRINT(messageCount);
  DEBUG_PRINTLN(" 条消息");
  
  return true;
}

void MessageManager::sortMessages() {
  // 按时间顺序排序消息（最新的在前面）
  // 这里假设消息已经按照时间顺序添加，不需要再次排序
  // 如果需要，可以实现冒泡排序或其他排序算法
}

int MessageManager::findMessageIndex(int id) {
  // 查找消息索引
  for (int i = 0; i < messageCount; i++) {
    if (messages[i].id == id) {
      return i;
    }
  }
  return -1;
}

bool MessageManager::isValidMessageId(int id) {
  // 检查消息ID是否有效
  return findMessageIndex(id) != -1;
}