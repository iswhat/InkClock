<?php
/**
 * 消息控制器
 */

class MessageController extends BaseController {
    /**
     * 发送消息
     */
    public function sendMessage($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('message_send', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 检查目标设备是否属于当前用户
        if (isset($data['device_id'])) {
            require_once __DIR__ . '/../models/User.php';
            $userModel = new User();
            if (!$userModel->isDeviceOwnedByUser($user['id'], $data['device_id'])) {
                $this->response::forbidden();
            }
        }
        
        require_once __DIR__ . '/../models/Message.php';
        $messageModel = new Message();
        $result = $messageModel->sendMessage(
            $data['device_id'],
            $data['content'],
            $data['type'] ?? 'text',
            $user['id']
        );
        
        if ($result['success']) {
            $this->response::success('消息发送成功', array('message_id' => $result['message_id']));
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取设备消息列表
     */
    public function getMessages($params) {
        $deviceId = $params['deviceId'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('message_get_list', array('device_id' => $deviceId));
        
        require_once __DIR__ . '/../models/Message.php';
        $messageModel = new Message();
        $messages = $messageModel->getMessages($deviceId);
        
        $this->response::success('获取成功', $messages);
    }
    
    /**
     * 获取设备未读消息
     */
    public function getUnreadMessages($params) {
        $deviceId = $params['deviceId'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('message_get_unread', array('device_id' => $deviceId));
        
        require_once __DIR__ . '/../models/Message.php';
        $messageModel = new Message();
        $messages = $messageModel->getUnreadMessages($deviceId);
        
        $this->response::success('获取成功', $messages);
    }
    
    /**
     * 标记消息为已读
     */
    public function markAsRead($params) {
        $deviceId = $params['deviceId'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('message_mark_read', array('device_id' => $deviceId));
        
        require_once __DIR__ . '/../models/Message.php';
        $messageModel = new Message();
        $result = $messageModel->markAsRead($deviceId);
        
        if ($result['success']) {
            $this->response::success('标记成功');
        } else {
            $this->response::error('标记失败', 400);
        }
    }
    
    /**
     * 删除消息
     */
    public function deleteMessage($params) {
        $deviceId = $params['deviceId'];
        $messageId = $params['messageId'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('message_delete', array('device_id' => $deviceId, 'message_id' => $messageId));
        
        require_once __DIR__ . '/../models/Message.php';
        $messageModel = new Message();
        $result = $messageModel->deleteMessage($messageId, $deviceId);
        
        if ($result['success']) {
            $this->response::success('删除成功');
        } else {
            $this->response::error('删除失败', 400);
        }
    }
}
?>