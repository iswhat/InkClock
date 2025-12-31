<?php
/**
 * 日志工具类
 */

class Logger {
    const DEBUG = 'debug';
    const INFO = 'info';
    const WARNING = 'warning';
    const ERROR = 'error';
    const CRITICAL = 'critical';
    
    private static $instance = null;
    private $logFile;
    private $level;
    
    /**
     * 私有构造函数
     */
    private function __construct() {
        $this->logFile = __DIR__ . '/../logs/app.log';
        $this->level = self::INFO;
        $this->ensureLogDirectory();
    }
    
    /**
     * 确保日志目录存在
     */
    private function ensureLogDirectory() {
        $logDir = dirname($this->logFile);
        if (!is_dir($logDir)) {
            mkdir($logDir, 0755, true);
        }
    }
    
    /**
     * 获取日志实例
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new Logger();
        }
        return self::$instance;
    }
    
    /**
     * 设置日志级别
     */
    public function setLevel($level) {
        $this->level = $level;
    }
    
    /**
     * 设置日志文件路径
     */
    public function setLogFile($filePath) {
        $this->logFile = $filePath;
        $this->ensureLogDirectory();
    }
    
    /**
     * 记录日志
     */
    public function log($level, $message, $context = array()) {
        $levels = array(
            self::DEBUG => 100,
            self::INFO => 200,
            self::WARNING => 300,
            self::ERROR => 400,
            self::CRITICAL => 500
        );
        
        // 检查日志级别
        if ($levels[$level] < $levels[$this->level]) {
            return;
        }
        
        $timestamp = date('Y-m-d H:i:s');
        $contextStr = empty($context) ? '' : ' ' . json_encode($context, JSON_UNESCAPED_UNICODE);
        $logLine = "[$timestamp] [$level] $message$contextStr\n";
        
        // 写入日志文件
        file_put_contents($this->logFile, $logLine, FILE_APPEND);
    }
    
    /**
     * 记录DEBUG级别日志
     */
    public function debug($message, $context = array()) {
        $this->log(self::DEBUG, $message, $context);
    }
    
    /**
     * 记录INFO级别日志
     */
    public function info($message, $context = array()) {
        $this->log(self::INFO, $message, $context);
    }
    
    /**
     * 记录WARNING级别日志
     */
    public function warning($message, $context = array()) {
        $this->log(self::WARNING, $message, $context);
    }
    
    /**
     * 记录ERROR级别日志
     */
    public function error($message, $context = array()) {
        $this->log(self::ERROR, $message, $context);
    }
    
    /**
     * 记录CRITICAL级别日志
     */
    public function critical($message, $context = array()) {
        $this->log(self::CRITICAL, $message, $context);
    }
    
    /**
     * 清理旧日志
     */
    public function cleanOldLogs($days = 30) {
        $cutoffTime = time() - ($days * 24 * 60 * 60);
        
        if (file_exists($this->logFile)) {
            $logLines = file($this->logFile);
            $newLogLines = array();
            
            foreach ($logLines as $line) {
                if (preg_match('/\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\]/', $line, $matches)) {
                    $logTime = strtotime($matches[1]);
                    if ($logTime >= $cutoffTime) {
                        $newLogLines[] = $line;
                    }
                } else {
                    $newLogLines[] = $line;
                }
            }
            
            file_put_contents($this->logFile, implode('', $newLogLines));
        }
    }
}
?>