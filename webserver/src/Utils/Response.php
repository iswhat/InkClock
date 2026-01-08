<?php
/**
 * HTTP响应工具类
 */

namespace InkClock\Utils;

class Response {
    /**
     * 单例实例
     * @var Response
     */
    private static $instance;
    
    /**
     * 日志服务
     * @var Logger
     */
    private $logger;
    
    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        $this->logger = Logger::getInstance();
    }
    
    /**
     * 获取单例实例
     * @return Response
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }
    
    /**
     * 设置日志服务
     * @param Logger $logger
     */
    public function setLogger(Logger $logger) {
        $this->logger = $logger;
    }
    
    /**
     * 处理CORS请求
     */
    public static function handleCORS() {
        // 允许所有来源
        header('Access-Control-Allow-Origin: *');
        // 允许的HTTP方法
        header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS');
        // 允许的HTTP头
        header('Access-Control-Allow-Headers: Content-Type, X-API-Key, Authorization, X-CSRF-Token');
        // 允许携带凭证
        header('Access-Control-Allow-Credentials: true');
        
        // 处理OPTIONS请求
        if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
            header('HTTP/1.1 204 No Content');
            exit;
        }
    }

    /**
     * 过滤输出数据，防止XSS和其他安全问题
     * @param mixed $data 待过滤的数据
     * @return mixed 过滤后的数据
     */
    private function filterOutput($data) {
        if (is_string($data)) {
            // 过滤字符串，防止XSS
            return htmlspecialchars($data, ENT_QUOTES, 'UTF-8');
        } elseif (is_array($data)) {
            // 递归过滤数组
            foreach ($data as $key => &$value) {
                $data[$key] = $this->filterOutput($value);
            }
            return $data;
        } elseif (is_object($data)) {
            // 递归过滤对象
            foreach (get_object_vars($data) as $key => &$value) {
                $data->$key = $this->filterOutput($value);
            }
            return $data;
        }
        // 其他类型直接返回
        return $data;
    }
    
    /**
     * 发送JSON响应
     * @param mixed $data 响应数据
     * @param int $statusCode HTTP状态码
     */
    public function json($data, $statusCode = 200) {
        header('Content-Type: application/json');
        http_response_code($statusCode);
        
        // 过滤输出数据
        $filteredData = $this->filterOutput($data);
        
        echo json_encode($filteredData, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
        exit;
    }
    
    /**
     * 静态JSON响应方法（兼容旧代码）
     * @param mixed $data 响应数据
     * @param int $statusCode HTTP状态码
     */
    public static function jsonStatic($data, $statusCode = 200) {
        self::getInstance()->json($data, $statusCode);
    }

    /**
     * 发送成功响应
     * @param string $message 响应消息
     * @param mixed $data 响应数据
     */
    public function success($message = '操作成功', $data = []) {
        $responseData = [
            'success' => true,
            'message' => $message,
            'data' => $data,
            'timestamp' => time(),
            'request_id' => $this->generateRequestId()
        ];
        $this->json($responseData, 200);
    }
    
    /**
     * 静态成功响应方法（兼容旧代码）
     * @param string $message 响应消息
     * @param mixed $data 响应数据
     */
    public static function successStatic($message = '操作成功', $data = []) {
        self::getInstance()->success($message, $data);
    }

    /**
     * 发送错误响应
     * @param string $message 错误消息
     * @param int $statusCode HTTP状态码
     * @param string $errorCode 错误代码
     * @param mixed $details 错误详情
     */
    public function error($message = '操作失败', $statusCode = 400, $errorCode = 'ERROR', $details = null) {
        $responseData = [
            'success' => false,
            'message' => $message,
            'error_code' => $errorCode,
            'timestamp' => time(),
            'request_id' => $this->generateRequestId()
        ];
        
        // 只在开发环境返回详细错误信息
        if ($details !== null && env('APP_ENV', 'development') === 'development') {
            $responseData['details'] = $details;
        }
        
        // 记录错误日志
        $this->logger->error($message, [
            'status_code' => $statusCode,
            'error_code' => $errorCode,
            'details' => $details,
            'request_id' => $responseData['request_id']
        ]);
        
        $this->json($responseData, $statusCode);
    }
    
    /**
     * 静态错误响应方法（兼容旧代码）
     * @param string $message 错误消息
     * @param int $statusCode HTTP状态码
     */
    public static function errorStatic($message = '操作失败', $statusCode = 400) {
        self::getInstance()->error($message, $statusCode);
    }

    /**
     * 发送未授权响应
     * @param string $message 响应消息
     * @param string $errorCode 错误代码
     */
    public function unauthorized($message = '未授权访问', $errorCode = 'UNAUTHORIZED') {
        $this->error($message, 401, $errorCode);
    }
    
    /**
     * 静态未授权响应方法（兼容旧代码）
     * @param string $message 响应消息
     */
    public static function unauthorizedStatic($message = '未授权访问') {
        self::getInstance()->unauthorized($message);
    }

    /**
     * 发送禁止访问响应
     * @param string $message 响应消息
     * @param string $errorCode 错误代码
     */
    public function forbidden($message = '禁止访问', $errorCode = 'FORBIDDEN') {
        $this->error($message, 403, $errorCode);
    }
    
    /**
     * 静态禁止访问响应方法（兼容旧代码）
     * @param string $message 响应消息
     */
    public static function forbiddenStatic($message = '禁止访问') {
        self::getInstance()->forbidden($message);
    }

    /**
     * 发送未找到响应
     * @param string $message 响应消息
     * @param string $errorCode 错误代码
     */
    public function notFound($message = '资源未找到', $errorCode = 'NOT_FOUND') {
        $this->error($message, 404, $errorCode);
    }
    
    /**
     * 静态未找到响应方法（兼容旧代码）
     * @param string $message 响应消息
     */
    public static function notFoundStatic($message = '资源未找到') {
        self::getInstance()->notFound($message);
    }

    /**
     * 发送服务器错误响应
     * @param string $message 响应消息
     * @param string $errorCode 错误代码
     * @param mixed $details 错误详情
     */
    public function serverError($message = '服务器内部错误', $errorCode = 'SERVER_ERROR', $details = null) {
        $this->error($message, 500, $errorCode, $details);
    }
    
    /**
     * 静态服务器错误响应方法（兼容旧代码）
     * @param string $message 响应消息
     */
    public static function serverErrorStatic($message = '服务器内部错误') {
        self::getInstance()->serverError($message);
    }

    /**
     * 发送方法不允许响应
     * @param string $message 响应消息
     * @param string $errorCode 错误代码
     */
    public function methodNotAllowed($message = '方法不允许', $errorCode = 'METHOD_NOT_ALLOWED') {
        $this->error($message, 405, $errorCode);
    }
    
    /**
     * 静态方法不允许响应方法（兼容旧代码）
     * @param string $message 响应消息
     */
    public static function methodNotAllowedStatic($message = '方法不允许') {
        self::getInstance()->methodNotAllowed($message);
    }

    /**
     * 发送请求超时响应
     * @param string $message 响应消息
     * @param string $errorCode 错误代码
     */
    public function requestTimeout($message = '请求超时', $errorCode = 'REQUEST_TIMEOUT') {
        $this->error($message, 408, $errorCode);
    }
    
    /**
     * 静态请求超时响应方法（兼容旧代码）
     * @param string $message 响应消息
     */
    public static function requestTimeoutStatic($message = '请求超时') {
        self::getInstance()->requestTimeout($message);
    }

    /**
     * 发送冲突响应
     * @param string $message 响应消息
     * @param string $errorCode 错误代码
     */
    public function conflict($message = '资源冲突', $errorCode = 'CONFLICT') {
        $this->error($message, 409, $errorCode);
    }
    
    /**
     * 静态冲突响应方法（兼容旧代码）
     * @param string $message 响应消息
     */
    public static function conflictStatic($message = '资源冲突') {
        self::getInstance()->conflict($message);
    }
    
    /**
     * 生成请求ID
     * @return string
     */
    private function generateRequestId() {
        return substr(md5(uniqid(rand(), true)), 0, 16);
    }
    
    // 静态方法别名，保持向后兼容
    public static function __callStatic($name, $arguments) {
        $instance = self::getInstance();
        $methodName = $name . 'Static';
        if (method_exists($instance, $methodName)) {
            return call_user_func_array([$instance, $methodName], $arguments);
        }
        throw new \BadMethodCallException("Method \$name does not exist");
    }
    
    // 实例方法调用
    public function __call($name, $arguments) {
        if (method_exists($this, $name)) {
            return call_user_func_array([$this, $name], $arguments);
        }
        throw new \BadMethodCallException("Method \$name does not exist");
    }
}