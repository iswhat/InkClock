<?php
/**
 * HTTP响应工具类
 */

namespace InkClock\Utils;

class Response {
    /**
     * 处理CORS请求
     */
    public static function handleCORS() {
        // 允许所有来源
        header('Access-Control-Allow-Origin: *');
        // 允许的HTTP方法
        header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS');
        // 允许的HTTP头
        header('Access-Control-Allow-Headers: Content-Type, X-API-Key, Authorization');
        // 允许携带凭证
        header('Access-Control-Allow-Credentials: true');
        
        // 处理OPTIONS请求
        if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
            header('HTTP/1.1 204 No Content');
            exit;
        }
    }

    /**
     * 发送JSON响应
     * @param mixed $data 响应数据
     * @param int $statusCode HTTP状态码
     */
    public static function json($data, $statusCode = 200) {
        header('Content-Type: application/json');
        http_response_code($statusCode);
        echo json_encode($data, JSON_UNESCAPED_UNICODE);
        exit;
    }

    /**
     * 发送成功响应
     * @param string $message 响应消息
     * @param mixed $data 响应数据
     */
    public static function success($message = '操作成功', $data = []) {
        self::json([
            'success' => true,
            'message' => $message,
            'data' => $data
        ]);
    }

    /**
     * 发送错误响应
     * @param string $message 错误消息
     * @param int $statusCode HTTP状态码
     */
    public static function error($message = '操作失败', $statusCode = 400) {
        self::json([
            'success' => false,
            'message' => $message
        ], $statusCode);
    }

    /**
     * 发送未授权响应
     */
    public static function unauthorized($message = '未授权访问') {
        self::json([
            'success' => false,
            'message' => $message
        ], 401);
    }

    /**
     * 发送禁止访问响应
     */
    public static function forbidden($message = '禁止访问') {
        self::json([
            'success' => false,
            'message' => $message
        ], 403);
    }

    /**
     * 发送未找到响应
     */
    public static function notFound($message = '资源未找到') {
        self::json([
            'success' => false,
            'message' => $message
        ], 404);
    }

    /**
     * 发送服务器错误响应
     */
    public static function serverError($message = '服务器内部错误') {
        self::json([
            'success' => false,
            'message' => $message
        ], 500);
    }

    /**
     * 发送方法不允许响应
     */
    public static function methodNotAllowed($message = '方法不允许') {
        self::json([
            'success' => false,
            'message' => $message
        ], 405);
    }

    /**
     * 发送请求超时响应
     */
    public static function requestTimeout($message = '请求超时') {
        self::json([
            'success' => false,
            'message' => $message
        ], 408);
    }

    /**
     * 发送冲突响应
     */
    public static function conflict($message = '资源冲突') {
        self::json([
            'success' => false,
            'message' => $message
        ], 409);
    }
}