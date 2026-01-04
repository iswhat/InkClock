<?php
/**
 * UserModel测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../src/Model/User.php';
require_once __DIR__ . '/../src/Utils/Logger.php';
require_once __DIR__ . '/../src/Utils/Database.php';

use InkClock\Model\User;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

class UserModelTest extends TestCase {
    private $userModel;
    private $db;
    private $logger;
    private $testUserId = 2;
    
    public function __construct($testName) {
        parent::__construct($testName);
        
        // 初始化依赖
        $this->logger = new Logger();
        $this->logger->setLevel('error'); // 测试时只记录错误
        
        // 获取数据库连接
        $database = Database::getInstance();
        $this->db = $database->getConnection();
        
        // 创建UserModel实例
        $this->userModel = new User($this->db);
        
        // 准备测试数据
        $this->prepareTestData();
    }
    
    /**
     * 准备测试数据
     */
    private function prepareTestData() {
        // 删除之前的测试用户
        $deleteSql = "DELETE FROM users WHERE id = {$this->testUserId}";
        $this->db->exec($deleteSql);
        
        // 确保测试设备存在
        $deviceSql = "INSERT OR IGNORE INTO devices (device_id, user_id, device_name, device_type, last_active, created_at) VALUES ('test-device-owner-123', 1, 'Test Device', 'inkclock', datetime('now'), datetime('now'))";
        $this->db->exec($deviceSql);
    }
    
    /**
     * 测试创建用户
     */
    public function testCreateUser() {
        $userInfo = [
            'username' => 'testuser2',
            'email' => 'testuser2@example.com',
            'password_hash' => 'password_hash_2',
            'api_key' => 'test_api_key_2'
        ];
        
        $userId = $this->userModel->createUser($userInfo);
        $this->assertGreaterThan(0, $userId, '创建用户应该返回有效的用户ID');
        
        // 测试获取用户信息
        $user = $this->userModel->getUserById($userId);
        $this->assertIsArray($user, '获取用户信息应该返回数组');
        $this->assertEquals($userInfo['username'], $user['username'], '用户名应该匹配');
    }
    
    /**
     * 测试验证API密钥
     */
    public function testValidateApiKey() {
        // 先创建一个测试用户
        $userInfo = [
            'username' => 'testuser_api',
            'email' => 'testuser_api@example.com',
            'password_hash' => 'password_hash',
            'api_key' => 'test_api_key_validate',
            'ip_whitelist' => '127.0.0.1,192.168.1.0/24'
        ];
        
        $userId = $this->userModel->createUser($userInfo);
        
        // 测试有效API密钥和允许的IP
        $result = $this->userModel->validateApiKey($userInfo['api_key'], '127.0.0.1');
        $this->assertIsArray($result, '有效API密钥应该返回用户信息');
        $this->assertEquals($userId, $result['id'], '用户ID应该匹配');
        
        // 测试有效API密钥但不允许的IP
        $result = $this->userModel->validateApiKey($userInfo['api_key'], '10.0.0.1');
        $this->assertFalse($result, 'IP不在白名单中应该返回false');
        
        // 测试无效API密钥
        $result = $this->userModel->validateApiKey('invalid_api_key', '127.0.0.1');
        $this->assertFalse($result, '无效API密钥应该返回false');
    }
    
    /**
     * 测试设备所有权验证
     */
    public function testIsDeviceOwnedByUser() {
        // 测试有效设备所有权
        $result = $this->userModel->isDeviceOwnedByUser(1, 'test-device-owner-123');
        $this->assertTrue($result, '设备应该属于用户1');
        
        // 测试无效设备所有权
        $result = $this->userModel->isDeviceOwnedByUser(2, 'test-device-owner-123');
        $this->assertFalse($result, '设备不应该属于用户2');
        
        // 测试无效设备ID
        $result = $this->userModel->isDeviceOwnedByUser(1, 'invalid-device-id');
        $this->assertFalse($result, '无效设备ID应该返回false');
    }
    
    /**
     * 测试获取用户设备
     */
    public function testGetUserDevices() {
        // 测试获取用户设备
        $devices = $this->userModel->getUserDevices(1);
        $this->assertIsArray($devices, '获取用户设备应该返回数组');
        
        // 测试获取无效用户设备
        $devices = $this->userModel->getUserDevices(999);
        $this->assertIsArray($devices, '获取无效用户设备应该返回数组');
        $this->assertEmpty($devices, '无效用户设备列表应该为空');
    }
    
    /**
     * 测试更新API密钥
     */
    public function testUpdateApiKey() {
        // 先创建一个测试用户
        $userInfo = [
            'username' => 'testuser_update',
            'email' => 'testuser_update@example.com',
            'password_hash' => 'password_hash',
            'api_key' => 'old_api_key'
        ];
        
        $userId = $this->userModel->createUser($userInfo);
        
        // 更新API密钥
        $newApiKey = 'new_api_key_' . time();
        $result = $this->userModel->updateApiKey($userId, $newApiKey);
        $this->assertTrue($result, '更新API密钥应该成功');
        
        // 验证API密钥已更新
        $user = $this->userModel->getUserById($userId);
        $this->assertEquals($newApiKey, $user['api_key'], 'API密钥应该已更新');
    }
    
    /**
     * 测试设置API密钥过期时间
     */
    public function testSetApiKeyExpiry() {
        // 先创建一个测试用户
        $userInfo = [
            'username' => 'testuser_expiry',
            'email' => 'testuser_expiry@example.com',
            'password_hash' => 'password_hash',
            'api_key' => 'test_api_key_expiry'
        ];
        
        $userId = $this->userModel->createUser($userInfo);
        
        // 设置API密钥过期时间（24小时后）
        $expiryTime = date('Y-m-d H:i:s', strtotime('+24 hours'));
        $result = $this->userModel->setApiKeyExpiry($userId, $expiryTime);
        $this->assertTrue($result, '设置API密钥过期时间应该成功');
        
        // 验证过期时间已设置
        $user = $this->userModel->getUserById($userId);
        $this->assertEquals($expiryTime, $user['api_key_expiry'], 'API密钥过期时间应该已设置');
    }
    
    /**
     * 测试设置IP白名单
     */
    public function testSetIpWhitelist() {
        // 先创建一个测试用户
        $userInfo = [
            'username' => 'testuser_whitelist',
            'email' => 'testuser_whitelist@example.com',
            'password_hash' => 'password_hash',
            'api_key' => 'test_api_key_whitelist'
        ];
        
        $userId = $this->userModel->createUser($userInfo);
        
        // 设置IP白名单
        $ipWhitelist = '127.0.0.1,192.168.1.0/24,10.0.0.1-10.0.0.10';
        $result = $this->userModel->setIpWhitelist($userId, $ipWhitelist);
        $this->assertTrue($result, '设置IP白名单应该成功');
        
        // 验证IP白名单已设置
        $user = $this->userModel->getUserById($userId);
        $this->assertEquals($ipWhitelist, $user['ip_whitelist'], 'IP白名单应该已设置');
    }
    
    /**
     * 测试检查IP是否在白名单中
     */
    public function testIsIpAllowed() {
        // 测试IP在白名单中
        $allowed = $this->userModel->isIpAllowed('127.0.0.1', '127.0.0.1,192.168.1.0/24');
        $this->assertTrue($allowed, '127.0.0.1应该在白名单中');
        
        // 测试IP不在白名单中
        $allowed = $this->userModel->isIpAllowed('10.0.0.1', '127.0.0.1,192.168.1.0/24');
        $this->assertFalse($allowed, '10.0.0.1不应该在白名单中');
        
        // 测试空白名单（允许所有IP）
        $allowed = $this->userModel->isIpAllowed('10.0.0.1', '');
        $this->assertTrue($allowed, '空白名单应该允许所有IP');
        
        // 测试CIDR范围
        $allowed = $this->userModel->isIpAllowed('192.168.1.50', '192.168.1.0/24');
        $this->assertTrue($allowed, '192.168.1.50应该在192.168.1.0/24范围内');
        
        // 测试IP范围
        $allowed = $this->userModel->isIpAllowed('10.0.0.5', '10.0.0.1-10.0.0.10');
        $this->assertTrue($allowed, '10.0.0.5应该在10.0.0.1-10.0.0.10范围内');
    }
    
    /**
     * 测试获取用户信息
     */
    public function testGetUserByApiKey() {
        // 先创建一个测试用户
        $userInfo = [
            'username' => 'testuser_api_get',
            'email' => 'testuser_api_get@example.com',
            'password_hash' => 'password_hash',
            'api_key' => 'test_api_key_get'
        ];
        
        $userId = $this->userModel->createUser($userInfo);
        
        // 通过API密钥获取用户信息
        $user = $this->userModel->getUserByApiKey($userInfo['api_key']);
        $this->assertIsArray($user, '通过API密钥获取用户信息应该返回数组');
        $this->assertEquals($userId, $user['id'], '用户ID应该匹配');
        
        // 测试无效API密钥
        $user = $this->userModel->getUserByApiKey('invalid_api_key');
        $this->assertFalse($user, '无效API密钥应该返回false');
    }
    
    /**
     * 清理测试数据
     */
    public function tearDown() {
        // 删除测试用户
        $deleteSql = "DELETE FROM users WHERE id = {$this->testUserId} OR username LIKE 'testuser_%'";
        $this->db->exec($deleteSql);
    }
}
