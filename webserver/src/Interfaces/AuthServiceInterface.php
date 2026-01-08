<?php
/**
 * 认证服务接口
 */

namespace InkClock\Interfaces;

interface AuthServiceInterface {
    /**
     * 用户注册
     * @param array $userInfo 用户信息
     * @return array 注册结果
     */
    public function registerUser($userInfo);
    
    /**
     * 用户登录
     * @param string $username 用户名或邮箱
     * @param string $password 密码
     * @return array 登录结果
     */
    public function loginUser($username, $password);
    
    /**
     * 通过API密钥验证用户
     * @param string $apiKey API密钥
     * @param string $ipAddress 请求IP地址
     * @return array 验证结果
     */
    public function validateApiKey($apiKey, $ipAddress = '');
    
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
     * @param mixed $user 用户信息或用户名
     * @return bool 是否为管理员
     */
    public function isAdmin($user);
}
?>