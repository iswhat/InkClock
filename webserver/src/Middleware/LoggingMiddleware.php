<?php
/**
 * 日志中间件 - 记录所有传入请求
 */

namespace App\Middleware;

use App\Utils\Logger;

class LoggingMiddleware implements MiddlewareInterface {
    private $logger;
    
    /**
     * 构造函数
     * @param Logger $logger 日志服务
     */
    public function __construct(Logger $logger) {
        $this->logger = $logger;
    }
    
    /**
     * 处理请求
     * @param array $request 请求信息
     * @param callable $next 下一个中间件或处理函数
     * @return mixed 处理结果
     */
    public function handle($request, $next) {
        // 记录请求开始时间
        $startTime = microtime(true);
        
        // 记录请求信息
        $this->logger->info('API请求', [
            'method' => $request['method'],
            'path' => $request['path'],
            'ip' => $request['ip'],
            'user_agent' => $request['headers']['User-Agent'] ?? 'Unknown'
        ]);
        
        // 调用下一个中间件或处理函数
        $result = $next($request);
        
        // 记录请求结束时间和执行时间
        $endTime = microtime(true);
        $executionTime = round(($endTime - $startTime) * 1000, 2);
        
        $this->logger->info('API响应', [
            'method' => $request['method'],
            'path' => $request['path'],
            'execution_time' => $executionTime . 'ms',
            'status_code' => $result['status_code'] ?? 200
        ]);
        
        return $result;
    }
}