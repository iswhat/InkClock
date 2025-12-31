<?php
/**
 * 通知模型
 */
require_once __DIR__ . '/../utils/Database.php';

class Notification {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
    }
    
    /**
     * 发送通知
     */
    public function sendNotification($userId, $title, $content, $type = 'system') {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO notifications (user_id, title, content, type, status, created_at) VALUES (?, ?, ?, ?, ?, ?)");
        $status = 'unread';
        $stmt->bind_param("isssss", $userId, $title, $content, $type, $status, $createdAt);
        $result = $stmt->execute();
        $notificationId = $this->db->insert_id;
        $stmt->close();
        
        return array('success' => $result, 'notification_id' => $notificationId);
    }
    
    /**
     * 获取用户的通知列表
     */
    public function getNotifications($userId, $status = null, $limit = 50, $offset = 0) {
        $query = "SELECT * FROM notifications WHERE user_id = ?";
        $params = array($userId);
        $types = "i";
        
        if ($status) {
            $query .= " AND status = ?";
            $params[] = $status;
            $types .= "s";
        }
        
        $query .= " ORDER BY created_at DESC LIMIT ? OFFSET ?";
        $params[] = $limit;
        $params[] = $offset;
        $types .= "ii";
        
        $stmt = $this->db->prepare($query);
        $stmt->bind_param($types, ...$params);
        $stmt->execute();
        $result = $stmt->get_result();
        $notifications = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
        return $notifications;
    }
    
    /**
     * 获取通知详情
     */
    public function getNotificationById($notificationId, $userId) {
        $stmt = $this->db->prepare("SELECT * FROM notifications WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ii", $notificationId, $userId);
        $stmt->execute();
        $result = $stmt->get_result();
        $notification = $result->fetch_assoc();
        $stmt->close();
        
        return $notification;
    }
    
    /**
     * 标记通知为已读
     */
    public function markAsRead($notificationId, $userId) {
        $stmt = $this->db->prepare("UPDATE notifications SET status = ? WHERE id = ? AND user_id = ?");
        $status = 'read';
        $stmt->bind_param("sii", $status, $notificationId, $userId);
        $result = $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $result, 'updated' => $affectedRows > 0);
    }
    
    /**
     * 标记所有通知为已读
     */
    public function markAllAsRead($userId) {
        $stmt = $this->db->prepare("UPDATE notifications SET status = ? WHERE user_id = ? AND status = ?");
        $readStatus = 'read';
        $unreadStatus = 'unread';
        $stmt->bind_param("sis", $readStatus, $userId, $unreadStatus);
        $result = $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $result, 'updated' => $affectedRows);
    }
    
    /**
     * 删除通知
     */
    public function deleteNotification($notificationId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM notifications WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ii", $notificationId, $userId);
        $result = $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $result, 'deleted' => $affectedRows > 0);
    }
    
    /**
     * 获取未读通知数量
     */
    public function getUnreadCount($userId) {
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM notifications WHERE user_id = ? AND status = ?");
        $status = 'unread';
        $stmt->bind_param("is", $userId, $status);
        $stmt->execute();
        $result = $stmt->get_result();
        $count = $result->fetch_assoc()['count'];
        $stmt->close();
        
        return $count;
    }
}
?>