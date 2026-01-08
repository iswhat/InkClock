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
        // 检查数据库连接是否有效
        if (!method_exists($this->db, 'prepare')) {
            return array('success' => false, 'error' => '数据库连接失败，请联系管理员');
        }
        
        $username = $userInfo['username'];
        $email = $userInfo['email'];
        $password = $userInfo['password'];
        
        try {
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
            
            // 设置API密钥过期时间（默认365天）
            $apiKeyExpiresAt = date('Y-m-d H:i:s', strtotime('+365 days'));
            
            // 插入用户
            $stmt = $this->db->prepare("INSERT INTO users (username, email, password_hash, api_key, api_key_created_at, api_key_expires_at, created_at) VALUES (:username, :email, :password_hash, :api_key, :apiKeyCreatedAt, :apiKeyExpiresAt, :createdAt)");
            $stmt->bindValue(':username', $username, SQLITE3_TEXT);
            $stmt->bindValue(':email', $email, SQLITE3_TEXT);
            $stmt->bindValue(':password_hash', $passwordHash, SQLITE3_TEXT);
            $stmt->bindValue(':api_key', $apiKey, SQLITE3_TEXT);
            $stmt->bindValue(':apiKeyCreatedAt', $createdAt, SQLITE3_TEXT);
            $stmt->bindValue(':apiKeyExpiresAt', $apiKeyExpiresAt, SQLITE3_TEXT);
            $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
            
            $result = $stmt->execute();
            
            if ($result) {
                $userId = $this->db->lastInsertRowID();
                return array('success' => true, 'user_id' => $userId, 'api_key' => $apiKey);
            } else {
                return array('success' => false, 'error' => '注册失败');
            }
        } catch (\Exception $e) {
            return array('success' => false, 'error' => '数据库操作失败: ' . $e->getMessage());
        }
    }
    
    /**
     * 用户登录
     * @param string $username 用户名或邮箱
     * @param string $password 密码
     * @return array 登录结果
     */
    public function login($username, $password) {
        // 检查数据库连接是否有效
        if (!method_exists($this->db, 'prepare')) {
            return array('success' => false, 'error' => '数据库连接失败，请联系管理员');
        }
        
        try {
            // 查找用户
            $stmt = $this->db->prepare("SELECT id, password_hash, api_key, api_key_expires_at, status, is_admin FROM users WHERE username = :username OR email = :email");
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
            
            // 检查API密钥是否即将过期（30天内），如果是则生成新密钥
            $now = time();
            $expiresAt = strtotime($user['api_key_expires_at']);
            $daysUntilExpiry = ($expiresAt - $now) / (60 * 60 * 24);
            
            $apiKey = $user['api_key'];
            if ($daysUntilExpiry < 30) {
                // 生成新的API密钥
                $apiKey = $this->generateApiKey();
                $apiKeyCreatedAt = date('Y-m-d H:i:s');
                $apiKeyExpiresAt = date('Y-m-d H:i:s', strtotime('+365 days'));
                
                // 更新API密钥信息
                $stmt = $this->db->prepare("UPDATE users SET api_key = :apiKey, api_key_created_at = :apiKeyCreatedAt, api_key_expires_at = :apiKeyExpiresAt WHERE id = :id");
                $stmt->bindValue(':apiKey', $apiKey, SQLITE3_TEXT);
                $stmt->bindValue(':apiKeyCreatedAt', $apiKeyCreatedAt, SQLITE3_TEXT);
                $stmt->bindValue(':apiKeyExpiresAt', $apiKeyExpiresAt, SQLITE3_TEXT);
                $stmt->bindValue(':id', $user['id'], SQLITE3_INTEGER);
                $stmt->execute();
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
                'api_key' => $apiKey,
                'is_admin' => $user['is_admin']
            );
        } catch (\Exception $e) {
            return array('success' => false, 'error' => '数据库操作失败: ' . $e->getMessage());
        }
    }
    
    /**
     * 通过API密钥获取用户信息，带安全性检查
     * @param string $apiKey API密钥
     * @param string $ipAddress 请求IP地址
     * @return array|null 用户信息
     */
    public function getUserByApiKey($apiKey, $ipAddress = '') {
        $stmt = $this->db->prepare("SELECT * FROM users WHERE api_key = :api_key AND status = 1");
        $stmt->bindValue(':api_key', $apiKey, SQLITE3_TEXT);
        $result = $stmt->execute();
        $user = $result->fetchArray(SQLITE3_ASSOC);
        
        if (!$user) {
            return null;
        }
        
        // 检查API密钥是否过期
        $now = date('Y-m-d H:i:s');
        if ($user['api_key_expires_at'] && $user['api_key_expires_at'] < $now) {
            return null;
        }
        
        // 检查IP白名单
        if ($user['api_key_ip_whitelist']) {
            $whitelist = explode(',', $user['api_key_ip_whitelist']);
            $whitelist = array_map('trim', $whitelist);
            if (!in_array($ipAddress, $whitelist)) {
                return null;
            }
        }
        
        return $user;
    }
    
    /**
     * 轮换API密钥
     * @param int $userId 用户ID
     * @return array 轮换结果
     */
    public function rotateApiKey($userId) {
        $newApiKey = $this->generateApiKey();
        $now = date('Y-m-d H:i:s');
        $expiresAt = date('Y-m-d H:i:s', strtotime('+365 days'));
        
        $stmt = $this->db->prepare("UPDATE users SET api_key = :apiKey, api_key_created_at = :createdAt, api_key_expires_at = :expiresAt WHERE id = :userId");
        $stmt->bindValue(':apiKey', $newApiKey, SQLITE3_TEXT);
        $stmt->bindValue(':createdAt', $now, SQLITE3_TEXT);
        $stmt->bindValue(':expiresAt', $expiresAt, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        
        $result = $stmt->execute();
        
        if ($result) {
            return array('success' => true, 'api_key' => $newApiKey);
        } else {
            return array('success' => false, 'error' => 'API密钥轮换失败');
        }
    }
    
    /**
     * 更新API密钥过期时间
     * @param int $userId 用户ID
     * @param string $expiresAt 过期时间
     * @return array 更新结果
     */
    public function updateApiKeyExpiration($userId, $expiresAt) {
        $stmt = $this->db->prepare("UPDATE users SET api_key_expires_at = :expiresAt WHERE id = :userId");
        $stmt->bindValue(':expiresAt', $expiresAt, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        
        $result = $stmt->execute();
        
        return array('success' => $result !== false);
    }
    
    /**
     * 更新API密钥IP白名单
     * @param int $userId 用户ID
     * @param array $ipList IP地址列表
     * @return array 更新结果
     */
    public function updateApiKeyIpWhitelist($userId, $ipList) {
        $whitelist = implode(',', $ipList);
        
        $stmt = $this->db->prepare("UPDATE users SET api_key_ip_whitelist = :whitelist WHERE id = :userId");
        $stmt->bindValue(':whitelist', $whitelist, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        
        $result = $stmt->execute();
        
        return array('success' => $result !== false);
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
        // 检查数据库连接是否有效
        if (!method_exists($this->db, 'query')) {
            return false;
        }
        
        try {
            $result = $this->db->query("SELECT COUNT(*) as count FROM users");
            $row = $result->fetchArray(SQLITE3_ASSOC);
            return $row['count'] > 0;
        } catch (\Exception $e) {
            return false;
        }
    }
    
    /**
     * 创建第一个管理员用户
     * @param array $adminInfo 管理员信息
     * @return array 创建结果
     */
    public function createFirstAdmin($adminInfo) {
        // 检查数据库连接是否有效
        if (!method_exists($this->db, 'prepare')) {
            return ['success' => false, 'error' => '数据库连接失败，请联系管理员'];
        }
        
        try {
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
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    /**
     * 获取用户的所有设备
     * @param int $userId 用户ID
     * @return array 用户设备列表
     */
    public function getUserDevices($userId) {
        $stmt = $this->db->prepare("SELECT d.* FROM devices d JOIN user_devices ud ON d.device_id = ud.device_id WHERE ud.user_id = :user_id");
        $stmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $devices = [];
        while ($device = $result->fetchArray(SQLITE3_ASSOC)) {
            $devices[] = $device;
        }
        return $devices;
    }
    
    /**
     * 检查设备是否属于用户
     * @param int $userId 用户ID
     * @param int $deviceId 设备ID
     * @return bool 是否属于用户
     */
    public function isDeviceOwnedByUser($userId, $deviceId) {
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM user_devices WHERE device_id = :device_id AND user_id = :user_id");
        $stmt->bindValue(':device_id', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $row = $result->fetchArray(SQLITE3_ASSOC);
        return $row['count'] > 0;
    }
    
    /**
     * 绑定设备到用户
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param string $nickname 设备昵称
     * @return array 绑定结果
     */
    public function bindDevice($userId, $deviceId, $nickname = '') {
        // 检查设备是否存在
        $deviceStmt = $this->db->prepare("SELECT device_id FROM devices WHERE device_id = :device_id");
        $deviceStmt->bindValue(':device_id', $deviceId, SQLITE3_TEXT);
        $deviceResult = $deviceStmt->execute();
        if (!$deviceResult->fetchArray(SQLITE3_ASSOC)) {
            return ['success' => false, 'error' => '设备不存在'];
        }
        
        // 检查是否已绑定
        $checkStmt = $this->db->prepare("SELECT id FROM user_devices WHERE user_id = :user_id AND device_id = :device_id");
        $checkStmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
        $checkStmt->bindValue(':device_id', $deviceId, SQLITE3_TEXT);
        $checkResult = $checkStmt->execute();
        if ($checkResult->fetchArray(SQLITE3_ASSOC)) {
            return ['success' => false, 'error' => '设备已绑定到该用户'];
        }
        
        // 绑定设备
        $stmt = $this->db->prepare("INSERT INTO user_devices (user_id, device_id, nickname, created_at) VALUES (:user_id, :device_id, :nickname, CURRENT_TIMESTAMP)");
        $stmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':device_id', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':nickname', $nickname, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        return ['success' => $result !== false];
    }
    
    /**
     * 解绑设备
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return array 解绑结果
     */
    public function unbindDevice($userId, $deviceId) {
        $stmt = $this->db->prepare("DELETE FROM user_devices WHERE user_id = :user_id AND device_id = :device_id");
        $stmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':device_id', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        return ['success' => $this->db->changes() > 0];
    }
    
    /**
     * 更新设备昵称
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param string $nickname 设备昵称
     * @return array 更新结果
     */
    public function updateDeviceNickname($userId, $deviceId, $nickname) {
        $stmt = $this->db->prepare("UPDATE user_devices SET nickname = :nickname WHERE user_id = :user_id AND device_id = :device_id");
        $stmt->bindValue(':nickname', $nickname, SQLITE3_TEXT);
        $stmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':device_id', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        return ['success' => $this->db->changes() > 0];
    }
}