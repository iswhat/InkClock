<?php
/**
 * CORS中间件 - 处理跨域请求
 */

namespace App\Middleware;

use App\Utils\Response;

class CorsMiddleware implements MiddlewareInterface {
    private $response;
    
    /**
     * 构造函数
     * @param Response $response 响应服务
     */
    public function __construct(Response $response) {
        $this->response = $response;
    }
    
    /**
     * 处理请求
     * @param array $request 请求信息
     * @param callable $next 下一个中间件或处理函数
     * @return mixed 处理结果
     */
    public function handle($request, $next) {
        // 处理CORS
        $this->response->handleCORS();
        
        // 对于OPTIONS请求，直接返回成功
        if ($request['method'] === 'OPTIONS') {
            $this->response->success('CORS Preflight OK');
        }
        
        // 调用下一个中间件或处理函数
        return $next($request);
    }
}