<?php
/**
 * 日志工具类
 */

namespace InkClock\Utils;

class Logger {
    private static $instance = null;
    private $logFile;
    private $logLevel;
    private $levels = [
        'debug' => 0,
        'info' => 1,
        'warning' => 2,
        'error' => 3
    ];

    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        $this->logLevel = 'info';
        $this->logFile = __DIR__ . '/../../logs/app.log';
        $this->ensureLogDirectory();
    }

    /**
     * 确保日志目录存在
     */
    private function ensureLogDirectory() {
        $logDir = dirname($this->logFile);
        if (!file_exists($logDir)) {
            mkdir($logDir, 0755, true);
        }
    }

    /**
     * 获取日志实例
     * @return Logger
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }

    /**
     * 设置日志级别
     * @param string $level 日志级别 (debug, info, warning, error)
     */
    public function setLevel($level) {
        if (isset($this->levels[$level])) {
            $this->logLevel = $level;
        }
    }

    /**
     * 设置日志文件路径
     * @param string $filePath 日志文件路径
     */
    public function setLogFile($filePath) {
        $this->logFile = $filePath;
        $this->ensureLogDirectory();
    }

    /**
     * 记录调试日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     */
    public function debug($message, $context = []) {
        $this->log('debug', $message, $context);
    }

    /**
     * 记录信息日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     */
    public function info($message, $context = []) {
        $this->log('info', $message, $context);
    }

    /**
     * 记录警告日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     */
    public function warning($message, $context = []) {
        $this->log('warning', $message, $context);
    }

    /**
     * 记录错误日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     */
    public function error($message, $context = []) {
        $this->log('error', $message, $context);
    }

    /**
     * 记录日志
     * @param string $level 日志级别
     * @param string $message 日志消息
     * @param array $context 上下文信息
     */
    private function log($level, $message, $context = []) {
        // 检查日志级别
        if ($this->levels[$level] < $this->levels[$this->logLevel]) {
            return;
        }

        // 格式化上下文信息
        $contextStr = !empty($context) ? ' ' . json_encode($context, JSON_UNESCAPED_UNICODE) : '';
        
        // 格式化日志消息
        $timestamp = date('Y-m-d H:i:s');
        $logMessage = sprintf("[%s] %s: %s%s\n", $timestamp, strtoupper($level), $message, $contextStr);
        
        // 写入日志文件
        file_put_contents($this->logFile, $logMessage, FILE_APPEND);
    }
}