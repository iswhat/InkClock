<?php
/**
 * MessageService测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../src/Service/MessageService.php';
require_once __DIR__ . '/../src/Model/Device.php';
require_once __DIR__ . '/../src/Model/User.php';
require_once __DIR__ . '/../src/Utils/Logger.php';
require_once __DIR__ . '/../src/Utils/Database.php';

use InkClock\Service\MessageService;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

class MessageServiceTest extends TestCase {
    private $messageService;
    private $db;
    private $logger;
    private $testUserId = 1;
    private $testDeviceId = 'test-device-123';
    
    public function __construct($testName) {
        parent::__construct($testName);
        
        // 初始化依赖
        $this->logger = new Logger();
        $this->logger->setLevel('error'); // 测试时只记录错误
        
        // 获取数据库连接
        $database = Database::getInstance();
        $this->db = $database->getConnection();
        
        // 创建MessageService实例
        $this->messageService = new MessageService($this->db, $this->logger);
        
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
        $deviceSql = "INSERT OR IGNORE INTO devices (device_id, user_id, device_name, device_type, last_active, created_at) VALUES ('test-device-123', 1, 'Test Device', 'inkclock', datetime('now'), datetime('now'))";
        $this->db->exec($deviceSql);
    }
    
    /**
     * 测试发送消息
     */
    public function testSendMessage() {
        // 测试有效消息发送
        $messageInfo = [
            'device_id' => $this->testDeviceId,
            'content' => 'Test message content',
            'type' => 'text'
        ];
        
        $result = $this->messageService->sendMessage($this->testUserId, $messageInfo);
        $this->assertTrue($result['success'], '有效消息应该发送成功');
        $this->assertNotNull($result['message_id'], '发送成功应该返回消息ID');
        
        // 测试无效消息发送（缺少设备ID）
        $invalidMessageInfo = [
            'content' => 'Test message content',
            'type' => 'text'
        ];
        
        $result = $this->messageService->sendMessage($this->testUserId, $invalidMessageInfo);
        $this->assertFalse($result['success'], '缺少设备ID的消息应该发送失败');
        
        // 测试无效消息发送（缺少内容）
        $invalidMessageInfo = [
            'device_id' => $this->testDeviceId,
            'type' => 'text'
        ];
        
        $result = $this->messageService->sendMessage($this->testUserId, $invalidMessageInfo);
        $this->assertFalse($result['success'], '缺少内容的消息应该发送失败');
    }
    
    /**
     * 测试获取消息列表
     */
    public function testGetMessageList() {
        // 先发送一些测试消息
        for ($i = 0; $i < 3; $i++) {
            $messageInfo = [
                'device_id' => $this->testDeviceId,
                'content' => "Test message {$i}",
                'type' => 'text'
            ];
            $this->messageService->sendMessage($this->testUserId, $messageInfo);
        }
        
        // 获取消息列表
        $result = $this->messageService->getMessageList($this->testUserId);
        $this->assertTrue($result['success'], '获取消息列表应该成功');
        $this->assertIsArray($result['messages'], '消息列表应该是数组');
        $this->assertGreaterThanOrEqual(3, count($result['messages']), '消息列表应该包含至少3条消息');
    }
    
    /**
     * 测试获取设备消息
     */
    public function testGetDeviceMessages() {
        // 发送一条设备消息
        $messageInfo = [
            'device_id' => $this->testDeviceId,
            'content' => 'Device-specific message',
            'type' => 'text'
        ];
        $this->messageService->sendMessage($this->testUserId, $messageInfo);
        
        // 获取设备消息
        $result = $this->messageService->getDeviceMessages($this->testUserId, $this->testDeviceId);
        $this->assertTrue($result['success'], '获取设备消息应该成功');
        $this->assertIsArray($result['messages'], '设备消息列表应该是数组');
        $this->assertGreaterThanOrEqual(1, count($result['messages']), '设备消息列表应该包含至少1条消息');
        
        // 测试无效设备ID
        $result = $this->messageService->getDeviceMessages($this->testUserId, 'invalid-device-id');
        $this->assertTrue($result['success'], '无效设备ID应该返回空列表');
        $this->assertEmpty($result['messages'], '无效设备ID应该返回空消息列表');
    }
    
    /**
     * 测试标记消息为已读
     */
    public function testMarkMessageAsRead() {
        // 先发送一条消息
        $messageInfo = [
            'device_id' => $this->testDeviceId,
            'content' => 'Message to mark as read',
            'type' => 'text'
        ];
        $sendResult = $this->messageService->sendMessage($this->testUserId, $messageInfo);
        $this->assertTrue($sendResult['success'], '发送测试消息应该成功');
        
        // 标记消息为已读
        $result = $this->messageService->markMessageAsRead($sendResult['message_id']);
        $this->assertTrue($result['success'], '标记消息为已读应该成功');
        
        // 测试无效消息ID
        $result = $this->messageService->markMessageAsRead('invalid-message-id');
        $this->assertFalse($result['success'], '标记无效消息为已读应该失败');
    }
    
    /**
     * 测试删除消息
     */
    public function testDeleteMessage() {
        // 先发送一条消息
        $messageInfo = [
            'device_id' => $this->testDeviceId,
            'content' => 'Message to delete',
            'type' => 'text'
        ];
        $sendResult = $this->messageService->sendMessage($this->testUserId, $messageInfo);
        $this->assertTrue($sendResult['success'], '发送测试消息应该成功');
        
        // 删除消息
        $result = $this->messageService->deleteMessage($this->testUserId, $sendResult['message_id']);
        $this->assertTrue($result['success'], '删除消息应该成功');
        
        // 测试无效消息ID
        $result = $this->messageService->deleteMessage($this->testUserId, 'invalid-message-id');
        $this->assertFalse($result['success'], '删除无效消息应该失败');
    }
    
    /**
     * 测试批量发送消息
     */
    public function testSendBatchMessages() {
        // 准备测试设备
        $deviceIds = [$this->testDeviceId, 'test-device-456', 'test-device-789'];
        foreach ($deviceIds as $deviceId) {
            $deviceSql = "INSERT OR IGNORE INTO devices (device_id, user_id, device_name, device_type, last_active, created_at) VALUES ('{$deviceId}', 1, 'Test Device', 'inkclock', datetime('now'), datetime('now'))";
            $this->db->exec($deviceSql);
        }
        
        // 测试批量发送消息
        $batchInfo = [
            'device_ids' => $deviceIds,
            'content' => 'Batch message content',
            'type' => 'text'
        ];
        
        $result = $this->messageService->sendBatchMessages($this->testUserId, $batchInfo);
        $this->assertTrue($result['success'], '批量发送消息应该成功');
        $this->assertIsArray($result['results'], '批量发送结果应该是数组');
        $this->assertEquals(count($deviceIds), count($result['results']), '批量发送结果数量应该与设备数量一致');
        
        // 测试无效批量消息（缺少设备ID）
        $invalidBatchInfo = [
            'content' => 'Batch message content',
            'type' => 'text'
        ];
        
        $result = $this->messageService->sendBatchMessages($this->testUserId, $invalidBatchInfo);
        $this->assertFalse($result['success'], '缺少设备ID的批量消息应该失败');
        
        // 测试无效批量消息（缺少内容）
        $invalidBatchInfo = [
            'device_ids' => $deviceIds,
            'type' => 'text'
        ];
        
        $result = $this->messageService->sendBatchMessages($this->testUserId, $invalidBatchInfo);
        $this->assertFalse($result['success'], '缺少内容的批量消息应该失败');
    }
    
    /**
     * 清理测试数据
     */
    public function tearDown() {
        // 删除测试消息
        $this->db->exec("DELETE FROM messages WHERE sender LIKE 'user_%'");
        
        // 删除测试设备
        $this->db->exec("DELETE FROM devices WHERE device_id LIKE 'test-device-%'");
    }
}
