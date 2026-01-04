<?php
/**
 * 认证中间件 - 处理API密钥验证
 */

namespace App\Middleware;

use App\Service\AuthService;
use App\Utils\Logger;
use App\Utils\Response;

class AuthMiddleware implements MiddlewareInterface {
    private $authService;
    private $logger;
    private $response;
    private $required;
    
    /**
     * 构造函数
     * @param AuthService $authService 认证服务
     * @param Logger $logger 日志服务
     * @param Response $response 响应服务
     * @param bool $required 是否必须认证
     */
    public function __construct(AuthService $authService, Logger $logger, Response $response, $required = false) {
        $this->authService = $authService;
        $this->logger = $logger;
        $this->response = $response;
        $this->required = $required;
    }
    
    /**
     * 处理请求
     * @param array $request 请求信息
     * @param callable $next 下一个中间件或处理函数
     * @return mixed 处理结果
     */
    public function handle($request, $next) {
        $apiKey = isset($request['headers']['X-API-Key']) ? $request['headers']['X-API-Key'] : (isset($request['query']['api_key']) ? $request['query']['api_key'] : '');
        $ipAddress = $request['ip'];
        
        $result = $this->authService->validateApiKey($apiKey, $ipAddress);
        
        if (!$result['success']) {
            if ($this->required) {
                $this->logger->warning('API密钥验证失败', ['api_key' => $apiKey, 'ip' => $ipAddress, 'path' => $request['path']]);
                $this->response->unauthorized();
            }
        } else {
            // 将当前用户添加到请求中
            $request['current_user'] = $result['user'];
        }
        
        return $next($request);
    }
}