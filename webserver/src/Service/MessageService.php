<?php
/**
 * 消息服务类
 */

namespace InkClock\Service;

use InkClock\Utils\Logger;
use InkClock\Model\User;
use InkClock\Model\Message;
use InkClock\Interfaces\MessageServiceInterface;

class MessageService implements MessageServiceInterface {
    private $db;
    private $logger;

    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     * @param Logger $logger 日志服务
     */
    public function __construct($db, Logger $logger) {
        $this->db = $db;
        $this->logger = $logger;
    }
    
    /**
     * 发送消息
     * @param string $deviceId 设备ID
     * @param string $sender 发送者
     * @param string $content 消息内容
     * @param string $type 消息类型
     * @return array 发送结果
     */
    public function sendMessage($deviceId, $sender, $content, $type = 'text') {
        // 构建消息信息数组
        $messageInfo = [
            'device_id' => $deviceId,
            'sender' => $sender,
            'content' => $content,
            'type' => $type
        ];
        
        $this->logger->info('发送消息请求', ['message' => $messageInfo]);
        
        // 验证消息信息
        if (!$this->validateMessageInfo($messageInfo)) {
            $this->logger->warning('消息验证失败', $messageInfo);
            return ['success' => false, 'error' => '无效的消息信息'];
        }
        
        // 调用模型发送消息
        $messageModel = new Message($this->db);
        $result = $messageModel->sendMessage($messageInfo);
        
        if ($result['success']) {
            $this->logger->info('消息发送成功', ['message_id' => $result['message_id'], 'device_id' => $deviceId]);
        } else {
            $this->logger->warning('消息发送失败', ['error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 获取消息列表
     * @param array $filters 过滤条件
     * @return array 消息列表
     */
    public function getMessageList($filters = []) {
        return $this->getDeviceMessages($filters['device_id'] ?? '', $filters);
    }
    
    /**
     * 获取设备消息
     * @param string $deviceId 设备ID
     * @param array $filters 过滤条件
     * @return array 消息列表
     */
    public function getDeviceMessages($deviceId, $filters = []) {
        $this->logger->info('获取设备消息请求', ['device_id' => $deviceId, 'filters' => $filters]);
        
        // 获取设备消息
        $messageModel = new Message($this->db);
        $messages = $messageModel->getMessagesByDeviceId($deviceId);
        
        $this->logger->info('获取设备消息成功', ['count' => count($messages)]);
        
        return ['success' => true, 'messages' => $messages];
    }
    
    /**
     * 标记消息为已读
     * @param string $messageId 消息ID
     * @return array 操作结果
     */
    public function markMessageAsRead($messageId) {
        $this->logger->info('标记消息为已读请求', ['message_id' => $messageId]);
        
        $messageModel = new Message($this->db);
        $result = $messageModel->markAsRead($messageId);
        
        if ($result['success']) {
            $this->logger->info('消息标记已读成功', ['message_id' => $messageId]);
        } else {
            $this->logger->warning('消息标记已读失败', ['message_id' => $messageId]);
        }
        
        return $result;
    }
    
    /**
     * 删除消息
     * @param string $messageId 消息ID
     * @return array 删除结果
     */
    public function deleteMessage($messageId) {
        $this->logger->info('删除消息请求', ['message_id' => $messageId]);
        
        $messageModel = new Message($this->db);
        $result = $messageModel->deleteMessage($messageId);
        
        if ($result['success']) {
            $this->logger->info('消息删除成功', ['message_id' => $messageId]);
        } else {
            $this->logger->warning('消息删除失败', ['message_id' => $messageId]);
        }
        
        return $result;
    }
    
    /**
     * 获取未读消息数
     * @param string $deviceId 设备ID
     * @return int 未读消息数
     */
    public function getUnreadMessageCount($deviceId) {
        $this->logger->info('获取未读消息数请求', ['device_id' => $deviceId]);
        
        $messageModel = new Message($this->db);
        $count = $messageModel->getMessageCount($deviceId, 'unread');
        
        $this->logger->info('获取未读消息数成功', ['count' => $count]);
        
        return $count;
    }
    
    /**
     * 批量标记消息为已读
     * @param array $messageIds 消息ID数组
     * @return array 操作结果
     */
    public function batchMarkAsRead($messageIds) {
        $this->logger->info('批量标记消息为已读请求', ['count' => count($messageIds)]);
        
        $messageModel = new Message($this->db);
        $successCount = 0;
        
        foreach ($messageIds as $messageId) {
            $result = $messageModel->markAsRead($messageId);
            if ($result['success']) {
                $successCount++;
            }
        }
        
        $this->logger->info('批量标记消息已读完成', ['success' => $successCount, 'total' => count($messageIds)]);
        
        return ['success' => true, 'count' => $successCount];
    }
    
    /**
     * 批量删除消息
     * @param array $messageIds 消息ID数组
     * @return array 删除结果
     */
    public function batchDeleteMessages($messageIds) {
        $this->logger->info('批量删除消息请求', ['count' => count($messageIds)]);
        
        $messageModel = new Message($this->db);
        $successCount = 0;
        
        foreach ($messageIds as $messageId) {
            $result = $messageModel->deleteMessage($messageId);
            if ($result['success']) {
                $successCount++;
            }
        }
        
        $this->logger->info('批量删除消息完成', ['success' => $successCount, 'total' => count($messageIds)]);
        
        return ['success' => true, 'count' => $successCount];
    }
    
    /**
     * 批量发送消息
     * @param int $userId 用户ID
     * @param array $batchInfo 批量消息信息
     * @return array 发送结果
     */
    public function sendBatchMessages($userId, $batchInfo) {
        $this->logger->info('批量发送消息请求', ['user_id' => $userId, 'batch' => $batchInfo]);
        
        $results = [];
        
        // 验证批量消息信息
        if (empty($batchInfo['device_ids']) || empty($batchInfo['content'])) {
            return ['success' => false, 'error' => '无效的批量消息信息'];
        }
        
        // 验证每个设备的所有权
        $userModel = new User($this->db);
        foreach ($batchInfo['device_ids'] as $deviceId) {
            if (!$userModel->isDeviceOwnedByUser($userId, $deviceId)) {
                return ['success' => false, 'error' => "无权向设备 {$deviceId} 发送消息"];
            }
        }
        
        // 批量发送消息
        $messageModel = new Message($this->db);
        foreach ($batchInfo['device_ids'] as $deviceId) {
            $messageInfo = [
                'device_id' => $deviceId,
                'content' => $batchInfo['content'],
                'type' => $batchInfo['type'] ?? 'text',
                'sender' => 'user_' . $userId
            ];
            
            $result = $messageModel->sendMessage($messageInfo);
            $results[$deviceId] = $result;
        }
        
        $successCount = count(array_filter($results, function($r) { return $r['success']; }));
        $totalCount = count($results);
        
        $this->logger->info('批量发送消息完成', ['success' => $successCount, 'total' => $totalCount]);
        
        return ['success' => true, 'results' => $results, 'summary' => [
            'success' => $successCount,
            'total' => $totalCount
        ]];
    }
    
    /**
     * 验证消息信息
     * @param array $messageInfo 消息信息
     * @return bool 验证结果
     */
    private function validateMessageInfo($messageInfo) {
        // 验证设备ID
        if (empty($messageInfo['device_id'])) {
            return false;
        }
        
        // 验证消息内容
        if (empty($messageInfo['content'])) {
            return false;
        }
        
        // 验证消息类型
        $validTypes = ['text', 'image', 'audio'];
        if (!isset($messageInfo['type']) || !in_array($messageInfo['type'], $validTypes)) {
            $messageInfo['type'] = 'text';
        }
        
        return true;
    }
}