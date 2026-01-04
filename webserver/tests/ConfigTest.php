<?php
/**
 * Config类测试
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../vendor/autoload.php';

use InkClock\Config\Config;

class ConfigTest extends TestCase {
    private $originalEnv;
    
    public function __construct($testName) {
        parent::__construct($testName);
        
        // 保存原始环境变量
        $this->originalEnv = $_ENV;
        
        // 清除现有配置缓存
        $reflection = new ReflectionClass(Config::class);
        $loadedProperty = $reflection->getProperty('loaded');
        $loadedProperty->setAccessible(true);
        $loadedProperty->setValue(null, false);
        
        $configProperty = $reflection->getProperty('config');
        $configProperty->setAccessible(true);
        $configProperty->setValue(null, []);
    }
    
    public function __destruct() {
        // 恢复原始环境变量
        $_ENV = $this->originalEnv;
    }
    
    /**
     * 测试配置加载
     */
    public function testConfigLoad() {
        $config = Config::get('db.host');
        $this->assertNotNull($config, '数据库主机配置应该存在');
    }
    
    /**
     * 测试环境变量覆盖配置
     */
    public function testEnvOverride() {
        // 设置环境变量
        putenv('INKCLOCK_DB_HOST=env_host');
        $_ENV['INKCLOCK_DB_HOST'] = 'env_host';
        
        // 清除配置缓存
        $reflection = new ReflectionClass(Config::class);
        $loadedProperty = $reflection->getProperty('loaded');
        $loadedProperty->setAccessible(true);
        $loadedProperty->setValue(null, false);
        
        $configProperty = $reflection->getProperty('config');
        $configProperty->setAccessible(true);
        $configProperty->setValue(null, []);
        
        // 获取配置，应该从环境变量读取
        $config = Config::get('db.host');
        $this->assertEquals('env_host', $config, '数据库主机配置应该被环境变量覆盖');
    }
    
    /**
     * 测试配置默认值
     */
    public function testConfigDefaultValue() {
        $config = Config::get('non_existent_key', 'default_value');
        $this->assertEquals('default_value', $config, '不存在的配置键应该返回默认值');
    }
    
    /**
     * 测试配置存在性检查
     */
    public function testConfigHas() {
        $hasConfig = Config::has('db.host');
        $this->assertTrue($hasConfig, '数据库主机配置应该存在');
        
        $hasConfig = Config::has('non_existent_key');
        $this->assertFalse($hasConfig, '不存在的配置键应该返回false');
    }
    
    /**
     * 测试配置设置
     */
    public function testConfigSet() {
        Config::set('test_key', 'test_value');
        $config = Config::get('test_key');
        $this->assertEquals('test_value', $config, '设置的配置值应该能正确获取');
    }
    
    /**
     * 测试嵌套配置设置
     */
    public function testNestedConfigSet() {
        Config::set('test.nested.key', 'nested_value');
        $config = Config::get('test.nested.key');
        $this->assertEquals('nested_value', $config, '嵌套配置值应该能正确获取');
    }
}
