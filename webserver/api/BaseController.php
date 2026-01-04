<?php
/**
 * 基础控制器类
 */

namespace InkClock\Api;

use InkClock\Service\AuthService;
use InkClock\Service\DeviceService;
use InkClock\Service\MessageService;
use InkClock\Model\Device as DeviceModel;

class BaseController {
    protected $container;
    protected $db;
    protected $logger;
    protected $response;
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
        $this->container = $container;
        
        // 从容器中获取依赖
        if ($container) {
            $this->db = $container->get('db');
            $this->logger = $container->get('logger');
            $this->response = $container->get('response');
        } else {
            // 直接使用依赖注入容器
            $this->container = \InkClock\Utils\DIContainer::getInstance();
            $this->db = $this->container->get('db');
            $this->logger = $this->container->get('logger');
            $this->response = $this->container->get('response');
        }
        
        // 初始化服务层
        $this->authService = new AuthService($this->db, $this->logger);
        $this->deviceService = new DeviceService($this->db, $this->logger);
        $this->messageService = new MessageService($this->db, $this->logger);
        
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
        
        // 获取客户端IP地址
        $ipAddress = $_SERVER['REMOTE_ADDR'];
        
        // 使用服务层验证API密钥
        $result = $this->authService->validateApiKey($apiKey, $ipAddress);
        return $result['success'] ? $result['user'] : null;
    }
    
    /**
     * 检查API权限
     */
    protected function checkApiPermission($required = false) {
        if ($required && !$this->currentUser) {
            $this->response->unauthorized();
        }
        
        return $this->currentUser;
    }
    
    /**
     * 解析请求数据
     */
    protected function parseRequestBody() {
        $data = json_decode(file_get_contents('php://input'), true);
        if (json_last_error() !== JSON_ERROR_NONE) {
            $this->response->error('无效的JSON数据', 400);
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
    }
    
    /**
     * 检查设备访问权限
     */
    protected function checkDevicePermission($deviceId) {
        // 使用重构后的设备模型
        $deviceModel = new DeviceModel($this->db);
        
        // 设备可以自己访问自己的资源，不需要验证用户
        $ip = $_SERVER['REMOTE_ADDR'];
        $device = $deviceModel->getDevice($deviceId);
        if ($device && $device['ip_address'] == $ip) {
            return true;
        }
        
        // 其他情况需要验证用户权限
        if (!$this->currentUser) {
            $this->response->unauthorized();
        }
        
        if (!$this->validateDeviceOwnership($deviceId)) {
            $this->response->forbidden();
        }
        
        return true;
    }
    
    /**
     * 检查是否为管理员
     */
    protected function isAdmin() {
        return $this->authService->isAdmin($this->currentUser);
    }
}
?>