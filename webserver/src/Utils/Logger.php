<?php
/**
 * 日志工具类
 */

namespace InkClock\Utils;

class Logger {
    private static $instance = null;
    private $logFiles = [];
    private $logLevel;
    private $levels = [
        'debug' => 0,
        'info' => 1,
        'warning' => 2,
        'error' => 3,
        'critical' => 4
    ];
    private $consoleOutput = false;
    private $logRotation = true;
    private $rotationStrategy = 'size'; // size, daily, hourly
    private $maxFileSize = 10485760; // 10MB
    private $maxBackupFiles = 7; // 最大备份文件数
    private $logFormat = 'text'; // text, json
    private $dateFormat = 'Y-m-d H:i:s.u'; // 日志日期格式

    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        $this->logLevel = 'info';
        $this->initLogFiles();
        $this->ensureLogDirectory();
    }

    /**
     * 初始化日志文件配置
     */
    private function initLogFiles() {
        $logDir = __DIR__ . '/../../logs';
        $this->logFiles = [
            'app' => $logDir . '/app.log',
            'error' => $logDir . '/error.log',
            'access' => $logDir . '/access.log',
            'database' => $logDir . '/database.log'
        ];
    }

    /**
     * 确保日志目录存在
     */
    private function ensureLogDirectory() {
        foreach ($this->logFiles as $logFile) {
            $logDir = dirname($logFile);
            if (!file_exists($logDir)) {
                mkdir($logDir, 0755, true);
            }
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
     * @param string $level 日志级别 (debug, info, warning, error, critical)
     */
    public function setLevel($level) {
        if (isset($this->levels[$level])) {
            $this->logLevel = $level;
        }
    }

    /**
     * 启用控制台输出
     * @param bool $enabled 是否启用
     */
    public function setConsoleOutput($enabled) {
        $this->consoleOutput = $enabled;
    }

    /**
     * 设置日志轮转
     * @param bool $enabled 是否启用
     */
    public function setLogRotation($enabled) {
        $this->logRotation = $enabled;
    }

    /**
     * 设置最大日志文件大小
     * @param int $size 大小（字节）
     */
    public function setMaxFileSize($size) {
        $this->maxFileSize = $size;
    }
    
    /**
     * 设置日志轮转策略
     * @param string $strategy 轮转策略 (size, daily, hourly)
     */
    public function setRotationStrategy($strategy) {
        if (in_array($strategy, ['size', 'daily', 'hourly'])) {
            $this->rotationStrategy = $strategy;
        }
    }
    
    /**
     * 设置最大备份文件数
     * @param int $count 最大备份文件数
     */
    public function setMaxBackupFiles($count) {
        $this->maxBackupFiles = $count;
    }
    
    /**
     * 设置日志格式
     * @param string $format 日志格式 (text, json)
     */
    public function setLogFormat($format) {
        if (in_array($format, ['text', 'json'])) {
            $this->logFormat = $format;
        }
    }
    
    /**
     * 设置日期格式
     * @param string $format 日期格式
     */
    public function setDateFormat($format) {
        $this->dateFormat = $format;
    }

    /**
     * 记录调试日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     * @param string $logType 日志类型
     */
    public function debug($message, $context = [], $logType = 'app') {
        $this->log('debug', $message, $context, $logType);
    }

    /**
     * 记录信息日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     * @param string $logType 日志类型
     */
    public function info($message, $context = [], $logType = 'app') {
        $this->log('info', $message, $context, $logType);
    }

    /**
     * 记录警告日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     * @param string $logType 日志类型
     */
    public function warning($message, $context = [], $logType = 'app') {
        $this->log('warning', $message, $context, $logType);
    }

    /**
     * 记录错误日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     * @param string $logType 日志类型
     */
    public function error($message, $context = [], $logType = 'error') {
        $this->log('error', $message, $context, $logType);
    }

    /**
     * 记录致命错误日志
     * @param string $message 日志消息
     * @param array $context 上下文信息
     * @param string $logType 日志类型
     */
    public function critical($message, $context = [], $logType = 'error') {
        $this->log('critical', $message, $context, $logType);
    }

    /**
     * 记录访问日志
     * @param string $method 请求方法
     * @param string $path 请求路径
     * @param int $statusCode 状态码
     * @param float $duration 响应时间（秒）
     * @param string $ip IP地址
     * @param array $additional 额外信息
     */
    public function accessLog($method, $path, $statusCode, $duration, $ip, $additional = []) {
        $context = array_merge([
            'method' => $method,
            'path' => $path,
            'status_code' => $statusCode,
            'duration' => $duration,
            'ip' => $ip
        ], $additional);
        
        $message = sprintf("%s %s %d %.3fms", $method, $path, $statusCode, $duration * 1000);
        $this->log('info', $message, $context, 'access');
    }

    /**
     * 记录数据库查询日志
     * @param string $query SQL查询
     * @param array $params 查询参数
     * @param float $duration 执行时间（秒）
     */
    public function databaseLog($query, $params = [], $duration = 0) {
        $context = [
            'query' => $query,
            'params' => $params,
            'duration' => $duration
        ];
        
        $message = sprintf("Query executed in %.3fms", $duration * 1000);
        $this->log('info', $message, $context, 'database');
    }

    /**
     * 记录日志
     * @param string $level 日志级别
     * @param string $message 日志消息
     * @param array $context 上下文信息
     * @param string $logType 日志类型
     */
    private function log($level, $message, $context = [], $logType = 'app') {
        // 检查日志级别
        if ($this->levels[$level] < $this->levels[$this->logLevel]) {
            return;
        }

        // 获取日志文件路径
        $logFile = $this->logFiles[$logType] ?? $this->logFiles['app'];
        
        // 检查日志文件，进行轮转
        $this->checkLogRotation($logFile);

        // 格式化日志消息
        $logMessage = $this->formatLogMessage($level, $message, $context);
        
        // 写入日志文件
        file_put_contents($logFile, $logMessage, FILE_APPEND);
        
        // 控制台输出
        if ($this->consoleOutput) {
            $this->outputToConsole($level, $logMessage);
        }
    }

    /**
     * 检查日志文件，进行轮转
     * @param string $logFile 日志文件路径
     */
    private function checkLogRotation($logFile) {
        if (!$this->logRotation || !file_exists($logFile)) {
            return;
        }
        
        $shouldRotate = false;
        $rotationSuffix = '';
        
        switch ($this->rotationStrategy) {
            case 'size':
                // 按大小轮转
                if (filesize($logFile) >= $this->maxFileSize) {
                    $shouldRotate = true;
                    $rotationSuffix = date('Y-m-d_H-i-s');
                }
                break;
            case 'daily':
                // 按天轮转
                $currentDate = date('Y-m-d');
                $lastRotationDate = $this->getLastRotationDate($logFile);
                if ($currentDate !== $lastRotationDate) {
                    $shouldRotate = true;
                    $rotationSuffix = $lastRotationDate;
                }
                break;
            case 'hourly':
                // 按小时轮转
                $currentHour = date('Y-m-d_H');
                $lastRotationHour = $this->getLastRotationHour($logFile);
                if ($currentHour !== $lastRotationHour) {
                    $shouldRotate = true;
                    $rotationSuffix = $lastRotationHour;
                }
                break;
        }
        
        if ($shouldRotate) {
            $backupFile = $logFile . '.' . $rotationSuffix . '.bak';
            rename($logFile, $backupFile);
            $this->cleanupOldBackups($logFile);
        }
    }
    
    /**
     * 获取日志文件的最后轮转日期
     * @param string $logFile 日志文件路径
     * @return string 最后轮转日期
     */
    private function getLastRotationDate($logFile) {
        if (!file_exists($logFile)) {
            return date('Y-m-d');
        }
        
        $mtime = filemtime($logFile);
        return date('Y-m-d', $mtime);
    }
    
    /**
     * 获取日志文件的最后轮转小时
     * @param string $logFile 日志文件路径
     * @return string 最后轮转小时
     */
    private function getLastRotationHour($logFile) {
        if (!file_exists($logFile)) {
            return date('Y-m-d_H');
        }
        
        $mtime = filemtime($logFile);
        return date('Y-m-d_H', $mtime);
    }
    
    /**
     * 清理旧的备份文件
     * @param string $logFile 日志文件路径
     */
    private function cleanupOldBackups($logFile) {
        $backupPattern = $logFile . '.*.bak';
        $backupFiles = glob($backupPattern);
        
        if (count($backupFiles) > $this->maxBackupFiles) {
            // 按修改时间排序，保留最新的文件
            usort($backupFiles, function($a, $b) {
                return filemtime($b) - filemtime($a);
            });
            
            // 删除多余的备份文件
            $filesToDelete = array_slice($backupFiles, $this->maxBackupFiles);
            foreach ($filesToDelete as $file) {
                unlink($file);
            }
        }
    }

    /**
     * 格式化日志消息
     * @param string $level 日志级别
     * @param string $message 日志消息
     * @param array $context 上下文信息
     * @return string 格式化后的日志消息
     */
    private function formatLogMessage($level, $message, $context) {
        $timestamp = date($this->dateFormat);
        
        if ($this->logFormat === 'json') {
            $logData = array_merge([
                'timestamp' => $timestamp,
                'level' => strtoupper($level),
                'message' => $message
            ], $context);
            
            return json_encode($logData, JSON_UNESCAPED_UNICODE) . "\n";
        } else {
            // 文本格式
            $contextStr = !empty($context) ? ' ' . json_encode($context, JSON_UNESCAPED_UNICODE) : '';
            return sprintf("[%s] %s: %s%s\n", $timestamp, strtoupper($level), $message, $contextStr);
        }
    }

    /**
     * 输出日志到控制台
     * @param string $level 日志级别
     * @param string $logMessage 日志消息
     */
    private function outputToConsole($level, $logMessage) {
        // 根据日志级别设置颜色
        $colors = [
            'debug' => '\033[0;36m',     // 青色
            'info' => '\033[0;32m',      // 绿色
            'warning' => '\033[0;33m',   // 黄色
            'error' => '\033[0;31m',     // 红色
            'critical' => '\033[1;31m'   // 亮红色
        ];
        
        $color = $colors[$level] ?? '\033[0m';
        $reset = '\033[0m';
        
        echo $color . $logMessage . $reset;
    }
}