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
        try {
            $this->logger = Logger::getInstance();
        } catch (\Exception $e) {
            // 如果日志服务初始化失败，使用错误日志
            error_log('Response initialization error: ' . $e->getMessage());
        }
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
     * @param array $allowedOrigins 允许的来源，默认为['*']
     */
    public static function handleCORS($allowedOrigins = ['*']) {
        try {
            // 确定允许的来源
            $origin = $_SERVER['HTTP_ORIGIN'] ?? '';
            if (in_array('*', $allowedOrigins)) {
                // 允许所有来源
                header('Access-Control-Allow-Origin: *');
            } elseif (!empty($origin) && in_array($origin, $allowedOrigins)) {
                // 允许特定来源
                header('Access-Control-Allow-Origin: ' . $origin);
            }
            
            // 允许的HTTP方法
            header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS, PATCH');
            // 允许的HTTP头
            header('Access-Control-Allow-Headers: Content-Type, X-API-Key, Authorization, X-CSRF-Token, X-Requested-With');
            // 允许携带凭证
            header('Access-Control-Allow-Credentials: true');
            // 预检请求缓存时间
            header('Access-Control-Max-Age: 86400');
            
            // 处理OPTIONS请求
            if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
                header('HTTP/1.1 204 No Content');
                exit;
            }
        } catch (\Exception $e) {
            // 记录错误但不中断流程
            error_log('CORS handling error: ' . $e->getMessage());
        }
    }

    /**
     * 过滤输出数据，防止XSS和其他安全问题
     * @param mixed $data 待过滤的数据
     * @return mixed 过滤后的数据
     */
    private function filterOutput($data) {
        try {
            if (is_string($data)) {
                // 过滤字符串，防止XSS
                return htmlspecialchars($data, ENT_QUOTES | ENT_HTML5, 'UTF-8');
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
        } catch (\Exception $e) {
            // 记录错误但不中断流程
            if (isset($this->logger)) {
                $this->logger->warning('Error filtering output', [
                    'error' => $e->getMessage()
                ]);
            }
            return $data;
        }
    }
    
    /**
     * 发送JSON响应
     * @param mixed $data 响应数据
     * @param int $statusCode HTTP状态码
     * @param array $headers 额外的响应头
     */
    public function json($data, $statusCode = 200, $headers = []) {
        try {
            // 设置内容类型
            header('Content-Type: application/json; charset=utf-8');
            
            // 设置额外的响应头
            foreach ($headers as $name => $value) {
                header($name . ': ' . $value);
            }
            
            // 设置状态码
            http_response_code($statusCode);
            
            // 过滤输出数据
            $filteredData = $this->filterOutput($data);
            
            // 生成JSON响应
            $jsonOptions = JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES | JSON_PRETTY_PRINT;
            $jsonResponse = json_encode($filteredData, $jsonOptions);
            
            // 检查JSON编码错误
            if (json_last_error() !== JSON_ERROR_NONE) {
                throw new \Exception('JSON encoding error: ' . json_last_error_msg());
            }
            
            // 发送响应
            echo $jsonResponse;
            exit;
        } catch (\Exception $e) {
            // 记录错误
            if (isset($this->logger)) {
                $this->logger->error('Error sending JSON response', [
                    'error' => $e->getMessage(),
                    'status_code' => $statusCode
                ]);
            } else {
                error_log('Error sending JSON response: ' . $e->getMessage());
            }
            
            // 发送错误响应
            http_response_code(500);
            header('Content-Type: application/json');
            echo json_encode([
                'success' => false,
                'message' => '服务器内部错误',
                'error_code' => 'JSON_ENCODING_ERROR',
                'timestamp' => time()
            ]);
            exit;
        }
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
     * @param array $headers 额外的响应头
     */
    public function success($message = '操作成功', $data = [], $headers = []) {
        $responseData = [
            'success' => true,
            'message' => $message,
            'data' => $data,
            'timestamp' => time(),
            'request_id' => $this->generateRequestId(),
            'processing_time' => $this->getProcessingTime()
        ];
        $this->json($responseData, 200, $headers);
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
     * @param array $headers 额外的响应头
     */
    public function error($message = '操作失败', $statusCode = 400, $errorCode = 'ERROR', $details = null, $headers = []) {
        $responseData = [
            'success' => false,
            'message' => $message,
            'error_code' => $errorCode,
            'timestamp' => time(),
            'request_id' => $this->generateRequestId(),
            'processing_time' => $this->getProcessingTime()
        ];
        
        // 只在开发环境返回详细错误信息
        if ($details !== null && $this->isDevelopmentEnvironment()) {
            $responseData['details'] = $this->filterOutput($details);
        }
        
        // 记录错误日志
        if (isset($this->logger)) {
            $this->logger->error($message, [
                'status_code' => $statusCode,
                'error_code' => $errorCode,
                'details' => $details,
                'request_id' => $responseData['request_id'],
                'client_ip' => $_SERVER['REMOTE_ADDR'] ?? 'unknown',
                'user_agent' => $_SERVER['HTTP_USER_AGENT'] ?? 'unknown'
            ]);
        } else {
            error_log(sprintf('Error response: %s (Code: %d, ErrorCode: %s)', $message, $statusCode, $errorCode));
        }
        
        $this->json($responseData, $statusCode, $headers);
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
     * @param array $allowedMethods 允许的方法
     */
    public function methodNotAllowed($message = '方法不允许', $errorCode = 'METHOD_NOT_ALLOWED', $allowedMethods = []) {
        $headers = [];
        if (!empty($allowedMethods)) {
            $headers['Allow'] = implode(', ', $allowedMethods);
        }
        $this->error($message, 405, $errorCode, null, $headers);
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
        try {
            if (function_exists('random_bytes')) {
                return substr(bin2hex(random_bytes(8)), 0, 16);
            } else {
                return substr(md5(uniqid(rand(), true)), 0, 16);
            }
        } catch (\Exception $e) {
            // 备用方案
            return substr(md5(uniqid(time(), true)), 0, 16);
        }
    }
    
    /**
     * 检查是否为开发环境
     * @return bool
     */
    private function isDevelopmentEnvironment() {
        $env = getenv('APP_ENV') ?: $_ENV['APP_ENV'] ?? $_SERVER['APP_ENV'] ?? 'development';
        return in_array(strtolower($env), ['development', 'dev', 'local']);
    }
    
    /**
     * 获取请求处理时间
     * @return float
     */
    private function getProcessingTime() {
        if (isset($_SERVER['REQUEST_TIME_FLOAT'])) {
            return round(microtime(true) - $_SERVER['REQUEST_TIME_FLOAT'], 4);
        }
        return 0;
    }
    
    // 静态方法别名，保持向后兼容
    public static function __callStatic($name, $arguments) {
        try {
            $instance = self::getInstance();
            $methodName = $name . 'Static';
            if (method_exists($instance, $methodName)) {
                return call_user_func_array([$instance, $methodName], $arguments);
            }
            throw new \BadMethodCallException("Method \$name does not exist");
        } catch (\Exception $e) {
            error_log('Static method call error: ' . $e->getMessage());
            throw $e;
        }
    }
    
    // 实例方法调用
    public function __call($name, $arguments) {
        try {
            if (method_exists($this, $name)) {
                return call_user_func_array([$this, $name], $arguments);
            }
            throw new \BadMethodCallException("Method \$name does not exist");
        } catch (\Exception $e) {
            if (isset($this->logger)) {
                $this->logger->warning('Method call error', [
                    'error' => $e->getMessage(),
                    'method' => $name
                ]);
            }
            throw $e;
        }
    }
}

// 兼容旧代码的全局函数
if (!function_exists('env')) {
    function env($key, $default = null) {
        return getenv($key) ?: $_ENV[$key] ?? $_SERVER[$key] ?? $default;
    }
}
