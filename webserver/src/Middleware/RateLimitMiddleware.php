<?php
/**
 * 速率限制中间件
 */

namespace InkClock\Middleware;

use InkClock\Utils\Logger;
use InkClock\Utils\Response;

class RateLimitMiddleware implements MiddlewareInterface {
    private $logger;
    private $response;
    private $rateLimits = [];
    private $cache;
    
    /**
     * 构造函数
     * @param \InkClock\Utils\Logger $logger 日志服务
     * @param \InkClock\Utils\Response $response 响应服务
     * @param \InkClock\Utils\Cache $cache 缓存服务
     */
    public function __construct($logger = null, $response = null, $cache = null) {
        if ($logger === null) {
            $logger = \InkClock\Utils\Logger::getInstance();
        }
        if ($response === null) {
            $response = \InkClock\Utils\Response::getInstance();
        }
        if ($cache === null) {
            $config = require __DIR__ . '/../../config/config.php';
            $cache = new \InkClock\Utils\Cache($config['cache']['dir'], $config['cache']['expire']);
        }
        $this->logger = $logger;
        $this->response = $response;
        $this->cache = $cache;
        
        // 初始化速率限制配置
        $this->initRateLimits();
    }
    
    /**
     * 初始化速率限制配置
     */
    private function initRateLimits() {
        // 配置速率限制规则
        // 格式：[路径模式 => [请求次数, 时间窗口(秒)]]
        $this->rateLimits = [
            // API全局限制：60次/分钟
            'api/*' => [60, 60],
            // 认证相关API：10次/分钟
            'api/auth/*' => [10, 60],
            // 消息发送API：30次/分钟
            'api/messages/*' => [30, 60],
            // 设备管理API：20次/分钟
            'api/devices/*' => [20, 60],
        ];
    }
    
    /**
     * 处理中间件
     * @param array $request 请求信息
     * @param callable $next 下一个中间件
     * @return mixed 响应结果
     */
    public function handle($request, callable $next) {
        $this->logger->info('速率限制中间件执行', ['request' => $request]);
        
        // 获取客户端IP
        $clientIp = $request['ip'] ?? 'unknown';
        
        // 获取请求路径
        $path = $request['path'] ?? '';
        
        // 查找匹配的速率限制规则
        $limitRule = $this->findMatchingLimitRule($path);
        
        // 如果没有找到匹配的规则，直接放行
        if (!$limitRule) {
            return $next($request);
        }
        
        // 获取速率限制配置
        list($maxRequests, $timeWindow) = $limitRule;
        
        // 生成速率限制键
        $rateLimitKey = $this->generateRateLimitKey($clientIp, $path);
        
        // 检查速率限制
        if (!$this->checkRateLimit($rateLimitKey, $maxRequests, $timeWindow)) {
            $this->logger->warning('速率限制触发', [
                'ip' => $clientIp,
                'path' => $path,
                'max_requests' => $maxRequests,
                'time_window' => $timeWindow
            ]);
            
            // 返回429 Too Many Requests
            $response = $this->response->error(429, 'Too many requests');
            
            // 添加速率限制响应头
            $response['headers']['X-RateLimit-Limit'] = $maxRequests;
            $response['headers']['X-RateLimit-Remaining'] = 0;
            $response['headers']['X-RateLimit-Reset'] = time() + $timeWindow;
            
            return $response;
        }
        
        // 速率限制通过，继续执行下一个中间件
        return $next($request);
    }
    
    /**
     * 查找匹配的速率限制规则
     * @param string $path 请求路径
     * @return array|null 匹配的规则
     */
    private function findMatchingLimitRule($path) {
        foreach ($this->rateLimits as $pattern => $limit) {
            // 将模式转换为正则表达式
            $regex = str_replace('*', '.*', $pattern);
            $regex = '/^' . $regex . '$/';
            
            if (preg_match($regex, $path)) {
                return $limit;
            }
        }
        return null;
    }
    
    /**
     * 生成速率限制键
     * @param string $ip 客户端IP
     * @param string $path 请求路径
     * @return string 速率限制键
     */
    private function generateRateLimitKey($ip, $path) {
        // 为每个IP和路径生成唯一键
        // 实际应用中，应该使用Redis等缓存系统存储
        return "rate_limit:{$ip}:{$path}";
    }
    
    /**
     * 检查速率限制
     * @param string $key 速率限制键
     * @param int $maxRequests 最大请求次数
     * @param int $timeWindow 时间窗口（秒）
     * @return bool 是否允许请求
     */
    private function checkRateLimit($key, $maxRequests, $timeWindow) {
        // 实际应用中，应该使用Redis等缓存系统存储请求计数
        // 这里使用文件系统简单实现，仅用于演示
        
        $storageDir = __DIR__ . '/../../storage/rate_limits';
        
        // 创建存储目录
        if (!is_dir($storageDir)) {
            mkdir($storageDir, 0755, true);
        }
        
        $storageFile = $storageDir . '/' . md5($key) . '.json';
        
        // 获取当前时间
        $now = time();
        
        // 读取现有计数
        $requests = [];
        if (file_exists($storageFile)) {
            $content = file_get_contents($storageFile);
            $requests = json_decode($content, true) ?: [];
        }
        
        // 过滤掉过期的请求
        $requests = array_filter($requests, function($timestamp) use ($now, $timeWindow) {
            return $now - $timestamp < $timeWindow;
        });
        
        // 检查请求次数是否超过限制
        if (count($requests) >= $maxRequests) {
            return false;
        }
        
        // 添加当前请求时间戳
        $requests[] = $now;
        
        // 保存更新后的请求计数
        file_put_contents($storageFile, json_encode($requests));
        
        return true;
    }
}
