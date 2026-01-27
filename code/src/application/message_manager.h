#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <Arduino.h>
#include "../coresystem/config.h"
#include "../coresystem/data_types.h"

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

class MessageManager {
public:
  MessageManager();
  ~MessageManager();
  
  void init();
  void update();
  void loop();
  
  // 消息管理功能
  bool addMessage(String sender, String content, MessageType type = MESSAGE_TEXT, MessagePriority priority = MESSAGE_PRIORITY_NORMAL, MessageCategory category = MESSAGE_CATEGORY_GENERAL);
  bool deleteMessage(String id);
  bool markMessageAsRead(String id);
  bool hasNewMessage();
  
  // 获取消息
  MessageData getMessage(String id);
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
  int findMessageIndex(String id);
  bool isValidMessageId(String id);
};

#endif // MESSAGE_MANAGER_H