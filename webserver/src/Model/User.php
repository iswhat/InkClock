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
        $stmt = $this->db->prepare("SELECT id, username, email, is_admin, status, api_key_expires_at, api_key_ip_whitelist FROM users WHERE api_key = :api_key AND status = 1");
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
    
    /**
     * 获取所有用户列表
     * @param string $search 搜索关键词
     * @return array 用户列表
     */
    public function getAllUsers($search = '') {
        try {
            $query = "SELECT id, username, email, is_admin, created_at, last_login, status FROM users";
            $params = [];
            
            if (!empty($search)) {
                $query .= " WHERE username LIKE :search OR email LIKE :search";
                $search = "%$search%";
            }
            
            $query .= " ORDER BY created_at DESC";
            
            $stmt = $this->db->prepare($query);
            if (!empty($search)) {
                $stmt->bindValue(':search', $search, SQLITE3_TEXT);
            }
            
            $result = $stmt->execute();
            $users = [];
            
            while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
                // 转换is_admin为role字段
                $row['role'] = $row['is_admin'] ? 'admin' : 'user';
                unset($row['is_admin']);
                $users[] = $row;
            }
            
            return $users;
        } catch (\Exception $e) {
            return [];
        }
    }
    
    /**
     * 获取用户统计数据
     * @return array 统计数据
     */
    public function getUserStats() {
        try {
            // 获取总用户数
            $totalStmt = $this->db->prepare("SELECT COUNT(*) as count FROM users");
            $totalResult = $totalStmt->execute();
            $total = $totalResult->fetchArray(SQLITE3_ASSOC)['count'];
            
            // 获取管理员数
            $adminStmt = $this->db->prepare("SELECT COUNT(*) as count FROM users WHERE is_admin = 1");
            $adminResult = $adminStmt->execute();
            $admin = $adminResult->fetchArray(SQLITE3_ASSOC)['count'];
            
            // 获取普通用户数
            $userStmt = $this->db->prepare("SELECT COUNT(*) as count FROM users WHERE is_admin = 0");
            $userResult = $userStmt->execute();
            $user = $userResult->fetchArray(SQLITE3_ASSOC)['count'];
            
            // 获取活跃用户数（最近30天登录过的）
            $activeStmt = $this->db->prepare("SELECT COUNT(*) as count FROM users WHERE last_login >= datetime('now', '-30 days')");
            $activeResult = $activeStmt->execute();
            $active = $activeResult->fetchArray(SQLITE3_ASSOC)['count'];
            
            return [
                'total' => $total,
                'admin' => $admin,
                'user' => $user,
                'active' => $active
            ];
        } catch (\Exception $e) {
            return [
                'total' => 0,
                'admin' => 0,
                'user' => 0,
                'active' => 0
            ];
        }
    }
    
    /**
     * 通过ID获取用户信息
     * @param int $userId 用户ID
     * @return array|null 用户信息
     */
    public function getUserById($userId) {
        try {
            $stmt = $this->db->prepare("SELECT id, username, email, is_admin as role, created_at, last_login, status FROM users WHERE id = :id");
            $stmt->bindValue(':id', $userId, SQLITE3_INTEGER);
            $result = $stmt->execute();
            $user = $result->fetchArray(SQLITE3_ASSOC);
            
            if ($user) {
                // 转换role字段
                $user['role'] = $user['role'] ? 'admin' : 'user';
            }
            
            return $user;
        } catch (\Exception $e) {
            return null;
        }
    }
    
    /**
     * 更新用户信息
     * @param int $userId 用户ID
     * @param array $userData 用户数据
     * @return array 更新结果
     */
    public function updateUser($userId, $userData) {
        try {
            $fields = [];
            $params = [];
            
            if (isset($userData['username'])) {
                $fields[] = "username = :username";
                $params[':username'] = $userData['username'];
            }
            
            if (isset($userData['email'])) {
                $fields[] = "email = :email";
                $params[':email'] = $userData['email'];
            }
            
            if (!empty($userData['password'])) {
                $fields[] = "password_hash = :password_hash";
                $params[':password_hash'] = password_hash($userData['password'], PASSWORD_DEFAULT);
            }
            
            if (isset($userData['role'])) {
                $fields[] = "is_admin = :is_admin";
                $params[':is_admin'] = $userData['role'] === 'admin' ? 1 : 0;
            }
            
            if (isset($userData['status'])) {
                $fields[] = "status = :status";
                $params[':status'] = $userData['status'];
            }
            
            if (empty($fields)) {
                return ['success' => true, 'message' => '无需要更新的字段'];
            }
            
            $query = "UPDATE users SET " . implode(', ', $fields) . " WHERE id = :id";
            $params[':id'] = $userId;
            
            $stmt = $this->db->prepare($query);
            foreach ($params as $key => $value) {
                $stmt->bindValue($key, $value, is_numeric($value) ? SQLITE3_INTEGER : SQLITE3_TEXT);
            }
            
            $result = $stmt->execute();
            
            if ($result && $this->db->changes() > 0) {
                return ['success' => true, 'message' => '用户信息更新成功'];
            } else {
                return ['success' => false, 'error' => '更新失败或无变化'];
            }
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    /**
     * 删除用户
     * @param int $userId 用户ID
     * @return array 删除结果
     */
    public function deleteUser($userId) {
        try {
            // 开始事务
            $this->db->exec('BEGIN TRANSACTION');
            
            // 删除用户的设备绑定
            $deviceStmt = $this->db->prepare("DELETE FROM user_devices WHERE user_id = :user_id");
            $deviceStmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
            $deviceStmt->execute();
            
            // 删除用户
            $userStmt = $this->db->prepare("DELETE FROM users WHERE id = :user_id");
            $userStmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
            $userResult = $userStmt->execute();
            
            if ($userResult && $this->db->changes() > 0) {
                $this->db->exec('COMMIT');
                return ['success' => true, 'message' => '用户删除成功'];
            } else {
                $this->db->exec('ROLLBACK');
                return ['success' => false, 'error' => '删除失败或用户不存在'];
            }
        } catch (\Exception $e) {
            $this->db->exec('ROLLBACK');
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    /**
     * 添加用户
     * @param array $userData 用户数据
     * @return array 添加结果
     */
    public function addUser($userData) {
        try {
            $username = $userData['username'];
            $email = $userData['email'];
            $password = $userData['password'];
            $role = $userData['role'];
            
            // 检查用户名或邮箱是否已存在
            $checkStmt = $this->db->prepare("SELECT id FROM users WHERE username = :username OR email = :email");
            $checkStmt->bindValue(':username', $username, SQLITE3_TEXT);
            $checkStmt->bindValue(':email', $email, SQLITE3_TEXT);
            $checkResult = $checkStmt->execute();
            
            if ($checkResult->fetchArray(SQLITE3_ASSOC)) {
                return ['success' => false, 'error' => '用户名或邮箱已存在'];
            }
            
            // 哈希密码
            $passwordHash = password_hash($password, PASSWORD_DEFAULT);
            $apiKey = $this->generateApiKey();
            $createdAt = date('Y-m-d H:i:s');
            $isAdmin = $role === 'admin' ? 1 : 0;
            
            // 插入用户
            $stmt = $this->db->prepare("INSERT INTO users (username, email, password_hash, api_key, created_at, is_admin, status) VALUES (:username, :email, :password_hash, :api_key, :created_at, :is_admin, 1)");
            $stmt->bindValue(':username', $username, SQLITE3_TEXT);
            $stmt->bindValue(':email', $email, SQLITE3_TEXT);
            $stmt->bindValue(':password_hash', $passwordHash, SQLITE3_TEXT);
            $stmt->bindValue(':api_key', $apiKey, SQLITE3_TEXT);
            $stmt->bindValue(':created_at', $createdAt, SQLITE3_TEXT);
            $stmt->bindValue(':is_admin', $isAdmin, SQLITE3_INTEGER);
            
            $result = $stmt->execute();
            
            if ($result) {
                $userId = $this->db->lastInsertRowID();
                return ['success' => true, 'user_id' => $userId, 'message' => '用户添加成功'];
            } else {
                return ['success' => false, 'error' => '添加失败'];
            }
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
}