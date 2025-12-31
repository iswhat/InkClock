<?php
/**
 * 用户模型
 */

namespace InkClock\Model;

class User {
    private $db;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     */
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 生成API密钥
     * @return string 生成的API密钥
     */
    private function generateApiKey() {
        return bin2hex(random_bytes(32));
    }
    
    /**
     * 用户注册
     * @param array $userInfo 用户信息
     * @return array 注册结果
     */
    public function register($userInfo) {
        $username = $userInfo['username'];
        $email = $userInfo['email'];
        $password = $userInfo['password'];
        
        // 检查用户名或邮箱是否已存在
        $stmt = $this->db->prepare("SELECT id FROM users WHERE username = :username OR email = :email");
        $stmt->bindValue(':username', $username, SQLITE3_TEXT);
        $stmt->bindValue(':email', $email, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        if ($result->fetchArray(SQLITE3_ASSOC)) {
            return array('success' => false, 'error' => '用户名或邮箱已存在');
        }
        
        // 哈希密码
        $passwordHash = password_hash($password, PASSWORD_DEFAULT);
        $apiKey = $this->generateApiKey();
        $createdAt = date('Y-m-d H:i:s');
        
        // 插入用户
        $stmt = $this->db->prepare("INSERT INTO users (username, email, password_hash, api_key, created_at) VALUES (:username, :email, :password_hash, :api_key, :created_at)");
        $stmt->bindValue(':username', $username, SQLITE3_TEXT);
        $stmt->bindValue(':email', $email, SQLITE3_TEXT);
        $stmt->bindValue(':password_hash', $passwordHash, SQLITE3_TEXT);
        $stmt->bindValue(':api_key', $apiKey, SQLITE3_TEXT);
        $stmt->bindValue(':created_at', $createdAt, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        
        if ($result) {
            $userId = $this->db->lastInsertRowID();
            return array('success' => true, 'user_id' => $userId, 'api_key' => $apiKey);
        } else {
            return array('success' => false, 'error' => '注册失败');
        }
    }
    
    /**
     * 用户登录
     * @param string $username 用户名或邮箱
     * @param string $password 密码
     * @return array 登录结果
     */
    public function login($username, $password) {
        // 查找用户
        $stmt = $this->db->prepare("SELECT id, password_hash, api_key, status, is_admin FROM users WHERE username = :username OR email = :email");
        $stmt->bindValue(':username', $username, SQLITE3_TEXT);
        $stmt->bindValue(':email', $username, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $user = $result->fetchArray(SQLITE3_ASSOC);
        
        if (!$user) {
            return array('success' => false, 'error' => '用户名或密码错误');
        }
        
        // 检查用户状态
        if ($user['status'] === 0) {
            return array('success' => false, 'error' => '用户账号已禁用');
        }
        
        // 验证密码
        if (!password_verify($password, $user['password_hash'])) {
            return array('success' => false, 'error' => '用户名或密码错误');
        }
        
        // 更新最后登录时间
        $lastLogin = date('Y-m-d H:i:s');
        $stmt = $this->db->prepare("UPDATE users SET last_login = :last_login WHERE id = :id");
        $stmt->bindValue(':last_login', $lastLogin, SQLITE3_TEXT);
        $stmt->bindValue(':id', $user['id'], SQLITE3_INTEGER);
        $stmt->execute();
        
        return array(
            'success' => true, 
            'user_id' => $user['id'], 
            'api_key' => $user['api_key'],
            'is_admin' => $user['is_admin']
        );
    }
    
    /**
     * 通过API密钥获取用户信息
     * @param string $apiKey API密钥
     * @return array|null 用户信息
     */
    public function getUserByApiKey($apiKey) {
        $stmt = $this->db->prepare("SELECT * FROM users WHERE api_key = :api_key AND status = 1");
        $stmt->bindValue(':api_key', $apiKey, SQLITE3_TEXT);
        $result = $stmt->execute();
        return $result->fetchArray(SQLITE3_ASSOC);
    }
    
    /**
     * 通过用户名获取用户信息
     * @param string $username 用户名
     * @return array|null 用户信息
     */
    public function getUserByUsername($username) {
        $stmt = $this->db->prepare("SELECT * FROM users WHERE username = :username");
        $stmt->bindValue(':username', $username, SQLITE3_TEXT);
        $result = $stmt->execute();
        return $result->fetchArray(SQLITE3_ASSOC);
    }
    
    /**
     * 检查用户是否为管理员
     * @param int $userId 用户ID
     * @return bool 是否为管理员
     */
    public function isAdmin($userId) {
        $stmt = $this->db->prepare("SELECT is_admin FROM users WHERE id = :id");
        $stmt->bindValue(':id', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $user = $result->fetchArray(SQLITE3_ASSOC);
        return $user ? $user['is_admin'] : false;
    }
    
    /**
     * 检查是否有用户存在
     * @return bool 是否有用户
     */
    public function hasUsers() {
        $result = $this->db->query("SELECT COUNT(*) as count FROM users");
        $row = $result->fetchArray(SQLITE3_ASSOC);
        return $row['count'] > 0;
    }
    
    /**
     * 创建第一个管理员用户
     * @param array $adminInfo 管理员信息
     * @return array 创建结果
     */
    public function createFirstAdmin($adminInfo) {
        // 检查是否已有用户
        if ($this->hasUsers()) {
            return ['success' => false, 'error' => '已有用户存在，无法创建第一个管理员'];
        }
        
        $username = $adminInfo['username'];
        $email = $adminInfo['email'];
        $password = $adminInfo['password'];
        
        // 验证用户信息
        if (empty($username) || strlen($username) < 3 || empty($email) || !filter_var($email, FILTER_VALIDATE_EMAIL) || empty($password) || strlen($password) < 6) {
            return ['success' => false, 'error' => '无效的用户信息'];
        }
        
        // 哈希密码
        $passwordHash = password_hash($password, PASSWORD_DEFAULT);
        $apiKey = $this->generateApiKey();
        $createdAt = date('Y-m-d H:i:s');
        
        // 插入管理员用户，is_admin设为1
        $stmt = $this->db->prepare("INSERT INTO users (username, email, password_hash, api_key, created_at, is_admin) VALUES (:username, :email, :password_hash, :api_key, :created_at, 1)");
        $stmt->bindValue(':username', $username, SQLITE3_TEXT);
        $stmt->bindValue(':email', $email, SQLITE3_TEXT);
        $stmt->bindValue(':password_hash', $passwordHash, SQLITE3_TEXT);
        $stmt->bindValue(':api_key', $apiKey, SQLITE3_TEXT);
        $stmt->bindValue(':created_at', $createdAt, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        
        if ($result) {
            $userId = $this->db->lastInsertRowID();
            return ['success' => true, 'user_id' => $userId, 'api_key' => $apiKey];
        } else {
            return ['success' => false, 'error' => '管理员创建失败'];
        }
    }
}