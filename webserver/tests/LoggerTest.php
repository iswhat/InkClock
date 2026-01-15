<?php

namespace InkClock\Tests;

use InkClock\Utils\Logger;

class LoggerTest {
    private $logger;
    
    public function __construct() {
        $this->logger = new Logger();
    }
    
    public function testLoggerCreation() {
        echo "Testing logger creation...\n";
        return $this->logger !== null;
    }
    
    public function testLoggerInfo() {
        echo "Testing logger info...\n";
        try {
            $this->logger->info('Test info message', ['key' => 'value']);
            return true;
        } catch (\Exception $e) {
            echo "Error: " . $e->getMessage() . "\n";
            return false;
        }
    }
    
    public function testLoggerError() {
        echo "Testing logger error...\n";
        try {
            $this->logger->error('Test error message', ['error' => 'test']);
            return true;
        } catch (\Exception $e) {
            echo "Error: " . $e->getMessage() . "\n";
            return false;
        }
    }
    
    public function testLoggerWarning() {
        echo "Testing logger warning...\n";
        try {
            $this->logger->warning('Test warning message', ['warning' => 'test']);
            return true;
        } catch (\Exception $e) {
            echo "Error: " . $e->getMessage() . "\n";
            return false;
        }
    }
    
    public function testLoggerDebug() {
        echo "Testing logger debug...\n";
        try {
            $this->logger->debug('Test debug message', ['debug' => 'test']);
            return true;
        } catch (\Exception $e) {
            echo "Error: " . $e->getMessage() . "\n";
            return false;
        }
    }
    
    public function run() {
        echo "Running Logger tests...\n";
        
        $tests = [
            'testLoggerCreation',
            'testLoggerInfo',
            'testLoggerError',
            'testLoggerWarning',
            'testLoggerDebug'
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
$test = new LoggerTest();
$test->run();
?>