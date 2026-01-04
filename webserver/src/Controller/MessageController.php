<?php
/**
 * 消息控制器
 */

namespace InkClock\Controller;

use InkClock\Model\Message;

class MessageController extends BaseController {
    /**
     * 发送消息
     */
    public function sendMessage($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('message_send', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 使用服务层发送消息
        $result = $this->messageService->sendMessage($data);
        
        if ($result['success']) {
            $this->response->success('发送成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 获取消息列表
     */
    public function getMessages($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('message_get_list', array('user_id' => $user['id']));
        
        $limit = isset($_GET['limit']) ? (int)$_GET['limit'] : 50;
        $offset = isset($_GET['offset']) ? (int)$_GET['offset'] : 0;
        $status = isset($_GET['status']) ? $_GET['status'] : null;
        
        $messageModel = new Message($this->db);
        $messages = $messageModel->getMessagesByUserId($user['id'], $limit, $offset, $status);
        
        $this->response->success('获取成功', $messages);
    }
    
    /**
     * 获取消息详情
     */
    public function getMessage($params) {
        $user = $this->checkApiPermission(true);
        $messageId = $params['id'];
        $this->logAction('message_get_detail', array('user_id' => $user['id'], 'message_id' => $messageId));
        
        $messageModel = new Message($this->db);
        $message = $messageModel->getMessageById($messageId);
        
        if ($message) {
            $this->response->success('获取成功', $message);
        } else {
            $this->response->error('消息不存在', 404);
        }
    }
    
    /**
     * 更新消息状态
     */
    public function updateMessageStatus($params) {
        $user = $this->checkApiPermission(true);
        $messageId = $params['id'];
        $this->logAction('message_update_status', array('user_id' => $user['id'], 'message_id' => $messageId));
        
        $data = $this->parseRequestBody();
        
        $messageModel = new Message($this->db);
        $result = $messageModel->updateMessageStatus($messageId, $data['status']);
        
        if ($result['success']) {
            $this->response->success('更新成功');
        } else {
            $this->response->error('更新失败', 400);
        }
    }
    
    /**
     * 删除消息
     */
    public function deleteMessage($params) {
        $user = $this->checkApiPermission(true);
        $messageId = $params['id'];
        $this->logAction('message_delete', array('user_id' => $user['id'], 'message_id' => $messageId));
        
        $messageModel = new Message($this->db);
        $result = $messageModel->deleteMessage($messageId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error('删除失败', 400);
        }
    }
}
?>