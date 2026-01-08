<?php
/**
 * 用户模型接口
 */

namespace InkClock\Interfaces;

interface UserModelInterface {
    /**
     * 用户注册
     * @param array $userInfo 用户信息
     * @return array 注册结果
     */
    public function register($userInfo);
    
    /**
     * 用户登录
     * @param string $username 用户名或邮箱
     * @param string $password 密码
     * @return array 登录结果
     */
    public function login($username, $password);
    
    /**
     * 通过用户名获取用户
     * @param string $username 用户名
     * @return array|null 用户信息
     */
    public function getUserByUsername($username);
    
    /**
     * 通过邮箱获取用户
     * @param string $email 邮箱
     * @return array|null 用户信息
     */
    public function getUserByEmail($email);
    
    /**
     * 通过API密钥获取用户
     * @param string $apiKey API密钥
     * @param string $ipAddress 请求IP地址
     * @return array|null 用户信息
     */
    public function getUserByApiKey($apiKey, $ipAddress = '');
    
    /**
     * 更新用户信息
     * @param int $userId 用户ID
     * @param array $userInfo 用户信息
     * @return array 更新结果
     */
    public function updateUser($userId, $userInfo);
    
    /**
     * 更新用户密码
     * @param int $userId 用户ID
     * @param string $oldPassword 旧密码
     * @param string $newPassword 新密码
     * @return array 更新结果
     */
    public function updatePassword($userId, $oldPassword, $newPassword);
    
    /**
     * 检查是否有用户存在
     * @return bool 是否有用户
     */
    public function hasUsers();
    
    /**
     * 创建第一个管理员用户
     * @param array $adminInfo 管理员信息
     * @return array 创建结果
     */
    public function createFirstAdmin($adminInfo);
    
    /**
     * 检查用户是否为管理员
     * @param int $userId 用户ID
     * @return bool 是否为管理员
     */
    public function isAdmin($userId);
    
    /**
     * 获取所有用户
     * @param array $filters 过滤条件
     * @return array 用户列表
     */
    public function getAllUsers($filters = []);
    
    /**
     * 删除用户
     * @param int $userId 用户ID
     * @return array 删除结果
     */
    public function deleteUser($userId);
}
?>