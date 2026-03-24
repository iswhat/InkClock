<?php
/**
 * 消息模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class Message extends BaseModel {
    
    private function generateMessageId() {
        return 'msg_' . bin2hex(random_bytes(16));
    }
    
    public function sendMessage($messageInfo) {
        try {
            $deviceId = $messageInfo['device_id'];
            $sender = isset($messageInfo['sender']) ? $messageInfo['sender'] : 'Unknown';
            $content = $messageInfo['content'];
            $type = isset($messageInfo['type']) ? $messageInfo['type'] : 'text';
            $scheduledTime = isset($messageInfo['scheduled_time']) ? $messageInfo['scheduled_time'] : null;
            $messageId = $this->generateMessageId();
            $status = $scheduledTime ? 'pending' : 'unread';
            $syncStatus = 'pending';
            $createdAt = date('Y-m-d H:i:s');
            
            $sql = "INSERT INTO messages (message_id, device_id, sender, content, type, is_read, status, sync_status, sync_attempts, scheduled_time, created_at) VALUES (:messageId, :deviceId, :sender, :content, :type, 0, :status, :syncStatus, 0, :scheduledTime, :createdAt)";
            $params = [
                'messageId' => $messageId,
                'deviceId' => $deviceId,
                'sender' => $sender,
                'content' => $content,
                'type' => $type,
                'status' => $status,
                'syncStatus' => $syncStatus,
                'scheduledTime' => $scheduledTime,
                'createdAt' => $createdAt
            ];
            
            $result = $this->execute($sql, $params);
            
            if (!$result) {
                return ['success' => false, 'error' => '消息发送失败'];
            }
            
            $this->limitDeviceMessages($deviceId);
            
            return [
                'success' => true,
                'message_id' => $messageId,
                'sync_status' => $syncStatus
            ];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
    
    public function getUnreadMessages($deviceId) {
        $sql = "SELECT * FROM messages WHERE device_id = :deviceId AND status = 'unread' ORDER BY created_at DESC";
        $params = ['deviceId' => $deviceId];
        return $this->query($sql, $params);
    }
    
    public function getAllMessages($deviceId, $limit = 20, $offset = 0) {
        $sql = "SELECT * FROM messages WHERE device_id = :deviceId ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = ['deviceId' => $deviceId, 'limit' => $limit, 'offset' => $offset];
        return $this->query($sql, $params);
    }
    
    public function getMessages($deviceId, $limit = 20, $offset = 0) {
        return $this->getAllMessages($deviceId, $limit, $offset);
    }
    
    public function markAsRead($messageId) {
        $readAt = date('Y-m-d H:i:s');
        $sql = "UPDATE messages SET status = 'read', read_at = :readAt WHERE message_id = :messageId";
        $params = ['readAt' => $readAt, 'messageId' => $messageId];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function markAllAsRead($deviceId) {
        $readAt = date('Y-m-d H:i:s');
        $sql = "UPDATE messages SET status = 'read', read_at = :readAt WHERE device_id = :deviceId AND status = 'unread'";
        $params = ['readAt' => $readAt, 'deviceId' => $deviceId];
        return $this->execute($sql, $params) !== false;
    }
    
    public function deleteMessage($messageId) {
        $sql = "DELETE FROM messages WHERE message_id = :messageId";
        $params = ['messageId' => $messageId];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    private function limitDeviceMessages($deviceId) {
        $maxMessages = 100;
        
        $sql = "SELECT COUNT(*) as count FROM messages WHERE device_id = :deviceId";
        $params = ['deviceId' => $deviceId];
        $result = $this->query($sql, $params);
        $count = !empty($result) ? intval($result[0]['count']) : 0;
        
        if ($count > $maxMessages) {
            $deleteCount = $count - $maxMessages;
            $sql = "DELETE FROM messages WHERE device_id = :deviceId AND id IN (SELECT id FROM messages WHERE device_id = :deviceId2 ORDER BY created_at ASC LIMIT :limit)";
            $params = ['deviceId' => $deviceId, 'deviceId2' => $deviceId, 'limit' => $deleteCount];
            $this->execute($sql, $params);
        }
    }
    
    public function getMessageCount($deviceId, $status = '') {
        if ($status) {
            $sql = "SELECT COUNT(*) as count FROM messages WHERE device_id = :deviceId AND status = :status";
            $params = ['deviceId' => $deviceId, 'status' => $status];
        } else {
            $sql = "SELECT COUNT(*) as count FROM messages WHERE device_id = :deviceId";
            $params = ['deviceId' => $deviceId];
        }
        $result = $this->query($sql, $params);
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
    
    public function deleteExpiredMessages() {
        $expireDays = 30;
        $expireDate = date('Y-m-d H:i:s', strtotime("-$expireDays days"));
        $sql = "DELETE FROM messages WHERE created_at < :expireDate";
        $params = ['expireDate' => $expireDate];
        $this->execute($sql, $params);
    }
    
    public function getPendingSyncMessages($deviceId, $limit = 50) {
        $sql = "SELECT * FROM messages WHERE device_id = :deviceId AND sync_status IN ('pending', 'failed') ORDER BY created_at ASC LIMIT :limit";
        $params = ['deviceId' => $deviceId, 'limit' => $limit];
        return $this->query($sql, $params);
    }
    
    public function updateSyncStatus($messageId, $syncStatus) {
        try {
            $syncAttempts = $syncStatus === 'failed' ? 1 : 0;
            $syncTime = $syncStatus === 'synced' ? date('Y-m-d H:i:s') : null;
            
            $sql = "UPDATE messages SET sync_status = :syncStatus, sync_attempts = sync_attempts + :syncAttempts, last_sync_at = :syncTime WHERE message_id = :messageId";
            $params = [
                'syncStatus' => $syncStatus,
                'syncAttempts' => $syncAttempts,
                'syncTime' => $syncTime,
                'messageId' => $messageId
            ];
            
            $result = $this->execute($sql, $params);
            return ['success' => $result !== false];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
    
    public function getMessageById($messageId) {
        $sql = "SELECT * FROM messages WHERE message_id = :messageId";
        $params = ['messageId' => $messageId];
        $result = $this->query($sql, $params);
        return !empty($result) ? $result[0] : null;
    }
    
    public function getMessagesByDeviceId($deviceId) {
        $sql = "SELECT * FROM messages WHERE device_id = :deviceId ORDER BY created_at DESC";
        $params = ['deviceId' => $deviceId];
        return $this->query($sql, $params);
    }
    
    public function getRecentMessages($deviceId, $limit = 10) {
        $sql = "SELECT * FROM messages WHERE device_id = :deviceId ORDER BY created_at DESC LIMIT :limit";
        $params = ['deviceId' => $deviceId, 'limit' => $limit];
        return $this->query($sql, $params);
    }
}
?>
