<?php
/**
 * 基础控制器类
 */

class BaseController {
    protected $db;
    protected $logger;
    protected $response;
    protected $currentUser;
    
    /**
     * 构造函数
     */
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
        $this->logger = Logger::getInstance();
        $this->response = Response::class;
        $this->currentUser = $this->getCurrentUser();
    }
    
    /**
     * 获取当前用户
     */
    protected function getCurrentUser() {
        // 从请求中获取API密钥
        $apiKey = isset($_GET['api_key']) ? $_GET['api_key'] : (isset($_SERVER['HTTP_X_API_KEY']) ? $_SERVER['HTTP_X_API_KEY'] : '');
        
        if (!$apiKey) {
            return null;
        }
        
        // 验证API密钥并获取用户信息
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        return $userModel->getUserByApiKey($apiKey);
    }
    
    /**
     * 检查API权限
     */
    protected function checkApiPermission($required = false) {
        if ($required && !$this->currentUser) {
            $this->response::unauthorized();
        }
        
        return $this->currentUser;
    }
    
    /**
     * 解析请求数据
     */
    protected function parseRequestBody() {
        $data = json_decode(file_get_contents('php://input'), true);
        if (json_last_error() !== JSON_ERROR_NONE) {
            $this->response::error('无效的JSON数据', 400);
        }
        return $data;
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
        $data['user_id'] = $this->currentUser ? $this->currentUser['id'] : null;
        $data['action'] = $action;
        $this->logger->info('API操作', $data);
    }
    
    /**
     * 检查设备所有权
     */
    protected function validateDeviceOwnership($deviceId) {
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        
        if (!$this->currentUser) {
            return false;
        }
        
        return $userModel->isDeviceOwnedByUser($this->currentUser['id'], $deviceId);
    }
    
    /**
     * 检查设备访问权限
     */
    protected function checkDevicePermission($deviceId) {
        require_once __DIR__ . '/../models/Device.php';
        $deviceModel = new Device();
        
        // 设备可以自己访问自己的资源，不需要验证用户
        $ip = $_SERVER['REMOTE_ADDR'];
        $device = $deviceModel->getDevice($deviceId);
        if ($device && $device['ip_address'] == $ip) {
            return true;
        }
        
        // 其他情况需要验证用户权限
        if (!$this->currentUser) {
            $this->response::unauthorized();
        }
        
        if (!$this->validateDeviceOwnership($deviceId)) {
            $this->response::forbidden();
        }
        
        return true;
    }
}
?>