<?php
/**
 * 用户模型
 */
require_once __DIR__ . '/../utils/Database.php';

class User {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
    }
    
    /**
     * 生成API密钥
     */
    private function generateApiKey() {
        return bin2hex(random_bytes(32));
    }
    
    /**
     * 用户注册
     */
    public function register($userInfo) {
        $username = $userInfo['username'];
        $email = $userInfo['email'];
        $password = $userInfo['password'];
        
        // 检查用户名或邮箱是否已存在
        $stmt = $this->db->prepare("SELECT id FROM users WHERE username = ? OR email = ?");
        $stmt->bind_param("ss", $username, $email);
        $stmt->execute();
        $result = $stmt->get_result();
        
        if ($result->num_rows > 0) {
            return array('success' => false, 'error' => '用户名或邮箱已存在');
        }
        
        // 哈希密码
        $passwordHash = password_hash($password, PASSWORD_DEFAULT);
        $apiKey = $this->generateApiKey();
        $createdAt = date('Y-m-d H:i:s');
        
        // 插入用户
        $stmt = $this->db->prepare("INSERT INTO users (username, email, password_hash, api_key, created_at) VALUES (?, ?, ?, ?, ?)");
        $stmt->bind_param("sssss", $username, $email, $passwordHash, $apiKey, $createdAt);
        $stmt->execute();
        
        $userId = $this->db->insert_id;
        $stmt->close();
        
        return array('success' => true, 'user_id' => $userId, 'api_key' => $apiKey);
    }
    
    /**
     * 用户登录
     */
    public function login($username, $password) {
        // 查找用户
        $stmt = $this->db->prepare("SELECT id, password_hash, api_key, status FROM users WHERE username = ? OR email = ?");
        $stmt->bind_param("ss", $username, $username);
        $stmt->execute();
        $result = $stmt->get_result();
        
        if ($result->num_rows === 0) {
            return array('success' => false, 'error' => '用户名或密码错误');
        }
        
        $user = $result->fetch_assoc();
        
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
        $stmt = $this->db->prepare("UPDATE users SET last_login = ? WHERE id = ?");
        $stmt->bind_param("si", $lastLogin, $user['id']);
        $stmt->execute();
        $stmt->close();
        
        return array('success' => true, 'user_id' => $user['id'], 'api_key' => $user['api_key']);
    }
    
    /**
     * 通过API密钥获取用户信息
     */
    public function getUserByApiKey($apiKey) {
        $stmt = $this->db->prepare("SELECT * FROM users WHERE api_key = ? AND status = 1");
        $stmt->bind_param("s", $apiKey);
        $stmt->execute();
        $result = $stmt->get_result();
        $user = $result->fetch_assoc();
        $stmt->close();
        return $user;
    }
    
    /**
     * 绑定设备到用户
     */
    public function bindDevice($userId, $deviceId, $nickname = '') {
        // 检查设备是否存在
        $deviceModel = new Device();
        if (!$deviceModel->deviceExists($deviceId)) {
            return array('success' => false, 'error' => '设备不存在');
        }
        
        // 检查是否已绑定
        $stmt = $this->db->prepare("SELECT id FROM user_devices WHERE user_id = ? AND device_id = ?");
        $stmt->bind_param("is", $userId, $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        
        if ($result->num_rows > 0) {
            // 更新昵称
            $stmt = $this->db->prepare("UPDATE user_devices SET nickname = ? WHERE user_id = ? AND device_id = ?");
            $stmt->bind_param("sis", $nickname, $userId, $deviceId);
            $stmt->execute();
        } else {
            // 绑定设备
            $createdAt = date('Y-m-d H:i:s');
            $stmt = $this->db->prepare("INSERT INTO user_devices (user_id, device_id, nickname, created_at) VALUES (?, ?, ?, ?)");
            $stmt->bind_param("isss", $userId, $deviceId, $nickname, $createdAt);
            $stmt->execute();
        }
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 解绑设备
     */
    public function unbindDevice($userId, $deviceId) {
        $stmt = $this->db->prepare("DELETE FROM user_devices WHERE user_id = ? AND device_id = ?");
        $stmt->bind_param("is", $userId, $deviceId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 获取用户绑定的设备列表
     */
    public function getUserDevices($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT ud.*, d.model, d.firmware_version, d.last_active FROM user_devices ud JOIN devices d ON ud.device_id = d.device_id WHERE ud.user_id = ? ORDER BY ud.created_at DESC LIMIT ? OFFSET ?");
        $stmt->bind_param("iii", $userId, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $devices = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        return $devices;
    }
    
    /**
     * 检查设备是否属于用户
     */
    public function isDeviceOwnedByUser($userId, $deviceId) {
        $stmt = $this->db->prepare("SELECT id FROM user_devices WHERE user_id = ? AND device_id = ?");
        $stmt->bind_param("is", $userId, $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $exists = $result->num_rows > 0;
        $stmt->close();
        return $exists;
    }
    
    /**
     * 获取设备所属用户ID
     */
    public function getDeviceOwner($deviceId) {
        $stmt = $this->db->prepare("SELECT user_id FROM user_devices WHERE device_id = ?");
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $row = $result->fetch_assoc();
        $stmt->close();
        return $row ? $row['user_id'] : null;
    }
    
    /**
     * 更新设备昵称
     */
    public function updateDeviceNickname($userId, $deviceId, $nickname) {
        $stmt = $this->db->prepare("UPDATE user_devices SET nickname = ? WHERE user_id = ? AND device_id = ?");
        $stmt->bind_param("sis", $nickname, $userId, $deviceId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
}
?>