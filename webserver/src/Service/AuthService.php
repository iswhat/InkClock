<?php
/**
 * 认证服务类
 */

namespace InkClock\Service;

use InkClock\Utils\Logger;
use InkClock\Model\User;
use InkClock\Interfaces\AuthServiceInterface;

class AuthService implements AuthServiceInterface {
    private $db;
    private $logger;
    private $cache;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     * @param Logger $logger 日志服务
     * @param \InkClock\Utils\Cache $cache 缓存服务
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
        $validationResult = $this->validateUserInfo($userInfo);
        if ($validationResult !== true) {
            $this->logger->warning('用户注册信息验证失败', [
                'error' => $validationResult,
                'userInfo' => $userInfo
            ]);
            return ['success' => false, 'error' => $validationResult];
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
     * @return bool|string 验证结果，成功返回true，失败返回错误信息
     */
    private function validateUserInfo($userInfo) {
        // 验证用户名 - 只允许字母、数字、下划线，长度3-30
        if (empty($userInfo['username'])) {
            return '用户名不能为空';
        }
        // 添加用户名格式验证，防止特殊字符注入
        $username = $userInfo['username'];
        if (!preg_match('/^[a-zA-Z0-9_]{3,30}$/', $username)) {
            return '用户名只能包含字母、数字、下划线，长度3-30个字符';
        }
        // 检查用户名是否使用保留名称
        $reservedNames = ['admin', 'root', 'system', 'test', 'guest'];
        if (in_array(strtolower($username), $reservedNames)) {
            return '用户名不能使用保留名称';
        }

        // 验证邮箱
        if (empty($userInfo['email']) || !filter_var($userInfo['email'], FILTER_VALIDATE_EMAIL)) {
            return '邮箱格式无效';
        }

        // 验证密码 - 加强强度验证
        if (empty($userInfo['password'])) {
            return '密码不能为空';
        }
        $password = $userInfo['password'];
        if (strlen($password) < 8) {
            return '密码长度至少8个字符';
        }

        // 检查密码复杂度：至少包含3种字符类型（大写、小写、数字、特殊字符）
        $hasUpper = preg_match('/[A-Z]/', $password) ? 1 : 0;
        $hasLower = preg_match('/[a-z]/', $password) ? 1 : 0;
        $hasNumber = preg_match('/[0-9]/', $password) ? 1 : 0;
        $hasSpecial = preg_match('/[^A-Za-z0-9]/', $password) ? 1 : 0;

        $complexity = $hasUpper + $hasLower + $hasNumber + $hasSpecial;
        if ($complexity < 3) {
            return '密码必须包含至少3种字符类型：大写字母、小写字母、数字、特殊字符';
        }

        // 检查常见弱密码
        $commonPasswords = ['password', '12345678', 'qwerty', 'admin', 'root'];
        if (in_array(strtolower($password), $commonPasswords)) {
            return '密码不能使用常见弱密码';
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
            // 直接从用户数组中获取is_admin字段，使用宽松比较
            return isset($user['is_admin']) && (int)$user['is_admin'] === 1;
        } else if (is_numeric($user)) {
            // 通过用户ID检查
            return $userModel->isAdmin($user);
        }
        return false;
    }
    
    /**
     * 获取所有用户列表
     * @param string $search 搜索关键词
     * @return array 用户列表
     */
    public function getAllUsers($search = '') {
        $userModel = new User($this->db);
        return $userModel->getAllUsers($search);
    }
    
    /**
     * 获取用户统计数据
     * @return array 统计数据
     */
    public function getUserStats() {
        $userModel = new User($this->db);
        return $userModel->getUserStats();
    }
    
    /**
     * 通过ID获取用户信息
     * @param int $userId 用户ID
     * @return array|null 用户信息
     */
    public function getUserById($userId) {
        $userModel = new User($this->db);
        return $userModel->getUserById($userId);
    }
    
    /**
     * 更新用户信息
     * @param int $userId 用户ID
     * @param array $userData 用户数据
     * @return array 更新结果
     */
    public function updateUser($userId, $userData) {
        $userModel = new User($this->db);
        return $userModel->updateUser($userId, $userData);
    }
    
    /**
     * 删除用户
     * @param int $userId 用户ID
     * @return array 删除结果
     */
    public function deleteUser($userId) {
        $userModel = new User($this->db);
        return $userModel->deleteUser($userId);
    }
    
    /**
     * 添加用户
     * @param array $userData 用户数据
     * @return array 添加结果
     */
    public function addUser($userData) {
        $userModel = new User($this->db);
        return $userModel->addUser($userData);
    }
}