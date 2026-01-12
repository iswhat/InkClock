<?php
/**
 * 日志中间件 - 记录所有传入请求
 */

namespace InkClock\Middleware;

use InkClock\Utils\Logger;

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
        $requestId = uniqid('req_', true);
        
        // 提取请求体（限制大小）
        $requestBody = $this->extractRequestBody($request);
        
        // 记录请求信息
        $this->logger->info('API请求', [
            'request_id' => $requestId,
            'method' => $request['method'],
            'path' => $request['path'],
            'query' => $request['query'] ?? [],
            'ip' => $request['ip'],
            'user_agent' => $request['headers']['User-Agent'] ?? 'Unknown',
            'api_version' => $request['api_version'] ?? 'unknown',
            'content_type' => $request['headers']['Content-Type'] ?? 'unknown',
            'body' => $requestBody
        ]);
        
        // 调用下一个中间件或处理函数
        try {
            $result = $next($request);
            
            // 记录请求结束时间和执行时间
            $endTime = microtime(true);
            $executionTime = round(($endTime - $startTime) * 1000, 2);
            
            // 提取响应体（限制大小）
            $responseBody = $this->extractResponseBody($result);
            
            $statusCode = $result['status_code'] ?? 200;
            $logLevel = $statusCode >= 500 ? 'error' : ($statusCode >= 400 ? 'warning' : 'info');
            
            $this->logger->$logLevel('API响应', [
                'request_id' => $requestId,
                'method' => $request['method'],
                'path' => $request['path'],
                'execution_time' => $executionTime . 'ms',
                'status_code' => $statusCode,
                'response' => $responseBody,
                'memory_usage' => round(memory_get_usage() / 1024 / 1024, 2) . 'MB'
            ]);
            
            // 记录性能指标
            $this->recordPerformanceMetrics($request, $executionTime, $statusCode);
            
            return $result;
        } catch (\Exception $e) {
            // 记录异常信息
            $endTime = microtime(true);
            $executionTime = round(($endTime - $startTime) * 1000, 2);
            
            $this->logger->error('API异常', [
                'request_id' => $requestId,
                'method' => $request['method'],
                'path' => $request['path'],
                'execution_time' => $executionTime . 'ms',
                'exception' => $e->getMessage(),
                'trace' => $e->getTraceAsString(),
                'file' => $e->getFile(),
                'line' => $e->getLine()
            ]);
            
            // 重新抛出异常
            throw $e;
        }
    }
    
    /**
     * 提取请求体（限制大小）
     * @param array $request 请求信息
     * @return mixed 提取的请求体
     */
    private function extractRequestBody($request) {
        $body = $request['body'] ?? '';
        if (empty($body)) {
            return null;
        }
        
        // 限制请求体大小
        $maxSize = 1024 * 10; // 10KB
        if (strlen($body) > $maxSize) {
            return substr($body, 0, $maxSize) . '... (truncated)';
        }
        
        // 尝试解析JSON
        $jsonBody = json_decode($body, true);
        if (json_last_error() === JSON_ERROR_NONE) {
            return $jsonBody;
        }
        
        return $body;
    }
    
    /**
     * 提取响应体（限制大小）
     * @param mixed $result 响应结果
     * @return mixed 提取的响应体
     */
    private function extractResponseBody($result) {
        if (!is_array($result)) {
            return null;
        }
        
        // 移除敏感信息
        $safeResult = $this->removeSensitiveInfo($result);
        
        // 限制响应体大小
        $jsonResult = json_encode($safeResult);
        $maxSize = 1024 * 10; // 10KB
        if (strlen($jsonResult) > $maxSize) {
            return json_decode(substr($jsonResult, 0, $maxSize) . '... (truncated)', true);
        }
        
        return $safeResult;
    }
    
    /**
     * 移除敏感信息
     * @param array $data 数据
     * @return array 移除敏感信息后的数据
     */
    private function removeSensitiveInfo($data) {
        if (!is_array($data)) {
            return $data;
        }
        
        $sensitiveKeys = ['password', 'token', 'api_key', 'secret', 'key'];
        $result = [];
        
        foreach ($data as $key => $value) {
            $lowerKey = strtolower($key);
            if (in_array($lowerKey, $sensitiveKeys)) {
                $result[$key] = '***redacted***';
            } elseif (is_array($value)) {
                $result[$key] = $this->removeSensitiveInfo($value);
            } else {
                $result[$key] = $value;
            }
        }
        
        return $result;
    }
    
    /**
     * 记录性能指标
     * @param array $request 请求信息
     * @param float $executionTime 执行时间（毫秒）
     * @param int $statusCode 状态码
     */
    private function recordPerformanceMetrics($request, $executionTime, $statusCode) {
        // 这里可以实现性能指标的记录，如写入数据库或缓存
        // 例如：
        // $this->metrics->record([
        //     'endpoint' => $request['path'],
        //     'method' => $request['method'],
        //     'execution_time' => $executionTime,
        //     'status_code' => $statusCode,
        //     'timestamp' => time()
        // ]);
        
        // 暂时只记录到日志
        if ($executionTime > 1000) { // 超过1秒的请求
            $this->logger->warning('性能警告', [
                'method' => $request['method'],
                'path' => $request['path'],
                'execution_time' => $executionTime . 'ms',
                'threshold' => '1000ms'
            ]);
        }
    }
}