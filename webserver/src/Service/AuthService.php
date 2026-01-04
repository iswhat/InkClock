<?php
/**
 * 认证服务类
 */

namespace App\Service;

use App\Utils\Logger;
use App\Model\User;
use App\Interface\AuthServiceInterface;

class AuthService implements AuthServiceInterface {
    private $db;
    private $logger;
    private $cache;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     * @param Logger $logger 日志服务
     * @param \App\Utils\Cache $cache 缓存服务
     */
    public function __construct($db, Logger $logger, $cache = null) {
        $this->db = $db;
        $this->logger = $logger;
        $this->cache = $cache;
    }
    
    /**
     * 用户注册
     * @param array $userInfo 用户信息
     * @return array 注册结果
     */
    public function registerUser($userInfo) {
        $this->logger->info('用户注册请求', $userInfo);
        
        // 验证用户信息
        if (!$this->validateUserInfo($userInfo)) {
            $this->logger->warning('用户注册信息验证失败', $userInfo);
            return ['success' => false, 'error' => '无效的用户信息'];
        }
        
        // 调用模型进行注册
        $userModel = new User($this->db);
        $result = $userModel->register($userInfo);
        
        if ($result['success']) {
            $this->logger->info('用户注册成功', ['username' => $userInfo['username']]);
        } else {
            $this->logger->warning('用户注册失败', ['username' => $userInfo['username'], 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 用户登录
     * @param string $username 用户名或邮箱
     * @param string $password 密码
     * @return array 登录结果
     */
    public function loginUser($username, $password) {
        $this->logger->info('用户登录请求', ['username' => $username]);
        
        // 调用模型进行登录
        $userModel = new User($this->db);
        $result = $userModel->login($username, $password);
        
        if ($result['success']) {
            $this->logger->info('用户登录成功', ['username' => $username]);
        } else {
            $this->logger->warning('用户登录失败', ['username' => $username, 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 通过API密钥验证用户
     * @param string $apiKey API密钥
     * @param string $ipAddress 请求IP地址
     * @return array 验证结果
     */
    public function validateApiKey($apiKey, $ipAddress = '') {
        $this->logger->info('API密钥验证请求', ['ip' => $ipAddress]);
        
        $userModel = new User($this->db);
        $user = $userModel->getUserByApiKey($apiKey, $ipAddress);
        
        if ($user) {
            $this->logger->info('API密钥验证成功', ['username' => $user['username'], 'ip' => $ipAddress]);
            return ['success' => true, 'user' => $user];
        } else {
            $this->logger->warning('API密钥验证失败', ['api_key' => substr($apiKey, 0, 10) . '...', 'ip' => $ipAddress]);
            return ['success' => false, 'error' => '无效的API密钥'];
        }
    }
    
    /**
     * 验证用户信息
     * @param array $userInfo 用户信息
     * @return bool 验证结果
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
     * @return bool 是否有用户
     */
    public function hasUsers() {
        $userModel = new User($this->db);
        return $userModel->hasUsers();
    }
    
    /**
     * 创建第一个管理员用户
     * @param array $adminInfo 管理员信息
     * @return array 创建结果
     */
    public function createFirstAdmin($adminInfo) {
        $this->logger->info('创建第一个管理员用户请求', $adminInfo);
        
        $userModel = new User($this->db);
        $result = $userModel->createFirstAdmin($adminInfo);
        
        if ($result['success']) {
            $this->logger->info('第一个管理员用户创建成功', ['username' => $adminInfo['username']]);
        } else {
            $this->logger->warning('第一个管理员用户创建失败', ['error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 检查用户是否为管理员
     * @param mixed $user 用户信息或用户名
     * @return bool 是否为管理员
     */
    public function isAdmin($user) {
        $userModel = new User($this->db);
        
        if (is_string($user)) {
            // 兼容旧接口，通过用户名获取用户信息
            $userData = $userModel->getUserByUsername($user);
            return $userData ? ($userData['is_admin'] === 1) : false;
        } else if (is_array($user)) {
            // 直接从用户数组中获取is_admin字段
            return isset($user['is_admin']) && $user['is_admin'] === 1;
        } else if (is_numeric($user)) {
            // 通过用户ID检查
            return $userModel->isAdmin($user);
        }
        return false;
    }
}