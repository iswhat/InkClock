<?php
/**
 * 消息模型
 */
require_once '../config.php';

class Message {
    private $db;
    
    public function __construct() {
        $this->db = getDbConnection();
    }
    
    /**
     * 发送消息
     */
    public function sendMessage($messageInfo) {
        $deviceId = $messageInfo['device_id'];
        $sender = isset($messageInfo['sender']) ? $messageInfo['sender'] : 'Unknown';
        $content = $messageInfo['content'];
        $type = isset($messageInfo['type']) ? $messageInfo['type'] : 'text'; // text, image, audio
        $messageId = generateMessageId();
        $status = 'unread';
        $createdAt = date('Y-m-d H:i:s');
        
        // 插入消息
        $stmt = $this->db->prepare("INSERT INTO messages (message_id, device_id, sender, content, type, status, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)");
        $stmt->bind_param("sssssss", $messageId, $deviceId, $sender, $content, $type, $status, $createdAt);
        $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        // 限制设备消息数量
        $this->limitDeviceMessages($deviceId);
        
        return array(
            'success' => $affectedRows > 0,
            'message_id' => $messageId
        );
    }
    
    /**
     * 获取设备未读消息
     */
    public function getUnreadMessages($deviceId) {
        $stmt = $this->db->prepare("SELECT * FROM messages WHERE device_id = ? AND status = 'unread' ORDER BY created_at DESC");
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $messages = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        return $messages;
    }
    
    /**
     * 获取设备所有消息
     */
    public function getAllMessages($deviceId, $limit = 20, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM messages WHERE device_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bind_param("sii", $deviceId, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $messages = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        return $messages;
    }
    
    /**
     * 标记消息为已读
     */
    public function markAsRead($messageId, $deviceId) {
        $stmt = $this->db->prepare("UPDATE messages SET status = 'read', read_at = ? WHERE message_id = ? AND device_id = ?");
        $readAt = date('Y-m-d H:i:s');
        $stmt->bind_param("sss", $readAt, $messageId, $deviceId);
        $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        return $affectedRows > 0;
    }
    
    /**
     * 标记所有消息为已读
     */
    public function markAllAsRead($deviceId) {
        $stmt = $this->db->prepare("UPDATE messages SET status = 'read', read_at = ? WHERE device_id = ? AND status = 'unread'");
        $readAt = date('Y-m-d H:i:s');
        $stmt->bind_param("ss", $readAt, $deviceId);
        $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        return $affectedRows > 0;
    }
    
    /**
     * 删除消息
     */
    public function deleteMessage($messageId, $deviceId) {
        $stmt = $this->db->prepare("DELETE FROM messages WHERE message_id = ? AND device_id = ?");
        $stmt->bind_param("ss", $messageId, $deviceId);
        $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        return $affectedRows > 0;
    }
    
    /**
     * 限制设备消息数量
     */
    private function limitDeviceMessages($deviceId) {
        global $config;
        $maxMessages = $config['device']['max_messages_per_device'];
        
        // 获取设备消息总数
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM messages WHERE device_id = ?");
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $count = $result->fetch_assoc()['count'];
        $stmt->close();
        
        if ($count > $maxMessages) {
            // 删除最旧的消息
            $deleteCount = $count - $maxMessages;
            $stmt = $this->db->prepare("DELETE FROM messages WHERE device_id = ? ORDER BY created_at ASC LIMIT ?");
            $stmt->bind_param("si", $deviceId, $deleteCount);
            $stmt->execute();
            $stmt->close();
        }
    }
    
    /**
     * 获取消息数量
     */
    public function getMessageCount($deviceId, $status = '') {
        if ($status) {
            $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM messages WHERE device_id = ? AND status = ?");
            $stmt->bind_param("ss", $deviceId, $status);
        } else {
            $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM messages WHERE device_id = ?");
            $stmt->bind_param("s", $deviceId);
        }
        $stmt->execute();
        $result = $stmt->get_result();
        $count = $result->fetch_assoc()['count'];
        $stmt->close();
        return $count;
    }
    
    /**
     * 删除过期消息
     */
    public function deleteExpiredMessages() {
        global $config;
        $expireDays = $config['device']['message_expire_days'];
        $expireDate = date('Y-m-d H:i:s', strtotime("-$expireDays days"));
        
        $stmt = $this->db->prepare("DELETE FROM messages WHERE created_at < ?");
        $stmt->bind_param("s", $expireDate);
        $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        return $affectedRows;
    }
}
?>