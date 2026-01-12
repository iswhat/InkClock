<?php
/**
 * 监控控制器
 * 用于查看系统状态和性能指标
 */

namespace InkClock\Controller;

use InkClock\Utils\DIContainer;

class MonitoringController extends BaseController {
    /**
     * 构造函数
     * @param DIContainer $container 依赖注入容器
     */
    public function __construct(DIContainer $container) {
        parent::__construct($container);
    }
    
    /**
     * 获取系统状态
     */
    public function getSystemStatus() {
        $this->logAction('monitoring_system_status');
        
        // 系统信息
        $systemInfo = [
            'php_version' => PHP_VERSION,
            'server_software' => $_SERVER['SERVER_SOFTWARE'] ?? 'Unknown',
            'os' => PHP_OS,
            'memory_usage' => [
                'current' => round(memory_get_usage() / 1024 / 1024, 2) . 'MB',
                'peak' => round(memory_get_peak_usage() / 1024 / 1024, 2) . 'MB',
                'limit' => ini_get('memory_limit')
            ],
            'execution_time' => round((microtime(true) - $_SERVER['REQUEST_TIME_FLOAT']) * 1000, 2) . 'ms',
            'uptime' => $this->getServerUptime(),
            'timestamp' => time(),
            'time' => date('Y-m-d H:i:s'),
            'disk_usage' => $this->getDiskUsage(),
            'network' => $this->getNetworkStatus()
        ];
        
        // API统计
        $apiStats = [
            'version' => '1.0.0',
            'endpoints' => $this->countApiEndpoints(),
            'supported_versions' => ['v1']
        ];
        
        // 数据库状态
        $dbStatus = $this->getDatabaseStatus();
        
        // 缓存状态
        $cacheStatus = $this->getCacheStatus();
        
        // 插件状态
        $pluginStatus = $this->getPluginStatus();
        
        $this->response->success('系统状态获取成功', [
            'system' => $systemInfo,
            'api' => $apiStats,
            'database' => $dbStatus,
            'cache' => $cacheStatus,
            'plugins' => $pluginStatus
        ]);
    }
    
    /**
     * 获取API性能指标
     */
    public function getApiMetrics() {
        $this->logAction('monitoring_api_metrics');
        
        // 从缓存或数据库获取API性能指标
        $metrics = $this->collectApiMetrics();
        
        $this->response->success('API性能指标获取成功', $metrics);
    }
    
    /**
     * 获取日志分析
     */
    public function getLogAnalysis() {
        $this->logAction('monitoring_log_analysis');
        
        // 分析日志文件
        $analysis = $this->analyzeLogs();
        
        $this->response->success('日志分析获取成功', $analysis);
    }
    
    /**
     * 获取服务器 uptime
     * @return string 服务器运行时间
     */
    private function getServerUptime() {
        // 尝试获取服务器运行时间
        if (function_exists('sys_getloadavg')) {
            $load = sys_getloadavg();
            return [
                'load' => [
                    '1min' => $load[0],
                    '5min' => $load[1],
                    '15min' => $load[2]
                ]
            ];
        }
        return 'Unknown';
    }
    
    /**
     * 获取磁盘使用情况
     * @return array 磁盘使用情况
     */
    private function getDiskUsage() {
        $diskUsage = [];
        
        // 尝试获取当前目录的磁盘使用情况
        $diskFree = disk_free_space(__DIR__);
        $diskTotal = disk_total_space(__DIR__);
        $diskUsed = $diskTotal - $diskFree;
        
        $diskUsage = [
            'total' => round($diskTotal / 1024 / 1024 / 1024, 2) . 'GB',
            'used' => round($diskUsed / 1024 / 1024 / 1024, 2) . 'GB',
            'free' => round($diskFree / 1024 / 1024 / 1024, 2) . 'GB',
            'usage_percent' => $diskTotal > 0 ? round(($diskUsed / $diskTotal) * 100, 2) . '%' : '0%'
        ];
        
        return $diskUsage;
    }
    
    /**
     * 获取网络状态
     * @return array 网络状态
     */
    private function getNetworkStatus() {
        $network = [
            'server_ip' => $_SERVER['SERVER_ADDR'] ?? 'Unknown',
            'client_ip' => $_SERVER['REMOTE_ADDR'] ?? 'Unknown',
            'server_port' => $_SERVER['SERVER_PORT'] ?? 'Unknown',
            'protocol' => $_SERVER['SERVER_PROTOCOL'] ?? 'Unknown'
        ];
        
        // 尝试测试外部连接
        $network['external_connectivity'] = $this->testExternalConnectivity();
        
        return $network;
    }
    
    /**
     * 测试外部连接
     * @return bool 是否可以连接到外部网络
     */
    private function testExternalConnectivity() {
        try {
            $ch = curl_init('https://www.baidu.com');
            curl_setopt($ch, CURLOPT_TIMEOUT, 5);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
            $response = curl_exec($ch);
            $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            curl_close($ch);
            return $httpCode >= 200 && $httpCode < 400;
        } catch (\Exception $e) {
            return false;
        }
    }
    
    /**
     * 统计API端点数量
     * @return int API端点数量
     */
    private function countApiEndpoints() {
        $routes = require __DIR__ . '/../../config/routes.php';
        return count($routes);
    }
    
    /**
     * 获取数据库状态
     * @return array 数据库状态
     */
    private function getDatabaseStatus() {
        try {
            $db = $this->container->get('db');
            
            // 尝试执行一个简单的查询来检查数据库连接
            $startTime = microtime(true);
            $stmt = $db->query('SELECT 1');
            $endTime = microtime(true);
            $connected = $stmt !== false;
            $responseTime = round(($endTime - $startTime) * 1000, 2) . 'ms';
            
            // 获取数据库文件大小
            $dbPath = __DIR__ . '/../../data/inkclock.db';
            $dbSize = file_exists($dbPath) ? filesize($dbPath) : 0;
            $dbSizeFormatted = round($dbSize / 1024 / 1024, 2) . 'MB';
            
            return [
                'connected' => $connected,
                'type' => 'SQLite3',
                'response_time' => $responseTime,
                'database_size' => $dbSizeFormatted,
                'error' => $connected ? null : '无法连接到数据库'
            ];
        } catch (\Exception $e) {
            return [
                'connected' => false,
                'type' => 'SQLite3',
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * 获取缓存状态
     * @return array 缓存状态
     */
    private function getCacheStatus() {
        try {
            $cache = $this->container->get('cache');
            
            // 尝试设置和获取缓存来检查缓存是否正常工作
            $testKey = 'cache_test_' . time();
            $startTime = microtime(true);
            $cache->set($testKey, 'test_value', 1);
            $value = $cache->get($testKey);
            $endTime = microtime(true);
            $cache->delete($testKey);
            
            $responseTime = round(($endTime - $startTime) * 1000, 2) . 'ms';
            
            // 获取缓存目录大小
            $cacheDir = __DIR__ . '/../../cache';
            $cacheSize = $this->getDirectorySize($cacheDir);
            $cacheSizeFormatted = round($cacheSize / 1024 / 1024, 2) . 'MB';
            
            return [
                'working' => $value === 'test_value',
                'type' => 'FileCache',
                'response_time' => $responseTime,
                'cache_size' => $cacheSizeFormatted,
                'error' => $value === 'test_value' ? null : '缓存操作失败'
            ];
        } catch (\Exception $e) {
            return [
                'working' => false,
                'type' => 'FileCache',
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * 获取插件状态
     * @return array 插件状态
     */
    private function getPluginStatus() {
        try {
            $pluginConfig = require __DIR__ . '/../../plugin/plugin.json';
            $plugins = [];
            
            foreach ($pluginConfig as $plugin) {
                $pluginPath = __DIR__ . '/../../plugin/' . basename(dirname($plugin['url']));
                $pluginFile = $pluginPath . '/index.php';
                
                $plugins[] = [
                    'name' => $plugin['name'],
                    'status' => file_exists($pluginFile) ? 'active' : 'inactive',
                    'path' => $plugin['url'],
                    'refresh_interval' => $plugin['refresh_interval']
                ];
            }
            
            return [
                'total' => count($plugins),
                'active' => count(array_filter($plugins, function($p) { return $p['status'] === 'active'; })),
                'plugins' => $plugins
            ];
        } catch (\Exception $e) {
            return [
                'total' => 0,
                'active' => 0,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * 收集API性能指标
     * @return array API性能指标
     */
    private function collectApiMetrics() {
        // 从数据库或缓存中读取历史性能数据
        // 这里使用模拟数据，实际项目中应该从持久化存储中读取
        
        $metrics = [
            'requests_total' => $this->getTotalRequests(),
            'requests_per_minute' => $this->getRequestsPerMinute(),
            'average_response_time' => $this->getAverageResponseTime(),
            'slow_requests' => $this->getSlowRequests(),
            'error_rate' => $this->getErrorRate(),
            'top_endpoints' => $this->getTopEndpoints(),
            'status_codes' => $this->getStatusCodes(),
            'response_times' => $this->getResponseTimes()
        ];
        
        return $metrics;
    }
    
    /**
     * 获取总请求数
     * @return int 总请求数
     */
    private function getTotalRequests() {
        // 模拟数据，实际应该从日志或数据库中统计
        return rand(1000, 5000);
    }
    
    /**
     * 获取每分钟请求数
     * @return int 每分钟请求数
     */
    private function getRequestsPerMinute() {
        // 模拟数据，实际应该从日志或数据库中统计
        return rand(10, 50);
    }
    
    /**
     * 获取平均响应时间
     * @return float 平均响应时间（毫秒）
     */
    private function getAverageResponseTime() {
        // 模拟数据，实际应该从日志或数据库中统计
        return round(rand(50, 200) / 10, 1);
    }
    
    /**
     * 获取慢请求数
     * @return int 慢请求数
     */
    private function getSlowRequests() {
        // 模拟数据，实际应该从日志或数据库中统计
        return rand(0, 10);
    }
    
    /**
     * 获取错误率
     * @return float 错误率（百分比）
     */
    private function getErrorRate() {
        // 模拟数据，实际应该从日志或数据库中统计
        return round(rand(0, 5) / 10, 2);
    }
    
    /**
     * 获取热门端点
     * @return array 热门端点
     */
    private function getTopEndpoints() {
        // 模拟数据，实际应该从日志或数据库中统计
        return [
            ['endpoint' => '/api/device', 'requests' => rand(100, 500), 'avg_time' => round(rand(50, 150) / 10, 1)],
            ['endpoint' => '/api/user/login', 'requests' => rand(50, 200), 'avg_time' => round(rand(100, 250) / 10, 1)],
            ['endpoint' => '/api/message', 'requests' => rand(30, 150), 'avg_time' => round(rand(80, 200) / 10, 1)],
            ['endpoint' => '/api/status', 'requests' => rand(20, 100), 'avg_time' => round(rand(30, 100) / 10, 1)],
            ['endpoint' => '/api/plugin', 'requests' => rand(10, 50), 'avg_time' => round(rand(150, 300) / 10, 1)]
        ];
    }
    
    /**
     * 获取状态码统计
     * @return array 状态码统计
     */
    private function getStatusCodes() {
        // 模拟数据，实际应该从日志或数据库中统计
        return [
            '200' => rand(800, 4000),
            '400' => rand(10, 50),
            '401' => rand(5, 30),
            '403' => rand(2, 10),
            '404' => rand(10, 40),
            '500' => rand(1, 20)
        ];
    }
    
    /**
     * 获取响应时间分布
     * @return array 响应时间分布
     */
    private function getResponseTimes() {
        // 模拟数据，实际应该从日志或数据库中统计
        return [
            '<50ms' => rand(40, 60),
            '50-100ms' => rand(20, 30),
            '100-200ms' => rand(10, 20),
            '200-500ms' => rand(5, 10),
            '>500ms' => rand(1, 5)
        ];
    }
    
    /**
     * 分析日志文件
     * @return array 日志分析结果
     */
    private function analyzeLogs() {
        try {
            $logDir = __DIR__ . '/../../logs';
            $analysis = [
                'recent_errors' => [],
                'recent_warnings' => [],
                'top_errors' => [],
                'log_size' => 0,
                'last_log_rotation' => null
            ];
            
            // 检查日志目录
            if (is_dir($logDir)) {
                $logFiles = glob($logDir . '/*.log');
                $totalSize = 0;
                
                foreach ($logFiles as $logFile) {
                    $totalSize += filesize($logFile);
                    
                    // 读取最近的日志条目
                    $lines = file($logFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
                    if ($lines) {
                        // 取最近的100行
                        $recentLines = array_slice($lines, -100);
                        
                        foreach ($recentLines as $line) {
                            if (strpos($line, 'ERROR') !== false) {
                                $analysis['recent_errors'][] = [
                                    'message' => $line,
                                    'file' => basename($logFile),
                                    'timestamp' => $this->extractTimestamp($line)
                                ];
                            } elseif (strpos($line, 'WARNING') !== false) {
                                $analysis['recent_warnings'][] = [
                                    'message' => $line,
                                    'file' => basename($logFile),
                                    'timestamp' => $this->extractTimestamp($line)
                                ];
                            }
                        }
                    }
                }
                
                $analysis['log_size'] = round($totalSize / 1024 / 1024, 2) . 'MB';
                $analysis['last_log_rotation'] = $this->getLastLogRotation($logDir);
            }
            
            // 限制返回的错误和警告数量
            $analysis['recent_errors'] = array_slice($analysis['recent_errors'], 0, 20);
            $analysis['recent_warnings'] = array_slice($analysis['recent_warnings'], 0, 20);
            
            // 计算top errors
            $analysis['top_errors'] = $this->getTopErrors($analysis['recent_errors']);
            
            return $analysis;
        } catch (\Exception $e) {
            return [
                'recent_errors' => [],
                'recent_warnings' => [],
                'top_errors' => [],
                'log_size' => 0,
                'last_log_rotation' => null,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * 提取日志时间戳
     * @param string $line 日志行
     * @return string 时间戳
     */
    private function extractTimestamp($line) {
        // 简单的时间戳提取，实际项目中应该根据日志格式进行调整
        $pattern = '/\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\]/';
        if (preg_match($pattern, $line, $matches)) {
            return $matches[1];
        }
        return date('Y-m-d H:i:s');
    }
    
    /**
     * 获取最后一次日志轮转时间
     * @param string $logDir 日志目录
     * @return string 最后一次日志轮转时间
     */
    private function getLastLogRotation($logDir) {
        $logFiles = glob($logDir . '/*.log');
        if ($logFiles) {
            $lastModified = 0;
            foreach ($logFiles as $logFile) {
                $mtime = filemtime($logFile);
                if ($mtime > $lastModified) {
                    $lastModified = $mtime;
                }
            }
            return date('Y-m-d H:i:s', $lastModified);
        }
        return null;
    }
    
    /**
     * 获取top错误
     * @param array $errors 错误列表
     * @return array top错误
     */
    private function getTopErrors($errors) {
        $errorCounts = [];
        
        foreach ($errors as $error) {
            $message = $error['message'];
            if (isset($errorCounts[$message])) {
                $errorCounts[$message]++;
            } else {
                $errorCounts[$message] = 1;
            }
        }
        
        // 按错误数量排序
        arsort($errorCounts);
        
        // 取前5个
        $topErrors = [];
        $count = 0;
        foreach ($errorCounts as $message => $countValue) {
            $topErrors[] = [
                'message' => substr($message, 0, 200),
                'count' => $countValue
            ];
            $count++;
            if ($count >= 5) {
                break;
            }
        }
        
        return $topErrors;
    }
    
    /**
     * 获取目录大小
     * @param string $dir 目录路径
     * @return int 目录大小（字节）
     */
    private function getDirectorySize($dir) {
        $size = 0;
        if (is_dir($dir)) {
            $files = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($dir));
            foreach ($files as $file) {
                if ($file->isFile()) {
                    $size += $file->getSize();
                }
            }
        }
        return $size;
    }
}
?>