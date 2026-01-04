<?php
/**
 * 通知模型
 */
namespace App\Model;

use App\Utils\Database;

class Notification {
    private $db;
    
    public function __construct(Database $database) {
        $this->db = $database->getConnection();
    }
    
    /**
     * 发送通知
     */
    public function sendNotification($userId, $title, $content, $type = 'system') {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO notifications (user_id, title, content, type, status, created_at) VALUES (?, ?, ?, ?, ?, ?)");
        $status = 'unread';
        $stmt->bindValue(1, $userId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $title, SQLITE3_TEXT);
        $stmt->bindValue(3, $content, SQLITE3_TEXT);
        $stmt->bindValue(4, $type, SQLITE3_TEXT);
        $stmt->bindValue(5, $status, SQLITE3_TEXT);
        $stmt->bindValue(6, $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        $notificationId = $this->db->lastInsertRowID();
        $stmt->close();
        
        return array('success' => $result !== false, 'notification_id' => $notificationId);
    }
    
    /**
     * 获取用户的通知列表
     */
    public function getNotifications($userId, $status = null, $limit = 50, $offset = 0) {
        $query = "SELECT * FROM notifications WHERE user_id = ?";
        $params = array($userId);
        $types = array(SQLITE3_INTEGER);
        
        if ($status) {
            $query .= " AND status = ?";
            $params[] = $status;
            $types[] = SQLITE3_TEXT;
        }
        
        $query .= " ORDER BY created_at DESC LIMIT ? OFFSET ?";
        $params[] = $limit;
        $params[] = $offset;
        $types[] = SQLITE3_INTEGER;
        $types[] = SQLITE3_INTEGER;
        
        $stmt = $this->db->prepare($query);
        for ($i = 0; $i < count($params); $i++) {
            $stmt->bindValue($i + 1, $params[$i], $types[$i]);
        }
        $result = $stmt->execute();
        $notifications = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $notifications[] = $row;
        }
        $stmt->close();
        
        return $notifications;
    }
    
    /**
     * 获取通知详情
     */
    public function getNotificationById($notificationId, $userId) {
        $stmt = $this->db->prepare("SELECT * FROM notifications WHERE id = ? AND user_id = ?");
        $stmt->bindValue(1, $notificationId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $notification = $result->fetchArray(SQLITE3_ASSOC);
        $stmt->close();
        
        return $notification;
    }
    
    /**
     * 标记通知为已读
     */
    public function markAsRead($notificationId, $userId) {
        $stmt = $this->db->prepare("UPDATE notifications SET status = ? WHERE id = ? AND user_id = ?");
        $status = 'read';
        $stmt->bindValue(1, $status, SQLITE3_TEXT);
        $stmt->bindValue(2, $notificationId, SQLITE3_INTEGER);
        $stmt->bindValue(3, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $affectedRows = $result !== false ? 1 : 0;
        $stmt->close();
        
        return array('success' => $result !== false, 'updated' => $affectedRows > 0);
    }
    
    /**
     * 标记所有通知为已读
     */
    public function markAllAsRead($userId) {
        $stmt = $this->db->prepare("UPDATE notifications SET status = ? WHERE user_id = ? AND status = ?");
        $readStatus = 'read';
        $unreadStatus = 'unread';
        $stmt->bindValue(1, $readStatus, SQLITE3_TEXT);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $stmt->bindValue(3, $unreadStatus, SQLITE3_TEXT);
        $result = $stmt->execute();
        $affectedRows = $this->db->changes();
        $stmt->close();
        
        return array('success' => $result !== false, 'updated' => $affectedRows);
    }
    
    /**
     * 删除通知
     */
    public function deleteNotification($notificationId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM notifications WHERE id = ? AND user_id = ?");
        $stmt->bindValue(1, $notificationId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $affectedRows = $result !== false ? 1 : 0;
        $stmt->close();
        
        return array('success' => $result !== false, 'deleted' => $affectedRows > 0);
    }
    
    /**
     * 获取未读通知数量
     */
    public function getUnreadCount($userId) {
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM notifications WHERE user_id = ? AND status = ?");
        $status = 'unread';
        $stmt->bindValue(1, $userId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $status, SQLITE3_TEXT);
        $result = $stmt->execute();
        $count = $result->fetchArray(SQLITE3_ASSOC)['count'];
        $stmt->close();
        
        return $count;
    }
}
?>