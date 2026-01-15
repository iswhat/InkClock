<?php

namespace InkClock\Tests;

use InkClock\Utils\Cache;

class CacheTest {
    private $cache;
    
    public function __construct() {
        $this->cache = new Cache();
    }
    
    public function testCacheSetGet() {
        echo "Testing cache set and get...\n";
        
        // 设置缓存
        $this->cache->set('test_key', 'test_value', 60);
        
        // 获取缓存
        $value = $this->cache->get('test_key');
        return $value === 'test_value';
    }
    
    public function testCacheExpiration() {
        echo "Testing cache expiration...\n";
        
        // 设置一个1秒后过期的缓存
        $this->cache->set('expiring_key', 'expiring_value', 1);
        
        // 立即获取（应该存在）
        $value1 = $this->cache->get('expiring_key');
        
        // 等待2秒
        sleep(2);
        
        // 再次获取（应该不存在）
        $value2 = $this->cache->get('expiring_key');
        
        return $value1 === 'expiring_value' && $value2 === null;
    }
    
    public function testCacheDelete() {
        echo "Testing cache delete...\n";
        
        // 设置缓存
        $this->cache->set('delete_key', 'delete_value', 60);
        
        // 删除缓存
        $this->cache->delete('delete_key');
        
        // 尝试获取
        $value = $this->cache->get('delete_key');
        return $value === null;
    }
    
    public function testCacheClear() {
        echo "Testing cache clear...\n";
        
        // 设置多个缓存
        $this->cache->set('clear_key1', 'clear_value1', 60);
        $this->cache->set('clear_key2', 'clear_value2', 60);
        
        // 清空缓存
        $this->cache->clear();
        
        // 尝试获取
        $value1 = $this->cache->get('clear_key1');
        $value2 = $this->cache->get('clear_key2');
        
        return $value1 === null && $value2 === null;
    }
    
    public function testCacheStats() {
        echo "Testing cache stats...\n";
        
        // 获取缓存统计信息
        $stats = $this->cache->getStats();
        
        return is_array($stats) && isset($stats['hits']) && isset($stats['misses']);
    }
    
    public function run() {
        echo "Running Cache tests...\n";
        
        $tests = [
            'testCacheSetGet',
            'testCacheExpiration',
            'testCacheDelete',
            'testCacheClear',
            'testCacheStats'
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
$test = new CacheTest();
$test->run();
?>