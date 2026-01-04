<?php
/**
 * 消息服务类
 */

namespace InkClock\Service;

use InkClock\Utils\Logger;

class MessageService {
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
     * 发送消息到设备
     * @param int $userId 用户ID
     * @param array $messageInfo 消息信息
     * @return array 发送结果
     */
    public function sendMessage($userId, $messageInfo) {
        $this->logger->info('发送消息请求', ['user_id' => $userId, 'message' => $messageInfo]);
        
        // 验证消息信息
        if (!$this->validateMessageInfo($messageInfo)) {
            $this->logger->warning('消息验证失败', $messageInfo);
            return ['success' => false, 'error' => '无效的消息信息'];
        }
        
        // 验证设备所有权
        require_once __DIR__ . '/../Model/User.php';
        $userModel = new \InkClock\Model\User($this->db);
        if (!$userModel->isDeviceOwnedByUser($userId, $messageInfo['device_id'])) {
            $this->logger->warning('设备所有权验证失败', ['user_id' => $userId, 'device_id' => $messageInfo['device_id']]);
            return ['success' => false, 'error' => '无权向该设备发送消息'];
        }
        
        // 添加发送者信息
        $messageInfo['sender'] = 'user_' . $userId;
        
        // 调用模型发送消息
        require_once __DIR__ . '/../Model/Message.php';
        $messageModel = new \InkClock\Model\Message($this->db);
        $result = $messageModel->sendMessage($messageInfo);
        
        if ($result['success']) {
            $this->logger->info('消息发送成功', ['message_id' => $result['message_id'], 'device_id' => $messageInfo['device_id']]);
        } else {
            $this->logger->warning('消息发送失败', ['error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 获取消息列表
     * @param int $userId 用户ID
     * @param array $filters 过滤条件
     * @return array 消息列表
     */
    public function getMessageList($userId, $filters = []) {
        $this->logger->info('获取消息列表请求', ['user_id' => $userId, 'filters' => $filters]);
        
        // 普通用户只能查看自己设备的消息
        require_once __DIR__ . '/../Model/User.php';
        $userModel = new \InkClock\Model\User($this->db);
        $userDevices = $userModel->getUserDevices($userId);
        $deviceIds = array_column($userDevices, 'device_id');
        
        if (empty($deviceIds)) {
            return ['success' => true, 'messages' => []];
        }
        
        // 获取用户设备的消息
        require_once __DIR__ . '/../Model/Message.php';
        $messageModel = new \InkClock\Model\Message($this->db);
        $messages = $messageModel->getMessagesByDeviceIds($deviceIds);
        
        $this->logger->info('获取消息列表成功', ['count' => count($messages)]);
        
        return ['success' => true, 'messages' => $messages];
    }
    
    /**
     * 获取设备的消息
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return array 设备消息
     */
    public function getDeviceMessages($userId, $deviceId) {
        $this->logger->info('获取设备消息请求', ['user_id' => $userId, 'device_id' => $deviceId]);
        
        // 验证设备所有权
        require_once __DIR__ . '/../Model/User.php';
        $userModel = new \InkClock\Model\User($this->db);
        if (!$userModel->isDeviceOwnedByUser($userId, $deviceId)) {
            $this->logger->warning('设备所有权验证失败', ['user_id' => $userId, 'device_id' => $deviceId]);
            return ['success' => false, 'error' => '无权访问该设备的消息'];
        }
        
        // 获取设备消息
        require_once __DIR__ . '/../Model/Message.php';
        $messageModel = new \InkClock\Model\Message($this->db);
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
        
        require_once __DIR__ . '/../Model/Message.php';
        $messageModel = new \InkClock\Model\Message($this->db);
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
     * @param int $userId 用户ID
     * @param string $messageId 消息ID
     * @return array 操作结果
     */
    public function deleteMessage($userId, $messageId) {
        $this->logger->info('删除消息请求', ['user_id' => $userId, 'message_id' => $messageId]);
        
        // 获取消息详情，验证所有权
        require_once __DIR__ . '/../Model/Message.php';
        $messageModel = new \InkClock\Model\Message($this->db);
        $message = $messageModel->getMessageById($messageId);
        
        if (!$message) {
            return ['success' => false, 'error' => '消息不存在'];
        }
        
        // 验证设备所有权
        require_once __DIR__ . '/../Model/User.php';
        $userModel = new \InkClock\Model\User($this->db);
        if (!$userModel->isDeviceOwnedByUser($userId, $message['device_id'])) {
            $this->logger->warning('设备所有权验证失败', ['user_id' => $userId, 'device_id' => $message['device_id']]);
            return ['success' => false, 'error' => '无权删除该消息'];
        }
        
        $result = $messageModel->deleteMessage($messageId);
        
        if ($result['success']) {
            $this->logger->info('消息删除成功', ['message_id' => $messageId]);
        } else {
            $this->logger->warning('消息删除失败', ['message_id' => $messageId]);
        }
        
        return $result;
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
        require_once __DIR__ . '/../Model/User.php';
        $userModel = new \InkClock\Model\User($this->db);
        foreach ($batchInfo['device_ids'] as $deviceId) {
            if (!$userModel->isDeviceOwnedByUser($userId, $deviceId)) {
                return ['success' => false, 'error' => "无权向设备 {$deviceId} 发送消息"];
            }
        }
        
        // 批量发送消息
        require_once __DIR__ . '/../Model/Message.php';
        $messageModel = new \InkClock\Model\Message($this->db);
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