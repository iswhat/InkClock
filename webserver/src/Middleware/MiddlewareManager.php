<?php
/**
 * 中间件管理器 - 管理和执行中间件链
 */

namespace InkClock\Middleware;

use InkClock\Utils\DIContainer;

interface MiddlewareInterface {
    public function handle($request, $next);
}

class MiddlewareManager {
    private $middlewares = [];
    private $namedMiddlewares = [];
    private $middlewareGroups = [];
    private $container;
    
    /**
     * 构造函数
     * @param DIContainer|null $container 依赖注入容器
     */
    public function __construct(?DIContainer $container = null) {
        $this->container = $container;
    }
    
    /**
     * 添加中间件到链中
     * @param string|MiddlewareInterface $middleware 中间件类名或实例
     * @param string|null $name 中间件名称（可选）
     * @return $this
     */
    public function add($middleware, ?string $name = null) {
        $this->middlewares[] = $middleware;
        
        if ($name) {
            $this->namedMiddlewares[$name] = $middleware;
        }
        
        return $this;
    }
    
    /**
     * 注册命名中间件
     * @param string $name 中间件名称
     * @param string|MiddlewareInterface $middleware 中间件类名或实例
     * @return $this
     */
    public function register(string $name, $middleware) {
        $this->namedMiddlewares[$name] = $middleware;
        return $this;
    }
    
    /**
     * 定义中间件组
     * @param string $name 中间件组名称
     * @param array $middlewares 中间件列表
     * @return $this
     */
    public function group(string $name, array $middlewares) {
        $this->middlewareGroups[$name] = $middlewares;
        return $this;
    }
    
    /**
     * 使用中间件组或命名中间件
     * @param string|array $middlewares 中间件组名称、命名中间件或中间件列表
     * @return $this
     */
    public function use($middlewares) {
        if (is_string($middlewares)) {
            // 检查是否是中间件组
            if (isset($this->middlewareGroups[$middlewares])) {
                foreach ($this->middlewareGroups[$middlewares] as $middleware) {
                    $this->add($middleware);
                }
            } 
            // 检查是否是命名中间件
            elseif (isset($this->namedMiddlewares[$middlewares])) {
                $this->add($this->namedMiddlewares[$middlewares]);
            } 
            // 否则直接添加为中间件类名
            else {
                $this->add($middlewares);
            }
        } elseif (is_array($middlewares)) {
            foreach ($middlewares as $middleware) {
                $this->use($middleware);
            }
        }
        
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
            $currentMiddleware = $middleware;
            $chain = function($request) use ($currentMiddleware, $chain) {
                // 如果是中间件类名，使用依赖注入容器创建实例
                if (is_string($currentMiddleware)) {
                    if ($this->container) {
                        $middlewareInstance = $this->container->createInstance($currentMiddleware);
                    } else {
                        // 如果没有容器，尝试直接实例化
                        $middlewareInstance = new $currentMiddleware();
                    }
                } else {
                    $middlewareInstance = $currentMiddleware;
                }
                
                return $middlewareInstance->handle($request, $chain);
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
    
    /**
     * 获取命名中间件列表
     * @return array 命名中间件列表
     */
    public function getNamedMiddlewares() {
        return $this->namedMiddlewares;
    }
    
    /**
     * 获取中间件组列表
     * @return array 中间件组列表
     */
    public function getMiddlewareGroups() {
        return $this->middlewareGroups;
    }
}