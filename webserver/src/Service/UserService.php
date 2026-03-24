<?php
/**
 * 用户服务层
 */

namespace InkClock\Service;

use InkClock\Utils\Database;
use InkClock\Utils\Logger;
use InkClock\Utils\Cache;
use InkClock\Model\User;

class UserService {
    private $db;
    private $logger;
    private $cache;
    private $userModel;
    
    public function __construct(Database $db, Logger $logger, Cache $cache) {
        $this->db = $db;
        $this->logger = $logger;
        $this->cache = $cache;
        $this->userModel = new User($db);
    }
    
    /**
     * 检查用户是否需要首次修改密码
     * @param int $userId 用户 ID
     * @return bool 是否需要修改密码
     */
    public function mustChangePassword($userId) {
        return $this->userModel->mustChangePassword($userId);
    }
    
    /**
     * 设置用户必须修改密码标志
     * @param int $userId 用户 ID
     * @param bool $mustChange 是否必须修改
     * @return array 操作结果
     */
    public function setMustChangePassword($userId, $mustChange) {
        $this->logger->info('设置用户密码修改标志', ['user_id' => $userId, 'must_change' => $mustChange]);
        return $this->userModel->setMustChangePassword($userId, $mustChange);
    }
    
    /**
     * 修改用户密码
     * @param int $userId 用户 ID
     * @param string $oldPassword 旧密码
     * @param string $newPassword 新密码
     * @return array 操作结果
     */
    public function changePassword($userId, $oldPassword, $newPassword) {
        $this->logger->info('用户修改密码', ['user_id' => $userId]);
        
        $result = $this->userModel->changePassword($userId, $oldPassword, $newPassword);
        
        if ($result['success']) {
            $this->logger->info('用户密码修改成功', ['user_id' => $userId]);
            // 清除用户相关缓存
            $this->cache->delete("user:$userId");
        } else {
            $this->logger->error('用户密码修改失败', ['user_id' => $userId, 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 强制重置用户密码（管理员功能）
     * @param int $userId 用户 ID
     * @param string $newPassword 新密码
     * @return array 操作结果
     */
    public function forceResetPassword($userId, $newPassword) {
        $this->logger->info('管理员强制重置用户密码', ['admin_id' => $userId, 'target_user_id' => $userId]);
        
        $result = $this->userModel->forceResetPassword($userId, $newPassword);
        
        if ($result['success']) {
            $this->logger->info('用户密码重置成功', ['user_id' => $userId]);
            // 清除用户相关缓存
            $this->cache->delete("user:$userId");
        } else {
            $this->logger->error('用户密码重置失败', ['user_id' => $userId, 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 获取用户信息
     * @param int $userId 用户 ID
     * @return array|null 用户信息
     */
    public function getUser($userId) {
        $cacheKey = "user:$userId";
        $cached = $this->cache->get($cacheKey);
        
        if ($cached) {
            return $cached;
        }
        
        $sql = "SELECT id, username, email, is_admin, status, created_at, last_login FROM users WHERE id = :id";
        $params = ['id' => $userId];
        $result = $this->db->query($sql, $params);
        
        if (empty($result)) {
            return null;
        }
        
        $user = $result[0];
        $this->cache->set($cacheKey, $user, 300); // 缓存 5 分钟
        
        return $user;
    }
}
