<?php
/**
 * 中间件管理器 - 管理和执行中间件链
 */

namespace App\Middleware;

class MiddlewareManager {
    private $middlewares = [];
    
    /**
     * 添加中间件到链中
     * @param MiddlewareInterface $middleware 中间件实例
     * @return $this
     */
    public function add(MiddlewareInterface $middleware) {
        $this->middlewares[] = $middleware;
        return $this;
    }
    
    /**
     * 执行中间件链
     * @param array $request 请求信息
     * @param callable $handler 最终处理函数
     * @return mixed 处理结果
     */
    public function handle($request, $handler) {
        // 创建中间件链
        $chain = $this->createMiddlewareChain($handler);
        
        // 执行中间件链
        return $chain($request);
    }
    
    /**
     * 创建中间件链
     * @param callable $handler 最终处理函数
     * @return callable 中间件链
     */
    private function createMiddlewareChain($handler) {
        // 从最后一个中间件开始构建链
        $chain = $handler;
        
        // 反转中间件数组，从最后一个到第一个执行
        foreach (array_reverse($this->middlewares) as $middleware) {
            $chain = function($request) use ($middleware, $chain) {
                return $middleware->handle($request, $chain);
            };
        }
        
        return $chain;
    }
    
    /**
     * 清除所有中间件
     * @return $this
     */
    public function clear() {
        $this->middlewares = [];
        return $this;
    }
    
    /**
     * 获取中间件数量
     * @return int 中间件数量
     */
    public function count() {
        return count($this->middlewares);
    }
}