<?php
/**
 * 通知控制器
 */

class NotificationController extends BaseController {
    /**
     * 获取通知列表
     */
    public function getNotifications($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/Notification.php';
        $notificationModel = new Notification();
        $notifications = $notificationModel->getNotifications(
            $user['id'],
            $this->getQueryParams(array('page', 'limit'))
        );
        
        $this->response::success('获取成功', $notifications);
    }
    
    /**
     * 获取通知详情
     */
    public function getNotification($params) {
        $notification_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_get_detail', array('notification_id' => $notification_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/Notification.php';
        $notificationModel = new Notification();
        $notification = $notificationModel->getNotification($notification_id);
        
        if (!$notification) {
            $this->response::error('通知不存在', 404);
        }
        
        // 检查通知是否属于当前用户
        if ($notification['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $this->response::success('获取成功', $notification);
    }
    
    /**
     * 获取未读通知数量
     */
    public function getUnreadCount($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_get_unread_count', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/Notification.php';
        $notificationModel = new Notification();
        $count = $notificationModel->getUnreadCount($user['id']);
        
        $this->response::success('获取成功', array('unread_count' => $count));
    }
    
    /**
     * 标记所有通知为已读
     */
    public function markAllAsRead($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_mark_all_read', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/Notification.php';
        $notificationModel = new Notification();
        $result = $notificationModel->markAllAsRead($user['id']);
        
        if ($result['success']) {
            $this->response::success('所有通知已标记为已读');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 标记单个通知为已读
     */
    public function markAsRead($params) {
        $notification_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_mark_read', array('notification_id' => $notification_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/Notification.php';
        $notificationModel = new Notification();
        $notification = $notificationModel->getNotification($notification_id);
        
        if (!$notification) {
            $this->response::error('通知不存在', 404);
        }
        
        // 检查通知是否属于当前用户
        if ($notification['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $notificationModel->markAsRead($notification_id);
        
        if ($result['success']) {
            $this->response::success('通知已标记为已读');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 删除通知
     */
    public function deleteNotification($params) {
        $notification_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('notification_delete', array('notification_id' => $notification_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/Notification.php';
        $notificationModel = new Notification();
        $notification = $notificationModel->getNotification($notification_id);
        
        if (!$notification) {
            $this->response::error('通知不存在', 404);
        }
        
        // 检查通知是否属于当前用户
        if ($notification['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $notificationModel->deleteNotification($notification_id);
        
        if ($result['success']) {
            $this->response::success('通知已删除');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
}
?>