<?php

namespace InkClock\Tests;

use InkClock\Utils\DIContainer;

class DIContainerTest {
    private $container;
    
    public function __construct() {
        $this->container = new DIContainer();
    }
    
    public function testContainerCreation() {
        echo "Testing container creation...\n";
        return $this->container !== null;
    }
    
    public function testServiceRegistration() {
        echo "Testing service registration...\n";
        
        // 注册一个服务
        $this->container->set('test_service', function() {
            return ['key' => 'value'];
        });
        
        // 获取服务
        $service = $this->container->get('test_service');
        return $service !== null && $service['key'] === 'value';
    }
    
    public function testServiceSharing() {
        echo "Testing service sharing...\n";
        
        // 注册一个共享服务
        $this->container->setShared('shared_service', function() {
            return ['shared' => true];
        });
        
        // 获取两次服务
        $service1 = $this->container->get('shared_service');
        $service2 = $this->container->get('shared_service');
        
        return $service1 !== null && $service2 !== null && $service1 === $service2;
    }
    
    public function testFactoryService() {
        echo "Testing factory service...\n";
        
        // 注册一个工厂服务
        $this->container->set('factory_service', function() {
            return ['timestamp' => time()];
        });
        
        // 获取两次服务
        $service1 = $this->container->get('factory_service');
        sleep(1); // 等待1秒
        $service2 = $this->container->get('factory_service');
        
        return $service1 !== null && $service2 !== null && $service1 !== $service2;
    }
    
    public function run() {
        echo "Running DIContainer tests...\n";
        
        $tests = [
            'testContainerCreation',
            'testServiceRegistration',
            'testServiceSharing',
            'testFactoryService'
        ];
        
        $passed = 0;
        $total = count($tests);
        
        foreach ($tests as $test) {
            if ($this->$test()) {
                echo "✓ $test passed\n";
                $passed++;
            } else {
                echo "✗ $test failed\n";
            }
        }
        
        echo "\nTests completed: $passed/$total passed\n";
        return $passed === $total;
    }
}

// 运行测试
$test = new DIContainerTest();
$test->run();
?>