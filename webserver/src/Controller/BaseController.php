<?php
/**
 * 基础控制器类
 */

namespace InkClock\Controller;



class BaseController {
    protected $container;
    protected $db;
    protected $logger;
    protected $response;
    protected $cache;
    protected $currentUser;
    
    // 服务层实例
    protected $authService;
    protected $deviceService;
    protected $messageService;
    
    /**
     * 构造函数 - 使用依赖注入
     * @param \InkClock\Utils\DIContainer $container 依赖注入容器
     */
    public function __construct($container = null) {
        try {
            $this->container = $container;
            
            // 从容器中获取依赖
            if ($container) {
                $this->db = $container->get('db');
                $this->logger = $container->get('logger');
                $this->response = $container->get('response');
                $this->cache = $container->get('cache');
            } else {
                // 直接使用依赖注入容器
                $this->container = \InkClock\Utils\DIContainer::getInstance();
                $this->db = $this->container->get('db');
                $this->logger = $this->container->get('logger');
                $this->response = $this->container->get('response');
                $this->cache = $this->container->get('cache');
            }
            
            // 从容器中获取服务实例
            $this->authService = $this->container->get('authService');
            $this->deviceService = $this->container->get('deviceService');
            $this->messageService = $this->container->get('messageService');
            
            // 在所有服务初始化后再获取当前用户
            $this->currentUser = $this->getCurrentUser();
        } catch (\Exception $e) {
            // 记录初始化错误
            error_log('BaseController initialization error: ' . $e->getMessage());
            throw $e;
        }
    }
    
    /**
     * 获取当前用户
     */
    protected function getCurrentUser() {
        try {
            // 从请求中获取API密钥
            $apiKey = isset($_GET['api_key']) ? $_GET['api_key'] : (isset($_SERVER['HTTP_X_API_KEY']) ? $_SERVER['HTTP_X_API_KEY'] : '');
            
            if (!$apiKey) {
                return null;
            }
            
            // 获取客户端IP地址
            $ipAddress = $this->getClientIpAddress();
            
            // 使用服务层验证API密钥
            $result = $this->authService->validateApiKey($apiKey, $ipAddress);
            return $result['success'] ? $result['user'] : null;
        } catch (\Exception $e) {
            // 记录错误但不中断流程
            if (isset($this->logger)) {
                $this->logger->warning('Error getting current user', [
                    'error' => $e->getMessage()
                ]);
            }
            return null;
        }
    }
    
    /**
     * 获取客户端IP地址
     */
    protected function getClientIpAddress() {
        $ipKeys = ['HTTP_CF_CONNECTING_IP', 'HTTP_X_FORWARDED_FOR', 'HTTP_X_FORWARDED', 'HTTP_X_CLUSTER_CLIENT_IP', 'HTTP_CLIENT_IP', 'REMOTE_ADDR'];
        
        foreach ($ipKeys as $key) {
            if (isset($_SERVER[$key])) {
                $ip = $_SERVER[$key];
                // 处理多个IP地址的情况
                if (strpos($ip, ',') !== false) {
                    $ip = trim(explode(',', $ip)[0]);
                }
                // 验证IP地址格式
                if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE | FILTER_FLAG_NO_RES_RANGE)) {
                    return $ip;
                }
            }
        }
        
        return $_SERVER['REMOTE_ADDR'] ?? '0.0.0.0';
    }
    
    /**
     * 检查是否为管理员
     */
    protected function isAdmin() {
        try {
            if (!$this->currentUser) {
                return false;
            }
            
            // 使用服务层检查管理员权限
            return $this->authService->isAdmin($this->currentUser);
        } catch (\Exception $e) {
            if (isset($this->logger)) {
                $this->logger->warning('Error checking admin status', [
                    'error' => $e->getMessage()
                ]);
            }
            return false;
        }
    }
    
    /**
     * 检查API权限
     */
    protected function checkApiPermission($required = false) {
        try {
            if ($required && !$this->currentUser) {
                $errorDetails = [
                    'has_api_key' => isset($_GET['api_key']) || isset($_SERVER['HTTP_X_API_KEY']),
                    'api_key_provided' => isset($_GET['api_key']) ? 'GET参数' : (isset($_SERVER['HTTP_X_API_KEY']) ? 'X_API_KEY头' : '未提供'),
                    'client_ip' => $this->getClientIpAddress()
                ];
                
                if (isset($this->response)) {
                    $this->response->unauthorized('未授权访问', 'UNAUTHORIZED', $errorDetails);
                } else {
                    http_response_code(401);
                    echo json_encode([
                        'success' => false, 
                        'message' => '未授权访问',
                        'error_code' => 'UNAUTHORIZED',
                        'error_details' => $errorDetails
                    ]);
                    exit;
                }
            }
            
            return $this->currentUser;
        } catch (\Exception $e) {
            $errorDetails = [
                'exception' => $e->getMessage(),
                'exception_type' => get_class($e),
                'client_ip' => $this->getClientIpAddress()
            ];
            
            if (isset($this->logger)) {
                $this->logger->warning('Error checking API permission', $errorDetails);
            }
            if ($required) {
                http_response_code(401);
                echo json_encode([
                    'success' => false, 
                    'message' => '权限检查失败',
                    'error_code' => 'PERMISSION_CHECK_FAILED',
                    'error_details' => $errorDetails
                ]);
                exit;
            }
            return null;
        }
    }
    
    /**
     * 解析请求数据
     */
    protected function parseRequestBody() {
        try {
            $body = file_get_contents('php://input');
            if (empty($body)) {
                return [];
            }
            
            $data = json_decode($body, true);
            if (json_last_error() !== JSON_ERROR_NONE) {
                $jsonError = json_last_error();
                $errorMessages = [
                    JSON_ERROR_DEPTH => 'JSON数据嵌套过深',
                    JSON_ERROR_STATE_MISMATCH => 'JSON数据状态不匹配',
                    JSON_ERROR_CTRL_CHAR => 'JSON数据包含非法控制字符',
                    JSON_ERROR_SYNTAX => 'JSON数据语法错误',
                    JSON_ERROR_UTF8 => 'JSON数据包含无效UTF-8字符',
                    JSON_ERROR_RECURSION => 'JSON数据包含循环引用',
                    JSON_ERROR_INF_OR_NAN => 'JSON数据包含无穷大或NaN值',
                    JSON_ERROR_UNSUPPORTED_TYPE => 'JSON数据包含不支持的数据类型'
                ];
                $errorMessage = $errorMessages[$jsonError] ?? '无效的JSON数据';
                
                if (isset($this->response)) {
                    $this->response->error($errorMessage, 400, 'INVALID_JSON', [
                        'error_code' => $jsonError,
                        'error_message' => json_last_error_msg(),
                        'raw_body' => substr($body, 0, 100) // 只返回前100个字符，避免泄露敏感信息
                    ]);
                } else {
                    http_response_code(400);
                    echo json_encode([
                        'success' => false, 
                        'message' => $errorMessage,
                        'error_code' => 'INVALID_JSON',
                        'error_details' => [
                            'error_code' => $jsonError,
                            'error_message' => json_last_error_msg()
                        ]
                    ]);
                    exit;
                }
            }
            return $data;
        } catch (\Exception $e) {
            if (isset($this->logger)) {
                $this->logger->warning('Error parsing request body', [
                    'error' => $e->getMessage(),
                    'exception_type' => get_class($e)
                ]);
            }
            if (isset($this->response)) {
                $this->response->error('解析请求数据失败', 400, 'REQUEST_PARSE_ERROR', [
                    'exception' => $e->getMessage(),
                    'exception_type' => get_class($e)
                ]);
            } else {
                http_response_code(400);
                echo json_encode([
                    'success' => false, 
                    'message' => '解析请求数据失败',
                    'error_code' => 'REQUEST_PARSE_ERROR',
                    'error_details' => [
                        'exception' => $e->getMessage()
                    ]
                ]);
                exit;
            }
        }
    }
    
    /**
     * 获取查询参数
     */
    protected function getQueryParams($params = array()) {
        $result = array();
        
        foreach ($params as $param) {
            $result[$param] = $_GET[$param] ?? null;
        }
        
        return $result;
    }
    
    /**
     * 记录操作日志
     */
    protected function logAction($action, $data = array()) {
        try {
            $data['user_id'] = $this->currentUser ? $this->currentUser['id'] : null;
            $data['action'] = $action;
            $data['client_ip'] = $this->getClientIpAddress();
            $data['user_agent'] = $_SERVER['HTTP_USER_AGENT'] ?? '';
            
            if (isset($this->logger)) {
                $this->logger->info('API操作', $data);
            }
        } catch (\Exception $e) {
            // 记录日志错误但不中断流程
            error_log('Log action error: ' . $e->getMessage());
        }
    }
    
    /**
     * 检查设备所有权
     */
    protected function validateDeviceOwnership($deviceId) {
        try {
            if (!$this->currentUser) {
                return false;
            }
            
            // 使用服务层检查设备所有权
            $deviceList = $this->deviceService->getDeviceList($this->currentUser['id']);
            if ($deviceList['success']) {
                foreach ($deviceList['devices'] as $device) {
                    if ($device['device_id'] == $deviceId) {
                        return true;
                    }
                }
            }
            return false;
        } catch (\Exception $e) {
            if (isset($this->logger)) {
                $this->logger->warning('Error validating device ownership', [
                    'error' => $e->getMessage(),
                    'device_id' => $deviceId
                ]);
            }
            return false;
        }
    }
    
    /**
     * 检查设备访问权限
     */
    protected function checkDevicePermission($deviceId) {
        try {
            // 使用设备模型
            $deviceModel = new \InkClock\Model\Device($this->db);
            
            // 设备可以自己访问自己的资源，不需要验证用户
            $ip = $this->getClientIpAddress();
            $device = $deviceModel->getDevice($deviceId);
            if ($device && $device['ip_address'] == $ip) {
                return true;
            }
            
            // 其他情况需要验证用户权限
            if (!$this->currentUser) {
                if (isset($this->response)) {
                    $this->response->unauthorized();
                } else {
                    http_response_code(401);
                    echo json_encode(['success' => false, 'message' => 'Unauthorized']);
                    exit;
                }
            }
            
            if (!$this->validateDeviceOwnership($deviceId)) {
                if (isset($this->response)) {
                    $this->response->forbidden();
                } else {
                    http_response_code(403);
                    echo json_encode(['success' => false, 'message' => 'Forbidden']);
                    exit;
                }
            }
            
            return true;
        } catch (\Exception $e) {
            if (isset($this->logger)) {
                $this->logger->warning('Error checking device permission', [
                    'error' => $e->getMessage(),
                    'device_id' => $deviceId
                ]);
            }
            if (isset($this->response)) {
                $this->response->error('检查设备权限失败', 500);
            } else {
                http_response_code(500);
                echo json_encode(['success' => false, 'message' => 'Failed to check device permission']);
                exit;
            }
        }
    }
    
    /**
     * 安全地获取环境变量
     */
    protected function getEnv($key, $default = null) {
        return $_ENV[$key] ?? $_SERVER[$key] ?? getenv($key) ?? $default;
    }
    
    /**
     * 生成安全的随机字符串
     */
    protected function generateRandomString($length = 32) {
        $bytes = random_bytes($length);
        return bin2hex($bytes);
    }
    
}
?>