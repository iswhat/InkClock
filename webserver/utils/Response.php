<?php
/**
 * 响应工具类
 */

class Response {
    /**
     * 发送JSON响应
     */
    public static function json($data, $statusCode = 200) {
        header('Content-Type: application/json');
        http_response_code($statusCode);
        echo json_encode($data);
        exit;
    }

    /**
     * 发送成功响应
     */
    public static function success($data = array(), $message = '操作成功') {
        self::json(array(
            'success' => true,
            'message' => $message,
            'data' => $data
        ));
    }

    /**
     * 发送错误响应
     */
    public static function error($message, $code = 400, $details = array()) {
        self::json(array(
            'success' => false,
            'error' => array(
                'code' => $code,
                'message' => $message,
                'details' => $details
            )
        ), $code);
    }

    /**
     * 发送404响应
     */
    public static function notFound($message = '资源不存在') {
        self::error($message, 404);
    }

    /**
     * 发送401响应
     */
    public static function unauthorized($message = '未授权访问') {
        self::error($message, 401);
    }

    /**
     * 发送403响应
     */
    public static function forbidden($message = '无权访问该资源') {
        self::error($message, 403);
    }

    /**
     * 发送500响应
     */
    public static function serverError($message = '服务器内部错误') {
        self::error($message, 500);
    }

    /**
     * 发送分页响应
     */
    public static function paginate($data, $total, $limit, $offset) {
        self::json(array(
            'success' => true,
            'data' => $data,
            'pagination' => array(
                'total' => $total,
                'limit' => $limit,
                'offset' => $offset,
                'pages' => ceil($total / $limit),
                'current_page' => floor($offset / $limit) + 1
            )
        ));
    }

    /**
     * 处理CORS
     */
    public static function handleCORS() {
        header("Access-Control-Allow-Origin: *");
        header("Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS");
        header("Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With");
        
        if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
            http_response_code(200);
            exit;
        }
    }
}
?>