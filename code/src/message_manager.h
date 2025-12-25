#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <Arduino.h>
#include "config.h"

// 消息类型枚举
enum MessageType {
  MESSAGE_TEXT,
  MESSAGE_AUDIO
};

// 消息状态枚举
enum MessageStatus {
  MESSAGE_UNREAD,
  MESSAGE_READ
};

// 消息数据结构
typedef struct {
  int id;                     // 消息ID
  String sender;              // 发送者
  String content;             // 消息内容（文本消息）或文件名（音频消息）
  MessageType type;           // 消息类型
  MessageStatus status;       // 消息状态
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
  bool addMessage(String sender, String content, MessageType type = MESSAGE_TEXT);
  bool deleteMessage(int id);
  bool markMessageAsRead(int id);
  bool hasNewMessage();
  
  // 获取消息
  MessageData getMessage(int id);
  MessageData getLatestMessage();
  int getMessageCount();
  int getUnreadMessageCount();
  
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