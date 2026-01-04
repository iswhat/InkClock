<?php
/**
 * 消息模型
 */

namespace App\Model;

class Message {
    private $db;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     */
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 发送消息
     * @param array $messageInfo 消息信息
     * @return array 发送结果
     */
    public function sendMessage($messageInfo) {
        $deviceId = $messageInfo['device_id'];
        $sender = isset($messageInfo['sender']) ? $messageInfo['sender'] : 'Unknown';
        $content = $messageInfo['content'];
        $type = isset($messageInfo['type']) ? $messageInfo['type'] : 'text';
        $userId = isset($messageInfo['user_id']) ? $messageInfo['user_id'] : null;
        $messageId = $this->generateMessageId();
        $status = 'unread';
        $createdAt = date('Y-m-d H:i:s');
        
        // 插入消息
        $stmt = $this->db->prepare("INSERT INTO messages (message_id, device_id, user_id, sender, content, type, status, created_at) VALUES (:messageId, :deviceId, :userId, :sender, :content, :type, :status, :createdAt)");
        $stmt->bindValue(':messageId', $messageId, SQLITE3_TEXT);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':sender', $sender, SQLITE3_TEXT);
        $stmt->bindValue(':content', $content, SQLITE3_TEXT);
        $stmt->bindValue(':type', $type, SQLITE3_TEXT);
        $stmt->bindValue(':status', $status, SQLITE3_TEXT);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        $stmt->close();
        
        // 限制设备消息数量
        $this->limitDeviceMessages($deviceId);
        
        return [
            'success' => $result !== false,
            'message_id' => $messageId
        ];
    }
    
    /**
     * 获取设备未读消息
     * @param string $deviceId 设备ID
     * @return array 未读消息列表
     */
    public function getUnreadMessages($deviceId) {
        $stmt = $this->db->prepare("SELECT * FROM messages WHERE device_id = :deviceId AND status = 'unread' ORDER BY created_at DESC");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $messages = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $messages[] = $row;
        }
        $result->finalize();
        $stmt->close();
        
        return $messages;
    }
    
    /**
     * 获取设备所有消息
     * @param string $deviceId 设备ID
     * @param int $limit 限制数量
     * @param int $offset 偏移量
     * @return array 消息列表
     */
    public function getAllMessages($deviceId, $limit = 20, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM messages WHERE device_id = :deviceId ORDER BY created_at DESC LIMIT :limit OFFSET :offset");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':limit', $limit, SQLITE3_INTEGER);
        $stmt->bindValue(':offset', $offset, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $messages = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $messages[] = $row;
        }
        $result->finalize();
        $stmt->close();
        
        return $messages;
    }
    
    /**
     * 标记消息为已读
     * @param string $messageId 消息ID
     * @return array 操作结果
     */
    public function markAsRead($messageId) {
        $stmt = $this->db->prepare("UPDATE messages SET status = 'read', read_at = :readAt WHERE message_id = :messageId");
        $readAt = date('Y-m-d H:i:s');
        $stmt->bindValue(':readAt', $readAt, SQLITE3_TEXT);
        $stmt->bindValue(':messageId', $messageId, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        $stmt->close();
        
        return [
            'success' => $result !== false
        ];
    }
    
    /**
     * 标记所有消息为已读
     * @param string $deviceId 设备ID
     * @return bool 操作结果
     */
    public function markAllAsRead($deviceId) {
        $stmt = $this->db->prepare("UPDATE messages SET status = 'read', read_at = :readAt WHERE device_id = :deviceId AND status = 'unread'");
        $readAt = date('Y-m-d H:i:s');
        $stmt->bindValue(':readAt', $readAt, SQLITE3_TEXT);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        $stmt->close();
        
        return $result !== false;
    }
    
    /**
     * 删除消息
     * @param string $messageId 消息ID
     * @return array 操作结果
     */
    public function deleteMessage($messageId) {
        $stmt = $this->db->prepare("DELETE FROM messages WHERE message_id = :messageId");
        $stmt->bindValue(':messageId', $messageId, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        $stmt->close();
        
        return [
            'success' => $result !== false
        ];
    }
    
    /**
     * 限制设备消息数量
     * @param string $deviceId 设备ID
     */
    private function limitDeviceMessages($deviceId) {
        $maxMessages = 100; // 默认限制每个设备100条消息
        
        // 获取设备消息总数
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM messages WHERE device_id = :deviceId");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        $count = $result->fetchArray(SQLITE3_ASSOC)['count'];
        $result->finalize();
        $stmt->close();
        
        if ($count > $maxMessages) {
            // 删除最旧的消息
            $deleteCount = $count - $maxMessages;
            $stmt = $this->db->prepare("DELETE FROM messages WHERE device_id = :deviceId ORDER BY created_at ASC LIMIT :limit");
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
            $stmt->bindValue(':limit', $deleteCount, SQLITE3_INTEGER);
            $stmt->execute();
            $stmt->close();
        }
    }
    
    /**
     * 获取消息数量
     * @param string $deviceId 设备ID
     * @param string $status 状态筛选
     * @return int 消息数量
     */
    public function getMessageCount($deviceId, $status = '') {
        if ($status) {
            $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM messages WHERE device_id = :deviceId AND status = :status");
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
            $stmt->bindValue(':status', $status, SQLITE3_TEXT);
        } else {
            $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM messages WHERE device_id = :deviceId");
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        }
        $result = $stmt->execute();
        $count = $result->fetchArray(SQLITE3_ASSOC)['count'];
        $result->finalize();
        $stmt->close();
        
        return $count;
    }
    
    /**
     * 删除过期消息
     */
    public function deleteExpiredMessages() {
        $expireDays = 30; // 默认保留30天消息
        $expireDate = date('Y-m-d H:i:s', strtotime("-$expireDays days"));
        
        $stmt = $this->db->prepare("DELETE FROM messages WHERE created_at < :expireDate");
        $stmt->bindValue(':expireDate', $expireDate, SQLITE3_TEXT);
        $stmt->execute();
        $stmt->close();
    }
    
    /**
     * 获取消息详情
     * @param string $messageId 消息ID
     * @return array|null 消息详情
     */
    public function getMessageById($messageId) {
        $stmt = $this->db->prepare("SELECT * FROM messages WHERE message_id = :messageId");
        $stmt->bindValue(':messageId', $messageId, SQLITE3_TEXT);
        $result = $stmt->execute();
        $message = $result->fetchArray(SQLITE3_ASSOC);
        $result->finalize();
        $stmt->close();
        
        return $message;
    }
    
    /**
     * 根据设备ID获取消息
     * @param string $deviceId 设备ID
     * @return array 消息列表
     */
    public function getMessagesByDeviceId($deviceId) {
        $stmt = $this->db->prepare("SELECT * FROM messages WHERE device_id = :deviceId ORDER BY created_at DESC");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $messages = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $messages[] = $row;
        }
        $result->finalize();
        $stmt->close();
        
        return $messages;
    }
    
    /**
     * 根据设备ID列表获取消息
     * @param array $deviceIds 设备ID列表
     * @return array 消息列表
     */
    public function getMessagesByDeviceIds($deviceIds) {
        if (empty($deviceIds)) {
            return [];
        }
        
        $placeholders = rtrim(str_repeat('?,', count($deviceIds)), ',');
        $stmt = $this->db->prepare("SELECT * FROM messages WHERE device_id IN ($placeholders) ORDER BY created_at DESC");
        
        $i = 1;
        foreach ($deviceIds as $deviceId) {
            $stmt->bindValue($i++, $deviceId, SQLITE3_TEXT);
        }
        
        $result = $stmt->execute();
        $messages = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $messages[] = $row;
        }
        $result->finalize();
        $stmt->close();
        
        return $messages;
    }
    
    /**
     * 生成消息ID
     * @return string 消息ID
     */
    private function generateMessageId() {
        return uniqid('msg_', true);
    }
}