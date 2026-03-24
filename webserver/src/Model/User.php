<?php
/**
 * 用户模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class User extends BaseModel {
    
    private function generateApiKey() {
        return bin2hex(random_bytes(32));
    }
    
    public function generate2faSecret() {
        return bin2hex(random_bytes(16));
    }
    
    public function verify2faCode($secret, $code) {
        $timestamp = floor(time() / 30);
        for ($i = -1; $i <= 1; $i++) {
            $hash = hash_hmac('sha1', pack('N', $timestamp + $i), hex2bin($secret));
            $offset = ord(hex2bin(substr($hash, -1)));
            $truncatedHash = substr($hash, $offset * 2, 8);
            $calculatedCode = hexdec($truncatedHash) & 0x7fffffff;
            $calculatedCode = str_pad($calculatedCode % 1000000, 6, '0', STR_PAD_LEFT);
            if ($calculatedCode === $code) {
                return true;
            }
        }
        return false;
    }
    
    public function enable2fa($userId, $secret) {
        try {
            $sql = "UPDATE users SET two_factor_secret = :secret, two_factor_enabled = 1 WHERE id = :id";
            $params = ['secret' => $secret, 'id' => $userId];
            $result = $this->db->execute($sql, $params);
            return ['success' => $result !== false];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
    
    public function disable2fa($userId) {
        try {
            $sql = "UPDATE users SET two_factor_secret = NULL, two_factor_enabled = 0 WHERE id = :id";
            $params = ['id' => $userId];
            $result = $this->db->execute($sql, $params);
            return ['success' => $result !== false];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
    
    public function get2faSettings($userId) {
        try {
            $sql = "SELECT COALESCE(two_factor_secret, '') as two_factor_secret, COALESCE(two_factor_enabled, 0) as two_factor_enabled FROM users WHERE id = :id";
            $params = ['id' => $userId];
            $result = $this->db->query($sql, $params);
            return !empty($result) ? $result[0] : null;
        } catch (\Exception $e) {
            return null;
        }
    }
    
    public function register($userInfo) {
        $username = $userInfo['username'];
        $email = $userInfo['email'];
        $password = $userInfo['password'];
        
        try {
            $sql = "SELECT id FROM users WHERE username = :username OR email = :email";
            $params = ['username' => $username, 'email' => $email];
            $result = $this->db->query($sql, $params);
            
            if (!empty($result)) {
                return ['success' => false, 'error' => '用户名或邮箱已存在'];
            }
            
            $passwordHash = password_hash($password, PASSWORD_DEFAULT);
            $apiKey = $this->generateApiKey();
            $createdAt = date('Y-m-d H:i:s');
            $apiKeyExpiresAt = date('Y-m-d H:i:s', strtotime('+365 days'));
            
            $sql = "INSERT INTO users (username, email, password_hash, api_key, api_key_created_at, api_key_expires_at, created_at) VALUES (:username, :email, :password_hash, :api_key, :apiKeyCreatedAt, :apiKeyExpiresAt, :createdAt)";
            $params = [
                'username' => $username,
                'email' => $email,
                'password_hash' => $passwordHash,
                'api_key' => $apiKey,
                'apiKeyCreatedAt' => $createdAt,
                'apiKeyExpiresAt' => $apiKeyExpiresAt,
                'createdAt' => $createdAt
            ];
            
            $result = $this->db->execute($sql, $params);
            
            if ($result) {
                $userId = $this->db->lastInsertId();
                return ['success' => true, 'user_id' => $userId, 'api_key' => $apiKey];
            } else {
                return ['success' => false, 'error' => '注册失败'];
            }
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    public function login($username, $password) {
        try {
            $sql = "SELECT id, username, email, password_hash, api_key, api_key_expires_at, status, is_admin FROM users WHERE username = :username OR email = :email";
            $params = ['username' => $username, 'email' => $username];
            $result = $this->db->query($sql, $params);
            
            if (empty($result)) {
                return ['success' => false, 'error' => '用户名或密码错误'];
            }
            
            $user = $result[0];
            
            if (intval($user['status']) === 0) {
                return ['success' => false, 'error' => '用户账号已禁用'];
            }
            
            if (!password_verify($password, $user['password_hash'])) {
                return ['success' => false, 'error' => '用户名或密码错误'];
            }
            
            $now = time();
            $expiresAt = strtotime($user['api_key_expires_at']);
            $daysUntilExpiry = ($expiresAt - $now) / (60 * 60 * 24);
            
            $apiKey = $user['api_key'];
            if ($daysUntilExpiry < 30) {
                $apiKey = $this->generateApiKey();
                $apiKeyCreatedAt = date('Y-m-d H:i:s');
                $apiKeyExpiresAt = date('Y-m-d H:i:s', strtotime('+365 days'));
                
                $updateSql = "UPDATE users SET api_key = :apiKey, api_key_created_at = :apiKeyCreatedAt, api_key_expires_at = :apiKeyExpiresAt WHERE id = :id";
                $updateParams = [
                    'apiKey' => $apiKey,
                    'apiKeyCreatedAt' => $apiKeyCreatedAt,
                    'apiKeyExpiresAt' => $apiKeyExpiresAt,
                    'id' => $user['id']
                ];
                $this->db->execute($updateSql, $updateParams);
            }
            
            $lastLogin = date('Y-m-d H:i:s');
            $loginSql = "UPDATE users SET last_login = :last_login WHERE id = :id";
            $loginParams = ['last_login' => $lastLogin, 'id' => $user['id']];
            $this->db->execute($loginSql, $loginParams);
            
            return [
                'success' => true,
                'user_id' => $user['id'],
                'username' => $user['username'],
                'email' => $user['email'],
                'api_key' => $apiKey,
                'is_admin' => $user['is_admin'],
                'role' => $user['is_admin'] ? 'admin' : 'user'
            ];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    public function getUserByApiKey($apiKey, $ipAddress = '') {
        $sql = "SELECT id, username, email, is_admin, status, api_key_expires_at, api_key_ip_whitelist FROM users WHERE api_key = :api_key AND status = 1";
        $params = ['api_key' => $apiKey];
        $result = $this->db->query($sql, $params);
        
        if (empty($result)) {
            return null;
        }
        
        $user = $result[0];
        
        $now = date('Y-m-d H:i:s');
        if ($user['api_key_expires_at'] && $user['api_key_expires_at'] < $now) {
            return null;
        }
        
        if ($user['api_key_ip_whitelist']) {
            $whitelist = explode(',', $user['api_key_ip_whitelist']);
            $whitelist = array_map('trim', $whitelist);
            if (!in_array($ipAddress, $whitelist)) {
                return null;
            }
        }
        
        return $user;
    }
    
    public function rotateApiKey($userId) {
        $newApiKey = $this->generateApiKey();
        $now = date('Y-m-d H:i:s');
        $expiresAt = date('Y-m-d H:i:s', strtotime('+365 days'));
        
        $sql = "UPDATE users SET api_key = :apiKey, api_key_created_at = :createdAt, api_key_expires_at = :expiresAt WHERE id = :userId";
        $params = [
            'apiKey' => $newApiKey,
            'createdAt' => $now,
            'expiresAt' => $expiresAt,
            'userId' => $userId
        ];
        
        $this->db->execute($sql, $params);
        
        return ['success' => true, 'api_key' => $newApiKey, 'expires_at' => $expiresAt];
    }
    
    public function updateApiKeyExpiration($userId, $expiresAt) {
        $sql = "UPDATE users SET api_key_expires_at = :expiresAt WHERE id = :userId";
        $params = ['expiresAt' => $expiresAt, 'userId' => $userId];
        $result = $this->db->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function updateApiKeyIpWhitelist($userId, $ipList) {
        $whitelist = implode(',', $ipList);
        $sql = "UPDATE users SET api_key_ip_whitelist = :whitelist WHERE id = :userId";
        $params = ['whitelist' => $whitelist, 'userId' => $userId];
        $result = $this->db->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function getUserByUsername($username) {
        $sql = "SELECT * FROM users WHERE username = :username";
        $params = ['username' => $username];
        $result = $this->db->query($sql, $params);
        return !empty($result) ? $result[0] : null;
    }
    
    public function isAdmin($userId) {
        $sql = "SELECT is_admin FROM users WHERE id = :id";
        $params = ['id' => $userId];
        $result = $this->db->query($sql, $params);
        if (!empty($result)) {
            return intval($result[0]['is_admin']) === 1;
        }
        return false;
    }
    
    public function hasUsers() {
        try {
            $result = $this->db->query("SELECT * FROM users LIMIT 1");
            return !empty($result);
        } catch (\Exception $e) {
            return false;
        }
    }
    
    public function createFirstAdmin($adminInfo) {
        try {
            if ($this->hasUsers()) {
                return ['success' => false, 'error' => '已有用户存在，无法创建第一个管理员'];
            }
            
            $username = $adminInfo['username'];
            $email = $adminInfo['email'];
            $password = $adminInfo['password'];
            
            if (empty($username) || strlen($username) < 3 || empty($email) || !filter_var($email, FILTER_VALIDATE_EMAIL) || empty($password) || strlen($password) < 6) {
                return ['success' => false, 'error' => '无效的用户信息'];
            }
            
            $passwordHash = password_hash($password, PASSWORD_DEFAULT);
            $apiKey = $this->generateApiKey();
            $createdAt = date('Y-m-d H:i:s');
            
            $sql = "INSERT INTO users (username, email, password_hash, api_key, created_at, is_admin, status, must_change_password) VALUES (:username, :email, :password_hash, :api_key, :created_at, 1, 1, 1)";
            $params = [
                'username' => $username,
                'email' => $email,
                'password_hash' => $passwordHash,
                'api_key' => $apiKey,
                'created_at' => $createdAt
            ];
            
            $result = $this->db->execute($sql, $params);
            if ($result) {
                $userId = $this->db->lastInsertId();
                return ['success' => true, 'user_id' => $userId, 'api_key' => $apiKey];
            }
            
            return ['success' => false, 'error' => '管理员创建失败'];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    public function getUserDevices($userId) {
        $sql = "SELECT d.* FROM devices d JOIN user_devices ud ON d.device_id = ud.device_id WHERE ud.user_id = :user_id";
        $params = ['user_id' => $userId];
        return $this->db->query($sql, $params);
    }
    
    public function isDeviceOwnedByUser($userId, $deviceId) {
        $sql = "SELECT COUNT(*) as count FROM user_devices WHERE device_id = :device_id AND user_id = :user_id";
        $params = ['device_id' => $deviceId, 'user_id' => $userId];
        $result = $this->db->query($sql, $params);
        if (!empty($result)) {
            return intval($result[0]['count']) > 0;
        }
        return false;
    }
    
    public function bindDevice($userId, $deviceId, $nickname = '') {
        $sql = "SELECT device_id FROM devices WHERE device_id = :device_id";
        $params = ['device_id' => $deviceId];
        $result = $this->db->query($sql, $params);
        
        if (empty($result)) {
            return ['success' => false, 'error' => '设备不存在'];
        }
        
        $sql = "SELECT id FROM user_devices WHERE user_id = :user_id AND device_id = :device_id";
        $params = ['user_id' => $userId, 'device_id' => $deviceId];
        $result = $this->db->query($sql, $params);
        
        if (!empty($result)) {
            return ['success' => false, 'error' => '设备已绑定到该用户'];
        }
        
        $sql = "INSERT INTO user_devices (user_id, device_id, nickname, created_at) VALUES (:user_id, :device_id, :nickname, CURRENT_TIMESTAMP)";
        $params = ['user_id' => $userId, 'device_id' => $deviceId, 'nickname' => $nickname];
        $result = $this->db->execute($sql, $params);
        
        return ['success' => $result !== false];
    }
    
    public function unbindDevice($userId, $deviceId) {
        $sql = "DELETE FROM user_devices WHERE user_id = :user_id AND device_id = :device_id";
        $params = ['user_id' => $userId, 'device_id' => $deviceId];
        $result = $this->db->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function updateDeviceNickname($userId, $deviceId, $nickname) {
        $sql = "UPDATE user_devices SET nickname = :nickname WHERE user_id = :user_id AND device_id = :device_id";
        $params = ['nickname' => $nickname, 'user_id' => $userId, 'device_id' => $deviceId];
        $result = $this->db->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function getAllUsers($search = '') {
        try {
            $sql = "SELECT id, username, email, is_admin, created_at, last_login, status FROM users";
            $params = [];
            
            if (!empty($search)) {
                $sql .= " WHERE username LIKE :search OR email LIKE :search";
                $params['search'] = "%$search%";
            }
            
            $sql .= " ORDER BY created_at DESC";
            
            $users = $this->db->query($sql, $params);
            
            foreach ($users as &$row) {
                $row['role'] = $row['is_admin'] ? 'admin' : 'user';
                unset($row['is_admin']);
            }
            
            return $users;
        } catch (\Exception $e) {
            return [];
        }
    }
    
    public function getUserStats() {
        try {
            $totalResult = $this->db->query("SELECT COUNT(*) as count FROM users");
            $total = !empty($totalResult) ? intval($totalResult[0]['count']) : 0;
            
            $adminResult = $this->db->query("SELECT COUNT(*) as count FROM users WHERE is_admin = 1");
            $admin = !empty($adminResult) ? intval($adminResult[0]['count']) : 0;
            
            $userResult = $this->db->query("SELECT COUNT(*) as count FROM users WHERE is_admin = 0");
            $user = !empty($userResult) ? intval($userResult[0]['count']) : 0;
            
            $activeResult = $this->db->query("SELECT COUNT(*) as count FROM users WHERE last_login >= datetime('now', '-30 days')");
            $active = !empty($activeResult) ? intval($activeResult[0]['count']) : 0;
            
            return ['total' => $total, 'admin' => $admin, 'user' => $user, 'active' => $active];
        } catch (\Exception $e) {
            return ['total' => 0, 'admin' => 0, 'user' => 0, 'active' => 0];
        }
    }
    
    public function getUserById($userId) {
        try {
            $sql = "SELECT id, username, email, is_admin, created_at, last_login, status FROM users WHERE id = :id";
            $params = ['id' => $userId];
            $result = $this->db->query($sql, $params);
            
            if (!empty($result)) {
                $user = $result[0];
                $user['role'] = $user['is_admin'] ? 'admin' : 'user';
                return $user;
            }
            return null;
        } catch (\Exception $e) {
            return null;
        }
    }
    
    public function updateUser($userId, $userData) {
        try {
            $fields = [];
            $params = [];
            
            if (isset($userData['username'])) {
                $fields[] = "username = :username";
                $params['username'] = $userData['username'];
            }
            
            if (isset($userData['email'])) {
                $fields[] = "email = :email";
                $params['email'] = $userData['email'];
            }
            
            if (!empty($userData['password'])) {
                $fields[] = "password_hash = :password_hash";
                $params['password_hash'] = password_hash($userData['password'], PASSWORD_DEFAULT);
            }
            
            if (isset($userData['role'])) {
                $fields[] = "is_admin = :is_admin";
                $params['is_admin'] = $userData['role'] === 'admin' ? 1 : 0;
            }
            
            if (isset($userData['status'])) {
                $fields[] = "status = :status";
                $params['status'] = $userData['status'];
            }
            
            if (empty($fields)) {
                return ['success' => true, 'message' => '无需要更新的字段'];
            }
            
            $sql = "UPDATE users SET " . implode(', ', $fields) . " WHERE id = :id";
            $params['id'] = $userId;
            
            $result = $this->db->execute($sql, $params);
            
            if ($result !== false) {
                return ['success' => true, 'message' => '用户信息更新成功'];
            } else {
                return ['success' => false, 'error' => '更新失败或无变化'];
            }
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    public function deleteUser($userId) {
        try {
            $sql = "DELETE FROM user_devices WHERE user_id = :user_id";
            $params = ['user_id' => $userId];
            $this->db->execute($sql, $params);
            
            $sql = "DELETE FROM users WHERE id = :user_id";
            $params = ['user_id' => $userId];
            $result = $this->db->execute($sql, $params);
            
            if ($result !== false) {
                return ['success' => true, 'message' => '用户删除成功'];
            } else {
                return ['success' => false, 'error' => '删除失败或用户不存在'];
            }
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    public function addUser($userData) {
        try {
            $username = $userData['username'];
            $email = $userData['email'];
            $password = $userData['password'];
            $role = $userData['role'];
            
            $sql = "SELECT id FROM users WHERE username = :username OR email = :email";
            $params = ['username' => $username, 'email' => $email];
            $result = $this->db->query($sql, $params);
            
            if (!empty($result)) {
                return ['success' => false, 'error' => '用户名或邮箱已存在'];
            }
            
            $passwordHash = password_hash($password, PASSWORD_DEFAULT);
            $apiKey = $this->generateApiKey();
            $createdAt = date('Y-m-d H:i:s');
            $isAdmin = $role === 'admin' ? 1 : 0;
            
            $sql = "INSERT INTO users (username, email, password_hash, api_key, created_at, is_admin, status) VALUES (:username, :email, :password_hash, :api_key, :created_at, :is_admin, 1)";
            $params = [
                'username' => $username,
                'email' => $email,
                'password_hash' => $passwordHash,
                'api_key' => $apiKey,
                'created_at' => $createdAt,
                'is_admin' => $isAdmin
            ];
            
            $result = $this->db->execute($sql, $params);
            
            if ($result) {
                $userId = $this->db->lastInsertId();
                return ['success' => true, 'user_id' => $userId, 'message' => '用户添加成功'];
            } else {
                return ['success' => false, 'error' => '添加失败'];
            }
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '数据库操作失败: ' . $e->getMessage()];
        }
    }
    
    public function checkUsers() {
        try {
            $result = $this->db->query("SELECT COUNT(*) as count FROM users");
            return !empty($result) && intval($result[0]['count']) > 0;
        } catch (\Exception $e) {
            return false;
        }
    }
    
    /**
     * 检查用户是否需要首次修改密码
     * @param int $userId 用户 ID
     * @return bool 是否需要修改密码
     */
    public function mustChangePassword($userId) {
        try {
            $sql = "SELECT must_change_password FROM users WHERE id = :id";
            $params = ['id' => $userId];
            $result = $this->db->query($sql, $params);
            
            if (empty($result)) {
                return false;
            }
            
            return intval($result[0]['must_change_password']) === 1;
        } catch (\Exception $e) {
            return false;
        }
    }
    
    /**
     * 设置用户必须修改密码标志
     * @param int $userId 用户 ID
     * @param bool $mustChange 是否必须修改
     * @return array 操作结果
     */
    public function setMustChangePassword($userId, $mustChange) {
        try {
            $sql = "UPDATE users SET must_change_password = :must_change WHERE id = :id";
            $params = [
                'must_change' => $mustChange ? 1 : 0,
                'id' => $userId
            ];
            $result = $this->db->execute($sql, $params);
            return ['success' => $result !== false];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
    
    /**
     * 修改用户密码
     * @param int $userId 用户 ID
     * @param string $oldPassword 旧密码
     * @param string $newPassword 新密码
     * @return array 操作结果
     */
    public function changePassword($userId, $oldPassword, $newPassword) {
        try {
            // 验证旧密码
            $sql = "SELECT password_hash FROM users WHERE id = :id";
            $params = ['id' => $userId];
            $result = $this->db->query($sql, $params);
            
            if (empty($result)) {
                return ['success' => false, 'error' => '用户不存在'];
            }
            
            if (!password_verify($oldPassword, $result[0]['password_hash'])) {
                return ['success' => false, 'error' => '旧密码错误'];
            }
            
            // 更新密码
            $passwordHash = password_hash($newPassword, PASSWORD_DEFAULT);
            $sql = "UPDATE users SET password_hash = :password_hash, must_change_password = 0 WHERE id = :id";
            $params = [
                'password_hash' => $passwordHash,
                'id' => $userId
            ];
            $result = $this->db->execute($sql, $params);
            
            return ['success' => $result !== false];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
    
    /**
     * 强制重置用户密码（管理员功能）
     * @param int $userId 用户 ID
     * @param string $newPassword 新密码
     * @return array 操作结果
     */
    public function forceResetPassword($userId, $newPassword) {
        try {
            $passwordHash = password_hash($newPassword, PASSWORD_DEFAULT);
            $sql = "UPDATE users SET password_hash = :password_hash, must_change_password = 1 WHERE id = :id";
            $params = [
                'password_hash' => $passwordHash,
                'id' => $userId
            ];
            $result = $this->db->execute($sql, $params);
            return ['success' => $result !== false];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => $e->getMessage()];
        }
    }
}
?>
