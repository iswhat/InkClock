<?php
/**
 * 通知模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class Notification extends BaseModel {
    
    public function sendNotification($userId, $title, $content, $type = 'system') {
        $createdAt = date('Y-m-d H:i:s');
        $status = 'unread';
        
        $sql = "INSERT INTO notifications (user_id, title, content, type, status, created_at) VALUES (:userId, :title, :content, :type, :status, :createdAt)";
        $params = [
            'userId' => $userId,
            'title' => $title,
            'content' => $content,
            'type' => $type,
            'status' => $status,
            'createdAt' => $createdAt
        ];
        
        $result = $this->execute($sql, $params);
        $notificationId = $this->lastInsertId();
        
        return ['success' => $result !== false, 'notification_id' => $notificationId];
    }
    
    public function getNotifications($userId, $status = null, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM notifications WHERE user_id = :userId";
        $params = ['userId' => $userId];
        
        if ($status) {
            $sql .= " AND status = :status";
            $params['status'] = $status;
        }
        
        $sql .= " ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params['limit'] = $limit;
        $params['offset'] = $offset;
        
        return $this->query($sql, $params);
    }
    
    public function getNotificationById($notificationId, $userId) {
        $sql = "SELECT * FROM notifications WHERE id = :id AND user_id = :userId";
        $params = ['id' => $notificationId, 'userId' => $userId];
        $result = $this->query($sql, $params);
        return !empty($result) ? $result[0] : null;
    }
    
    public function markAsRead($notificationId, $userId) {
        $sql = "UPDATE notifications SET status = :status WHERE id = :id AND user_id = :userId";
        $params = ['status' => 'read', 'id' => $notificationId, 'userId' => $userId];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false, 'updated' => $result !== false];
    }
    
    public function markAllAsRead($userId) {
        $sql = "UPDATE notifications SET status = :status WHERE user_id = :userId AND status = :oldStatus";
        $params = ['status' => 'read', 'userId' => $userId, 'oldStatus' => 'unread'];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false, 'updated' => $result !== false];
    }
    
    public function deleteNotification($notificationId, $userId) {
        $sql = "DELETE FROM notifications WHERE id = :id AND user_id = :userId";
        $params = ['id' => $notificationId, 'userId' => $userId];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false, 'deleted' => $result !== false];
    }
    
    public function getUnreadCount($userId) {
        $sql = "SELECT COUNT(*) as count FROM notifications WHERE user_id = :userId AND status = :status";
        $params = ['userId' => $userId, 'status' => 'unread'];
        $result = $this->query($sql, $params);
        if (!empty($result)) {
            return intval($result[0]['count']);
        }
        return 0;
    }
    
    public function sendNotificationToAll($title, $content, $type = 'system') {
        try {
            $userResult = $this->query("SELECT id FROM users");
            $userIds = [];
            foreach ($userResult as $row) {
                $userIds[] = $row['id'];
            }
            
            $totalSent = 0;
            $createdAt = date('Y-m-d H:i:s');
            
            foreach ($userIds as $userId) {
                $sql = "INSERT INTO notifications (user_id, title, content, type, status, created_at) VALUES (:userId, :title, :content, :type, :status, :createdAt)";
                $params = [
                    'userId' => $userId,
                    'title' => $title,
                    'content' => $content,
                    'type' => $type,
                    'status' => 'unread',
                    'createdAt' => $createdAt
                ];
                $result = $this->execute($sql, $params);
                if ($result !== false) {
                    $totalSent++;
                }
            }
            
            return ['success' => $totalSent > 0, 'total_sent' => $totalSent];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
    
    public function getTotalNotifications() {
        $result = $this->query("SELECT COUNT(*) as count FROM notifications");
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
}
?>
