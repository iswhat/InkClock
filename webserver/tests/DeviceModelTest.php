<?php
/**
 * DeviceModel测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../src/Model/Device.php';
require_once __DIR__ . '/../src/Utils/Logger.php';
require_once __DIR__ . '/../src/Utils/Database.php';

use InkClock\Model\Device;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

class DeviceModelTest extends TestCase {
    private $deviceModel;
    private $db;
    private $logger;
    private $testDeviceId = 'test-device-789';
    private $testUserId = 1;
    
    public function __construct($testName) {
        parent::__construct($testName);
        
        // 初始化依赖
        $this->logger = new Logger();
        $this->logger->setLevel('error'); // 测试时只记录错误
        
        // 获取数据库连接
        $database = Database::getInstance();
        $this->db = $database->getConnection();
        
        // 创建DeviceModel实例
        $this->deviceModel = new Device($this->db);
        
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
        
        // 删除之前的测试设备
        $deleteSql = "DELETE FROM devices WHERE device_id = '{$this->testDeviceId}'";
        $this->db->exec($deleteSql);
    }
    
    /**
     * 测试添加设备
     */
    public function testAddDevice() {
        $deviceInfo = [
            'device_id' => $this->testDeviceId,
            'user_id' => $this->testUserId,
            'device_name' => 'Test Device',
            'device_type' => 'inkclock',
            'device_info' => json_encode(['model' => 'v1', 'firmware' => '1.0.0'])
        ];
        
        $result = $this->deviceModel->addDevice($deviceInfo);
        $this->assertTrue($result, '添加设备应该成功');
        
        // 测试重复设备ID
        $result = $this->deviceModel->addDevice($deviceInfo);
        $this->assertFalse($result, '重复设备ID应该添加失败');
    }
    
    /**
     * 测试获取设备信息
     */
    public function testGetDeviceById() {
        // 先添加测试设备
        $this->testAddDevice();
        
        // 获取设备信息
        $device = $this->deviceModel->getDeviceById($this->testDeviceId);
        $this->assertIsArray($device, '获取设备信息应该返回数组');
        $this->assertEquals($this->testDeviceId, $device['device_id'], '设备ID应该匹配');
        $this->assertEquals($this->testUserId, $device['user_id'], '用户ID应该匹配');
        
        // 测试无效设备ID
        $device = $this->deviceModel->getDeviceById('invalid-device-id');
        $this->assertNull($device, '无效设备ID应该返回null');
    }
    
    /**
     * 测试获取用户设备列表
     */
    public function testGetUserDevices() {
        // 先添加测试设备
        $this->testAddDevice();
        
        // 获取用户设备列表
        $devices = $this->deviceModel->getUserDevices($this->testUserId);
        $this->assertIsArray($devices, '获取用户设备列表应该返回数组');
        $this->assertGreaterThanOrEqual(1, count($devices), '用户设备列表应该包含至少1条设备');
        
        // 测试无效用户ID
        $devices = $this->deviceModel->getUserDevices(999);
        $this->assertIsArray($devices, '获取无效用户设备列表应该返回数组');
        $this->assertEmpty($devices, '无效用户设备列表应该为空');
    }
    
    /**
     * 测试更新设备信息
     */
    public function testUpdateDevice() {
        // 先添加测试设备
        $this->testAddDevice();
        
        // 更新设备信息
        $updateInfo = [
            'device_name' => 'Updated Test Device',
            'device_info' => json_encode(['model' => 'v1', 'firmware' => '1.0.1'])
        ];
        
        $result = $this->deviceModel->updateDevice($this->testDeviceId, $updateInfo);
        $this->assertTrue($result, '更新设备信息应该成功');
        
        // 验证更新结果
        $device = $this->deviceModel->getDeviceById($this->testDeviceId);
        $this->assertEquals('Updated Test Device', $device['device_name'], '设备名称应该已更新');
        
        // 测试无效设备ID
        $result = $this->deviceModel->updateDevice('invalid-device-id', $updateInfo);
        $this->assertFalse($result, '更新无效设备信息应该失败');
    }
    
    /**
     * 测试删除设备
     */
    public function testDeleteDevice() {
        // 先添加测试设备
        $this->testAddDevice();
        
        // 删除设备
        $result = $this->deviceModel->deleteDevice($this->testDeviceId);
        $this->assertTrue($result, '删除设备应该成功');
        
        // 验证删除结果
        $device = $this->deviceModel->getDeviceById($this->testDeviceId);
        $this->assertNull($device, '删除后的设备应该不存在');
        
        // 测试无效设备ID
        $result = $this->deviceModel->deleteDevice('invalid-device-id');
        $this->assertFalse($result, '删除无效设备应该失败');
    }
    
    /**
     * 测试更新设备状态
     */
    public function testUpdateDeviceStatus() {
        // 先添加测试设备
        $this->testAddDevice();
        
        // 更新设备状态
        $statusInfo = [
            'battery_level' => 85,
            'connection_status' => 'online',
            'last_active' => date('Y-m-d H:i:s')
        ];
        
        $result = $this->deviceModel->updateDeviceStatus($this->testDeviceId, $statusInfo);
        $this->assertTrue($result, '更新设备状态应该成功');
        
        // 验证更新结果
        $device = $this->deviceModel->getDeviceById($this->testDeviceId);
        $this->assertEquals(85, $device['battery_level'], '电池电量应该已更新');
        $this->assertEquals('online', $device['connection_status'], '连接状态应该已更新');
    }
    
    /**
     * 测试获取设备统计信息
     */
    public function testGetDeviceStats() {
        // 先添加测试设备
        $this->testAddDevice();
        
        // 获取设备统计信息
        $stats = $this->deviceModel->getDeviceStats($this->testUserId);
        $this->assertIsArray($stats, '获取设备统计信息应该返回数组');
        $this->assertArrayHasKey('total', $stats, '设备统计信息应该包含total字段');
        $this->assertArrayHasKey('online', $stats, '设备统计信息应该包含online字段');
        $this->assertArrayHasKey('offline', $stats, '设备统计信息应该包含offline字段');
    }
    
    /**
     * 清理测试数据
     */
    public function tearDown() {
        // 删除测试设备
        $deleteSql = "DELETE FROM devices WHERE device_id = '{$this->testDeviceId}'";
        $this->db->exec($deleteSql);
    }
}
