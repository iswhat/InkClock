<?php
/**
 * 通知控制器
 */

namespace InkClock\Controller;

use InkClock\Model\Notification as NotificationModel;

class NotificationController extends BaseController {
    /**
     * 获取通知列表
     */
    public function getNotifications($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_get_list', array('user_id' => $user['id']));
        
        $status = isset($_GET['status']) ? $_GET['status'] : null;
        $limit = isset($_GET['limit']) ? (int)$_GET['limit'] : 50;
        $offset = isset($_GET['offset']) ? (int)$_GET['offset'] : 0;
        
        $notificationModel = new NotificationModel($this->db);
        $notifications = $notificationModel->getNotifications($user['id'], $status, $limit, $offset);
        
        $this->response->success('获取成功', $notifications);
    }
    
    /**
     * 获取通知详情
     */
    public function getNotification($params) {
        $user = $this->checkApiPermission(true);
        $notificationId = $params['id'];
        $this->logAction('notification_get_detail', array('user_id' => $user['id'], 'notification_id' => $notificationId));
        
        $notificationModel = new NotificationModel($this->db);
        $notification = $notificationModel->getNotificationById($notificationId, $user['id']);
        
        if ($notification) {
            $this->response->success('获取成功', $notification);
        } else {
            $this->response->error('通知不存在', 404);
        }
    }
    
    /**
     * 标记通知为已读
     */
    public function markAsRead($params) {
        $user = $this->checkApiPermission(true);
        $notificationId = $params['id'];
        $this->logAction('notification_mark_read', array('user_id' => $user['id'], 'notification_id' => $notificationId));
        
        $notificationModel = new NotificationModel($this->db);
        $result = $notificationModel->markAsRead($notificationId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('标记成功');
        } else {
            $this->response->error('标记失败', 400);
        }
    }
    
    /**
     * 标记所有通知为已读
     */
    public function markAllAsRead($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_mark_all_read', array('user_id' => $user['id']));
        
        $notificationModel = new NotificationModel($this->db);
        $result = $notificationModel->markAllAsRead($user['id']);
        
        if ($result['success']) {
            $this->response->success('标记成功', array('updated' => $result['updated']));
        } else {
            $this->response->error('标记失败', 400);
        }
    }
    
    /**
     * 删除通知
     */
    public function deleteNotification($params) {
        $user = $this->checkApiPermission(true);
        $notificationId = $params['id'];
        $this->logAction('notification_delete', array('user_id' => $user['id'], 'notification_id' => $notificationId));
        
        $notificationModel = new NotificationModel($this->db);
        $result = $notificationModel->deleteNotification($notificationId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error('删除失败', 400);
        }
    }
    
    /**
     * 获取未读通知数量
     */
    public function getUnreadCount($params) {
        $user = $this->checkApiPermission(true);
        
        $notificationModel = new NotificationModel($this->db);
        $count = $notificationModel->getUnreadCount($user['id']);
        
        $this->response->success('获取成功', array('unread_count' => $count));
    }
    
    /**
     * 发布通知
     */
    public function publishNotification($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_publish', array('user_id' => $user['id']));
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $data = $this->parseRequestBody();
        
        // 验证必要参数
        if (empty($data['title']) || empty($data['content'])) {
            $this->response->error('标题和内容不能为空', 400);
        }
        
        $title = $data['title'];
        $content = $data['content'];
        $type = $data['type'] ?? 'system';
        $targetUserId = $data['user_id'] ?? null;
        
        $notificationModel = new NotificationModel($this->db);
        
        if ($targetUserId) {
            // 发送给特定用户
            $result = $notificationModel->sendNotification($targetUserId, $title, $content, $type);
        } else {
            // 发送给所有用户
            $result = $notificationModel->sendNotificationToAll($title, $content, $type);
        }
        
        if ($result['success']) {
            $this->response->success('发布成功', array('notification_id' => $result['notification_id'] ?? null, 'total_sent' => $result['total_sent'] ?? 1));
        } else {
            $this->response->error('发布失败', 400);
        }
    }
    
    /**
     * 获取通知统计数据
     */
    public function getNotificationStats($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_get_stats', array('user_id' => $user['id']));
        
        $notificationModel = new NotificationModel($this->db);
        
        // 获取用户的通知统计数据
        $totalNotifications = $notificationModel->getTotalNotificationsByUser($user['id']);
        $unreadNotifications = $notificationModel->getUnreadCount($user['id']);
        $readNotifications = $totalNotifications - $unreadNotifications;
        $systemNotifications = $notificationModel->getSystemNotificationsCount($user['id']);
        
        $stats = array(
            'total' => $totalNotifications,
            'unread' => $unreadNotifications,
            'read' => $readNotifications,
            'system' => $systemNotifications
        );
        
        $this->response->success('获取成功', $stats);
    }
}
?>