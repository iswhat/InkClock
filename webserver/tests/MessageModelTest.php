<?php
/**
 * MessageModel测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../src/Model/Message.php';
require_once __DIR__ . '/../src/Utils/Logger.php';
require_once __DIR__ . '/../src/Utils/Database.php';

use InkClock\Model\Message;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

class MessageModelTest extends TestCase {
    private $messageModel;
    private $db;
    private $logger;
    private $testDeviceId = 'test-device-msg-123';
    private $testUserId = 1;
    
    public function __construct($testName) {
        parent::__construct($testName);
        
        // 初始化依赖
        $this->logger = new Logger();
        $this->logger->setLevel('error'); // 测试时只记录错误
        
        // 获取数据库连接
        $database = Database::getInstance();
        $this->db = $database->getConnection();
        
        // 创建MessageModel实例
        $this->messageModel = new Message($this->db);
        
        // 准备测试数据
        $this->prepareTestData();
    }
    
    /**
     * 准备测试数据
     */
    private function prepareTestData() {
        // 确保测试用户存在
        $userSql = "INSERT OR IGNORE INTO users (id, username, email, password_hash, api_key, created_at, updated_at) VALUES (1, 'testuser', 'test@example.com', 'password_hash', 'test_api_key', datetime('now'), datetime('now'))";
        $this->db->exec($userSql);
        
        // 确保测试设备存在
        $deviceSql = "INSERT OR IGNORE INTO devices (device_id, user_id, device_name, device_type, last_active, created_at) VALUES ('{$this->testDeviceId}', 1, 'Test Device', 'inkclock', datetime('now'), datetime('now'))";
        $this->db->exec($deviceSql);
    }
    
    /**
     * 测试发送消息
     */
    public function testSendMessage() {
        $messageInfo = [
            'device_id' => $this->testDeviceId,
            'content' => 'Test message content',
            'type' => 'text',
            'sender' => 'user_1'
        ];
        
        $result = $this->messageModel->sendMessage($messageInfo);
        $this->assertTrue($result['success'], '发送消息应该成功');
        $this->assertNotNull($result['message_id'], '发送成功应该返回消息ID');
    }
    
    /**
     * 测试获取设备消息
     */
    public function testGetMessagesByDeviceId() {
        // 先发送一条测试消息
        $this->testSendMessage();
        
        // 获取设备消息
        $messages = $this->messageModel->getMessagesByDeviceId($this->testDeviceId);
        $this->assertIsArray($messages, '获取设备消息应该返回数组');
        $this->assertGreaterThanOrEqual(1, count($messages), '设备消息列表应该包含至少1条消息');
        
        // 测试无效设备ID
        $messages = $this->messageModel->getMessagesByDeviceId('invalid-device-id');
        $this->assertIsArray($messages, '获取无效设备消息应该返回数组');
        $this->assertEmpty($messages, '无效设备消息列表应该为空');
    }
    
    /**
     * 测试获取多条设备消息
     */
    public function testGetMessagesByDeviceIds() {
        // 先发送测试消息
        $this->testSendMessage();
        
        // 获取多条设备消息
        $deviceIds = [$this->testDeviceId, 'invalid-device-id'];
        $messages = $this->messageModel->getMessagesByDeviceIds($deviceIds);
        $this->assertIsArray($messages, '获取多条设备消息应该返回数组');
        $this->assertGreaterThanOrEqual(1, count($messages), '多条设备消息列表应该包含至少1条消息');
    }
    
    /**
     * 测试根据ID获取消息
     */
    public function testGetMessageById() {
        // 先发送一条测试消息
        $sendResult = $this->messageModel->sendMessage([
            'device_id' => $this->testDeviceId,
            'content' => 'Test message for ID retrieval',
            'type' => 'text',
            'sender' => 'user_1'
        ]);
        
        $messageId = $sendResult['message_id'];
        
        // 根据ID获取消息
        $message = $this->messageModel->getMessageById($messageId);
        $this->assertIsArray($message, '根据ID获取消息应该返回数组');
        $this->assertEquals($messageId, $message['id'], '消息ID应该匹配');
        
        // 测试无效消息ID
        $message = $this->messageModel->getMessageById('invalid-message-id');
        $this->assertFalse($message, '无效消息ID应该返回false');
    }
    
    /**
     * 测试标记消息为已读
     */
    public function testMarkAsRead() {
        // 先发送一条测试消息
        $sendResult = $this->messageModel->sendMessage([
            'device_id' => $this->testDeviceId,
            'content' => 'Test message to mark as read',
            'type' => 'text',
            'sender' => 'user_1'
        ]);
        
        $messageId = $sendResult['message_id'];
        
        // 标记消息为已读
        $result = $this->messageModel->markAsRead($messageId);
        $this->assertTrue($result['success'], '标记消息为已读应该成功');
        
        // 验证消息已读状态
        $message = $this->messageModel->getMessageById($messageId);
        $this->assertEquals(1, $message['is_read'], '消息状态应该已更新为已读');
        
        // 测试无效消息ID
        $result = $this->messageModel->markAsRead('invalid-message-id');
        $this->assertFalse($result['success'], '标记无效消息为已读应该失败');
    }
    
    /**
     * 测试删除消息
     */
    public function testDeleteMessage() {
        // 先发送一条测试消息
        $sendResult = $this->messageModel->sendMessage([
            'device_id' => $this->testDeviceId,
            'content' => 'Test message to delete',
            'type' => 'text',
            'sender' => 'user_1'
        ]);
        
        $messageId = $sendResult['message_id'];
        
        // 删除消息
        $result = $this->messageModel->deleteMessage($messageId);
        $this->assertTrue($result['success'], '删除消息应该成功');
        
        // 验证消息已删除
        $message = $this->messageModel->getMessageById($messageId);
        $this->assertFalse($message, '删除后的消息应该不存在');
        
        // 测试无效消息ID
        $result = $this->messageModel->deleteMessage('invalid-message-id');
        $this->assertFalse($result['success'], '删除无效消息应该失败');
    }
    
    /**
     * 测试获取未读消息计数
     */
    public function testGetUnreadCount() {
        // 先发送多条测试消息
        for ($i = 0; $i < 3; $i++) {
            $this->messageModel->sendMessage([
                'device_id' => $this->testDeviceId,
                'content' => "Test unread message {$i}",
                'type' => 'text',
                'sender' => 'user_1'
            ]);
        }
        
        // 获取未读消息计数
        $count = $this->messageModel->getUnreadCount($this->testDeviceId);
        $this->assertIsInt($count, '未读消息计数应该是整数');
        $this->assertGreaterThanOrEqual(3, $count, '未读消息计数应该至少为3');
    }
    
    /**
     * 测试获取消息统计信息
     */
    public function testGetMessageStats() {
        // 先发送测试消息
        $this->testSendMessage();
        
        // 获取消息统计信息
        $stats = $this->messageModel->getMessageStats($this->testDeviceId);
        $this->assertIsArray($stats, '获取消息统计信息应该返回数组');
        $this->assertArrayHasKey('total', $stats, '消息统计信息应该包含total字段');
        $this->assertArrayHasKey('unread', $stats, '消息统计信息应该包含unread字段');
        $this->assertArrayHasKey('today', $stats, '消息统计信息应该包含today字段');
    }
    
    /**
     * 清理测试数据
     */
    public function tearDown() {
        // 删除测试消息
        $deleteSql = "DELETE FROM messages WHERE device_id = '{$this->testDeviceId}'";
        $this->db->exec($deleteSql);
    }
}
