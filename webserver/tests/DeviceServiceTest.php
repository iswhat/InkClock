<?php
/**
 * DeviceService测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../vendor/autoload.php';

use InkClock\Service\DeviceService;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

class DeviceServiceTest extends TestCase {
    private $deviceService;
    private $db;
    private $logger;
    
    public function __construct($testName) {
        parent::__construct($testName);
        
        // 初始化依赖
        $this->logger = new Logger();
        $this->logger->setLevel('error'); // 测试时只记录错误
        
        // 获取数据库连接
        $database = Database::getInstance();
        $this->db = $database->getConnection();
        
        // 创建DeviceService实例
        $this->deviceService = new DeviceService($this->db, $this->logger);
    }
    
    /**
     * 测试设备注册
     */
    public function testRegisterDevice() {
        // 测试有效设备注册
        $deviceInfo = [
            'device_id' => 'device_' . time(),
            'model' => 'test_model',
            'firmware_version' => '1.0.0'
        ];
        
        $result = $this->deviceService->registerDevice($deviceInfo);
        $this->assertTrue($result['success'], '有效设备应该注册成功');
        $this->assertEquals($deviceInfo['device_id'], $result['device_id'], '注册成功应该返回正确的设备ID');
        
        // 测试无效设备注册（缺少必填字段）
        $invalidDevice = [
            'device_id' => 'invalid_device'
            // 缺少model字段
        ];
        
        $result = $this->deviceService->registerDevice($invalidDevice);
        $this->assertFalse($result['success'], '缺少必填字段应该注册失败');
        
        // 测试短设备ID
        $shortDevice = [
            'device_id' => 'dev',
            'model' => 'test_model'
        ];
        
        $result = $this->deviceService->registerDevice($shortDevice);
        $this->assertFalse($result['success'], '短设备ID应该注册失败');
    }
    
    /**
     * 测试设备列表获取
     */
    public function testGetDeviceList() {
        // 先注册一些设备
        $deviceIds = [];
        for ($i = 0; $i < 3; $i++) {
            $deviceId = 'device_' . time() . '_' . $i;
            $this->deviceService->registerDevice([
                'device_id' => $deviceId,
                'model' => 'test_model',
                'firmware_version' => '1.0.0'
            ]);
            $deviceIds[] = $deviceId;
        }
        
        // 测试获取设备列表
        $result = $this->deviceService->getDeviceList(1); // 假设用户ID为1
        $this->assertTrue($result['success'], '获取设备列表应该成功');
        $this->assertTrue(is_array($result['devices']), '设备列表应该是数组');
        $this->assertTrue(count($result['devices']) >= 3, '设备列表应该包含至少3个设备');
    }
    
    /**
     * 测试设备详情获取
     */
    public function testGetDeviceDetail() {
        // 先注册一个设备
        $deviceId = 'device_' . time();
        $this->deviceService->registerDevice([
            'device_id' => $deviceId,
            'model' => 'test_model',
            'firmware_version' => '1.0.0'
        ]);
        
        // 测试获取设备详情
        $result = $this->deviceService->getDeviceDetail(1, $deviceId); // 假设用户ID为1
        $this->assertTrue($result['success'], '获取设备详情应该成功');
        $this->assertEquals($deviceId, $result['device']['device_id'], '设备详情应该包含正确的设备ID');
        $this->assertEquals('test_model', $result['device']['model'], '设备详情应该包含正确的设备型号');
        
        // 测试获取不存在的设备详情
        $result = $this->deviceService->getDeviceDetail(1, 'nonexistent_device');
        $this->assertFalse($result['success'], '获取不存在的设备详情应该失败');
    }
    
    /**
     * 测试设备在线状态获取
     */
    public function testGetDeviceStatus() {
        // 先注册一个设备
        $deviceId = 'device_' . time();
        $this->deviceService->registerDevice([
            'device_id' => $deviceId,
            'model' => 'test_model',
            'firmware_version' => '1.0.0'
        ]);
        
        // 测试获取设备在线状态
        $result = $this->deviceService->getDeviceStatus($deviceId);
        $this->assertTrue($result['success'], '获取设备在线状态应该成功');
        $this->assertEquals($deviceId, $result['status']['device_id'], '设备状态应该包含正确的设备ID');
        $this->assertTrue(in_array($result['status']['status'], ['online', 'offline']), '设备状态应该是online或offline');
        
        // 测试获取不存在的设备在线状态
        $result = $this->deviceService->getDeviceStatus('nonexistent_device');
        $this->assertFalse($result['success'], '获取不存在的设备在线状态应该失败');
    }
}
