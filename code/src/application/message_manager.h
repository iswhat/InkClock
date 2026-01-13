#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <Arduino.h>
#include "../coresystem/config.h"

// 消息类型枚举
enum MessageType {
  MESSAGE_TEXT,
  MESSAGE_AUDIO,
  MESSAGE_IMAGE
};

// 消息状态枚举
enum MessageStatus {
  MESSAGE_UNREAD,
  MESSAGE_READ
};

// 消息优先级枚举
enum MessagePriority {
  MESSAGE_PRIORITY_LOW,    // 低优先级
  MESSAGE_PRIORITY_NORMAL, // 普通优先级
  MESSAGE_PRIORITY_HIGH,   // 高优先级
  MESSAGE_PRIORITY_URGENT  // 紧急优先级
};

// 消息分类枚举
enum MessageCategory {
  MESSAGE_CATEGORY_GENERAL,   // 通用消息
  MESSAGE_CATEGORY_ALARM,     // 报警消息
  MESSAGE_CATEGORY_NOTIFICATION, // 通知消息
  MESSAGE_CATEGORY_SYSTEM,    // 系统消息
  MESSAGE_CATEGORY_USER,      // 用户消息
  MESSAGE_CATEGORY_WEATHER,   // 天气消息
  MESSAGE_CATEGORY_SENSOR,    // 传感器消息
  MESSAGE_CATEGORY_STOCK,     // 股票消息
  MESSAGE_CATEGORY_CALENDAR   // 日历消息
};

// 消息数据结构
typedef struct {
  int id;                     // 消息ID
  String sender;              // 发送者
  String content;             // 消息内容（文本消息）或文件名（音频消息）
  MessageType type;           // 消息类型
  MessageStatus status;       // 消息状态
  MessagePriority priority;   // 消息优先级
  MessageCategory category;   // 消息分类
  String time;                // 发送时间
  bool valid;                 // 数据是否有效
} MessageData;

class MessageManager {
public:
  MessageManager();
  ~MessageManager();
  
  void init();
  void update();
  void loop();
  
  // 消息管理功能
  bool addMessage(String sender, String content, MessageType type = MESSAGE_TEXT, MessagePriority priority = MESSAGE_PRIORITY_NORMAL, MessageCategory category = MESSAGE_CATEGORY_GENERAL);
  bool deleteMessage(int id);
  bool markMessageAsRead(int id);
  bool hasNewMessage();
  
  // 获取消息
  MessageData getMessage(int id);
  MessageData getLatestMessage();
  int getMessageCount();
  int getUnreadMessageCount();
  
  // 消息分类和过滤功能
  MessageData* getMessagesByCategory(MessageCategory category, int& count);
  MessageData* getMessagesByPriority(MessagePriority priority, int& count);
  MessageData* getMessagesByStatus(MessageStatus status, int& count);
  MessageData* filterMessages(MessageCategory category, MessagePriority priority, MessageStatus status, int& count);
  
  // 消息存储功能
  bool saveMessages();
  bool loadMessages();
  
private:
  // 消息数组
  MessageData messages[MAX_MESSAGES];
  
  // 消息计数
  int messageCount;
  int nextId;
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  
  // 私有方法
  void sortMessages();
  int findMessageIndex(int id);
  bool isValidMessageId(int id);
};

#endif // MESSAGE_MANAGER_H