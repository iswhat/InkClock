<?php

namespace InkClock\Tests;

use PHPUnit\Framework\TestCase;
use InkClock\Controller\DeviceController;
use InkClock\Utils\Response;

class DeviceControllerTest extends TestCase
{
    private $controller;
    private $mockResponse;

    protected function setUp(): void
    {
        // 创建模拟Response对象
        $this->mockResponse = $this->createMock(Response::class);
        
        // 创建DeviceController实例
        $this->controller = new DeviceController();
        // 设置模拟的Response对象（需要根据实际代码调整）
        $reflection = new \ReflectionClass($this->controller);
        $responseProperty = $reflection->getProperty('response');
        $responseProperty->setAccessible(true);
        $responseProperty->setValue($this->controller, $this->mockResponse);
    }

    public function testRegisterDeviceWithValidParams()
    {
        // 模拟请求参数
        $params = [
            'device_id' => '123456789012',
            'model' => 'InkClock Pro',
            'firmware_version' => '1.0.0'
        ];
        
        // 配置模拟Response的期望
        $this->mockResponse->expects($this->once())
            ->method('success')
            ->with($this->stringContains('设备注册成功'));
        
        // 执行测试
        $this->controller->registerDevice($params);
    }

    public function testRegisterDeviceWithMissingParams()
    {
        // 模拟缺少参数的请求
        $params = [
            'device_id' => '123456789012'
            // 缺少model和firmware_version
        ];
        
        // 配置模拟Response的期望
        $this->mockResponse->expects($this->once())
            ->method('error')
            ->with($this->stringContains('缺少必要参数'), 400);
        
        // 执行测试
        $this->controller->registerDevice($params);
    }

    public function testUpdateDeviceStatusWithValidParams()
    {
        // 模拟请求参数
        $params = [
            'device_id' => '123456789012',
            'connection_status' => 'online',
            'battery_level' => 80
        ];
        
        // 配置模拟Response的期望
        $this->mockResponse->expects($this->once())
            ->method('success')
            ->with($this->stringContains('设备状态更新成功'));
        
        // 执行测试
        $this->controller->updateDeviceStatus($params);
    }
}
