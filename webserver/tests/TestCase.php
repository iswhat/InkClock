<?php
/**
 * 测试基类
 */

class TestCase {
    protected $passed = 0;
    protected $failed = 0;
    protected $total = 0;
    protected $testName;
    
    public function __construct($testName) {
        $this->testName = $testName;
        echo "\n=== 开始测试: {$this->testName} ===\n";
    }
    
    /**
     * 断言相等
     */
    protected function assertEquals($expected, $actual, $message = '') {
        $this->total++;
        if ($expected === $actual) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望: " . print_r($expected, true) . "\n";
            echo "  实际: " . print_r($actual, true) . "\n";
            return false;
        }
    }
    
    /**
     * 断言为真
     */
    protected function assertTrue($condition, $message = '') {
        return $this->assertEquals(true, $condition, $message);
    }
    
    /**
     * 断言为假
     */
    protected function assertFalse($condition, $message = '') {
        return $this->assertEquals(false, $condition, $message);
    }
    
    /**
     * 断言不为空
     */
    protected function assertNotNull($value, $message = '') {
        $this->total++;
        if ($value !== null) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  值为null\n";
            return false;
        }
    }
    
    /**
     * 断言为空
     */
    protected function assertNull($value, $message = '') {
        $this->total++;
        if ($value === null) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望null，实际: " . print_r($value, true) . "\n";
            return false;
        }
    }
    
    /**
     * 断言是数组
     */
    protected function assertIsArray($value, $message = '') {
        $this->total++;
        if (is_array($value)) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望数组，实际类型: " . gettype($value) . "\n";
            return false;
        }
    }
    
    /**
     * 断言是空数组
     */
    protected function assertEmpty($value, $message = '') {
        $this->total++;
        if (empty($value)) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望为空，实际: " . print_r($value, true) . "\n";
            return false;
        }
    }
    
    /**
     * 断言是整数
     */
    protected function assertIsInt($value, $message = '') {
        $this->total++;
        if (is_int($value)) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望整数，实际类型: " . gettype($value) . "\n";
            return false;
        }
    }
    
    /**
     * 断言大于
     */
    protected function assertGreaterThan($expected, $actual, $message = '') {
        $this->total++;
        if ($actual > $expected) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望大于 {$expected}，实际: {$actual}\n";
            return false;
        }
    }
    
    /**
     * 断言大于等于
     */
    protected function assertGreaterThanOrEqual($expected, $actual, $message = '') {
        $this->total++;
        if ($actual >= $expected) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望大于等于 {$expected}，实际: {$actual}\n";
            return false;
        }
    }
    
    /**
     * 断言数组有指定键
     */
    protected function assertArrayHasKey($key, $array, $message = '') {
        $this->total++;
        if (isset($array[$key])) {
            $this->passed++;
            echo "✓ {$message}\n";
            return true;
        } else {
            $this->failed++;
            echo "✗ {$message}\n";
            echo "  期望数组有键 '{$key}'，实际: " . print_r($array, true) . "\n";
            return false;
        }
    }
    
    /**
     * 运行测试
     */
    public function run() {
        // 获取所有测试方法
        $methods = get_class_methods($this);
        
        // 运行所有以test开头的方法
        foreach ($methods as $method) {
            if (strpos($method, 'test') === 0) {
                echo "\n--- 测试方法: {$method} ---\n";
                $this->{$method}();
            }
        }
        
        $this->summary();
    }
    
    /**
     * 测试结果总结
     */
    protected function summary() {
        echo "\n=== 测试总结: {$this->testName} ===\n";
        echo "总测试数: {$this->total}\n";
        echo "通过数: {$this->passed}\n";
        echo "失败数: {$this->failed}\n";
        echo "通过率: " . ($this->total > 0 ? round(($this->passed / $this->total) * 100, 2) : 0) . "%\n";
        echo "====================\n";
    }
    
    /**
     * 获取测试结果
     */
    public function getResults() {
        return [
            'name' => $this->testName,
            'total' => $this->total,
            'passed' => $this->passed,
            'failed' => $this->failed,
            'passed_rate' => $this->total > 0 ? ($this->passed / $this->total) : 0
        ];
    }
}
?>