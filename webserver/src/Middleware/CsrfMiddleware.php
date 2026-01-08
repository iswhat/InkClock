<?php
/**
 * CSRF防护中间件
 */

namespace InkClock\Middleware;

use InkClock\Utils\Logger;
use InkClock\Utils\Response;

class CsrfMiddleware implements MiddlewareInterface {
    private $logger;
    private $response;
    
    /**
     * 构造函数
     * @param \InkClock\Utils\Logger $logger 日志服务
     * @param \InkClock\Utils\Response $response 响应服务
     */
    public function __construct($logger = null, $response = null) {
        if ($logger === null) {
            $logger = Logger::getInstance();
        }
        if ($response === null) {
            $response = Response::getInstance();
        }
        $this->logger = $logger;
        $this->response = $response;
    }
    
    /**
     * 处理请求
     * @param array $request 请求信息
     * @param mixed $next 下一个中间件或处理函数
     * @return mixed 响应结果
     */
    public function handle($request, $next) {
        // 记录请求
        $this->logger->info('CSRF防护中间件执行', ['request' => $request]);
        
        // 获取请求方法
        $method = strtoupper($request['method']);
        
        // 对于GET、HEAD、OPTIONS等安全方法，直接放行
        $safeMethods = ['GET', 'HEAD', 'OPTIONS'];
        if (in_array($method, $safeMethods)) {
            return $next($request);
        }
        
        // 对于不安全的方法（POST、PUT、DELETE等），进行CSRF验证
        // 从请求头或表单中获取CSRF令牌
        $csrfToken = $this->getCsrfTokenFromRequest($request);
        
        // 验证CSRF令牌
        if (!$this->validateCsrfToken($csrfToken, $request)) {
            $this->logger->warning('CSRF令牌验证失败', ['token' => $csrfToken]);
            $this->response->error('Invalid or missing CSRF token', 403, 'CSRF_TOKEN_INVALID');
        }
        
        // CSRT验证通过，继续执行下一个中间件
        return $next($request);
    }
    
    /**
     * 从请求中获取CSRF令牌
     * @param array $request 请求信息
     * @return string|null CSRF令牌
     */
    private function getCsrfTokenFromRequest($request) {
        // 从请求头中获取
        if (isset($request['headers']['X-CSRF-Token'])) {
            return $request['headers']['X-CSRF-Token'];
        }
        
        // 从请求头中获取（小写）
        if (isset($request['headers']['x-csrf-token'])) {
            return $request['headers']['x-csrf-token'];
        }
        
        // 从表单数据中获取
        if (isset($request['body']['csrf_token'])) {
            return $request['body']['csrf_token'];
        }
        
        // 从URL参数中获取
        if (isset($request['query']['csrf_token'])) {
            return $request['query']['csrf_token'];
        }
        
        return null;
    }
    
    /**
     * 验证CSRF令牌
     * @param string $token CSRF令牌
     * @param array $request 请求信息
     * @return bool 验证结果
     */
    private function validateCsrfToken($token, $request) {
        // 在实际应用中，应该验证令牌是否有效
        // 这里简单实现，后续可以扩展为更复杂的验证逻辑
        
        // 检查令牌是否存在
        if (empty($token)) {
            return false;
        }
        
        // 对于API请求，我们可以使用API密钥验证代替CSRF
        // 如果已经通过了AuthMiddleware的API密钥验证，直接放行
        if (isset($request['user'])) {
            return true;
        }
        
        // 简单的令牌格式验证（至少16个字符）
        if (strlen($token) < 16) {
            return false;
        }
        
        // 这里可以添加更复杂的验证逻辑，例如：
        // 1. 验证令牌是否在服务器端生成并存储
        // 2. 验证令牌是否与用户会话关联
        // 3. 验证令牌是否过期
        
        return true;
    }
}
