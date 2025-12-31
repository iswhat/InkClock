<?php
/**
 * DeviceService测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../services/DeviceService.php';

class DeviceServiceTest extends TestCase {
    private $deviceService;
    
    public function __construct($testName) {
        parent::__construct($testName);
        $this->deviceService = new DeviceService();
    }
    
    /**
     * 测试设备注册信息验证
     */
    public function testRegisterDeviceValidation() {
        // 测试缺少必填字段的情况
        $invalidDevice = ['device_id' => 'test123'];
        $result = $this->deviceService->registerDevice($invalidDevice);
        $this->assertTrue($result['success'] === false, '缺少设备型号应该注册失败');
        
        // 测试短设备ID
        $invalidDevice = [
            'device_id' => 'test',
            'model' => 'test_model'
        ];
        $result = $this->deviceService->registerDevice($invalidDevice);
        $this->assertTrue($result['success'] === false, '短设备ID应该注册失败');
    }
    
    /**
     * 测试设备列表获取
     */
    public function testGetDeviceList() {
        // 测试获取设备列表（模拟数据）
        $result = $this->deviceService->getDeviceList(1);
        $this->assertTrue($result['success'] === true, '获取设备列表应该成功');
        $this->assertTrue(is_array($result['devices']), '设备列表应该是数组');
    }
    
    /**
     * 测试设备在线状态检查
     */
    public function testGetDeviceStatus() {
        // 测试无效设备ID
        $result = $this->deviceService->getDeviceStatus('invalid_device_id');
        $this->assertTrue($result['success'] === false, '无效设备ID应该返回错误');
    }
}
?>