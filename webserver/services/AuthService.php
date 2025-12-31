<?php
/**
 * 认证服务类
 */
require_once __DIR__ . '/../models/User.php';
require_once __DIR__ . '/../utils/Logger.php';

class AuthService {
    private $userModel;
    private $logger;
    
    public function __construct() {
        $this->userModel = new User();
        $this->logger = Logger::getInstance();
    }
    
    /**
     * 用户注册
     */
    public function registerUser($userInfo) {
        $this->logger->info('用户注册请求', $userInfo);
        
        // 验证用户信息
        if (!$this->validateUserInfo($userInfo)) {
            $this->logger->warning('用户注册信息验证失败', $userInfo);
            return ['success' => false, 'error' => '无效的用户信息'];
        }
        
        // 调用模型进行注册
        $result = $this->userModel->register($userInfo);
        
        if ($result['success']) {
            $this->logger->info('用户注册成功', ['username' => $userInfo['username']]);
        } else {
            $this->logger->warning('用户注册失败', ['username' => $userInfo['username'], 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 用户登录
     */
    public function loginUser($username, $password) {
        $this->logger->info('用户登录请求', ['username' => $username]);
        
        // 调用模型进行登录
        $result = $this->userModel->login($username, $password);
        
        if ($result['success']) {
            $this->logger->info('用户登录成功', ['username' => $username]);
        } else {
            $this->logger->warning('用户登录失败', ['username' => $username, 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 通过API密钥验证用户
     */
    public function validateApiKey($apiKey) {
        $this->logger->info('API密钥验证请求');
        
        $user = $this->userModel->getUserByApiKey($apiKey);
        
        if ($user) {
            $this->logger->info('API密钥验证成功', ['username' => $user['username']]);
            return ['success' => true, 'user' => $user];
        } else {
            $this->logger->warning('API密钥验证失败', ['api_key' => substr($apiKey, 0, 10) . '...']);
            return ['success' => false, 'error' => '无效的API密钥'];
        }
    }
    
    /**
     * 验证用户信息
     */
    private function validateUserInfo($userInfo) {
        // 验证用户名
        if (empty($userInfo['username']) || strlen($userInfo['username']) < 3) {
            return false;
        }
        
        // 验证邮箱
        if (empty($userInfo['email']) || !filter_var($userInfo['email'], FILTER_VALIDATE_EMAIL)) {
            return false;
        }
        
        // 验证密码
        if (empty($userInfo['password']) || strlen($userInfo['password']) < 6) {
            return false;
        }
        
        return true;
    }
    
    /**
     * 检查是否有用户存在
     */
    public function hasUsers() {
        return $this->userModel->hasUsers();
    }
    
    /**
     * 创建第一个管理员用户
     */
    public function createFirstAdmin($adminInfo) {
        $this->logger->info('创建第一个管理员用户请求', $adminInfo);
        
        $result = $this->userModel->createFirstAdmin($adminInfo);
        
        if ($result['success']) {
            $this->logger->info('第一个管理员用户创建成功', ['username' => $adminInfo['username']]);
        } else {
            $this->logger->warning('第一个管理员用户创建失败', ['error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 检查用户是否为管理员
     */
    public function isAdmin($user) {
        if (is_string($user)) {
            // 兼容旧接口，通过用户名获取用户信息
            $userData = $this->userModel->getUserByUsername($user);
            return $userData ? ($userData['is_admin'] === 1) : false;
        } else if (is_array($user)) {
            // 直接从用户数组中获取is_admin字段
            return isset($user['is_admin']) && $user['is_admin'] === 1;
        } else if (is_numeric($user)) {
            // 通过用户ID检查
            return $this->userModel->isAdmin($user);
        }
        return false;
    }
}
?>