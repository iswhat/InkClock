<?php
/**
 * 中间件接口定义
 */

namespace InkClock\Middleware;

interface MiddlewareInterface {
    /**
     * 处理请求
     * @param array $request 请求信息
     * @param callable $next 下一个中间件或处理函数
     * @return mixed 处理结果
     */
    public function handle($request, $next);
}