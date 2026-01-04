<?php
/**
 * 用户模型接口
 */

namespace App\Interface;

interface UserModelInterface {
    /**
     * 用户注册
     * @param array $userInfo 用户信息
     * @return array 注册结果
     */
    public function register($userInfo);
    
    /**
     * 用户登录
     * @param string $username 用户名
     * @param string $password 密码
     * @return array 登录结果
     */
    public function login($username, $password);
    
    /**
     * 通过API密钥获取用户
     * @param string $apiKey API密钥
     * @param string $ipAddress IP地址
     * @return array 用户信息
     */
    public function getUserByApiKey($apiKey, $ipAddress = '');
    
    /**
     * 通过用户名获取用户
     * @param string $username 用户名
     * @return array 用户信息
     */
    public function getUserByUsername($username);
    
    /**
     * 获取用户设备列表
     * @param int $userId 用户ID
     * @return array 设备列表
     */
    public function getUserDevices($userId);
    
    /**
     * 检查用户是否拥有设备
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return bool 是否拥有
     */
    public function isDeviceOwnedByUser($userId, $deviceId);
    
    /**
     * 绑定设备到用户
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param string $nickname 设备昵称
     * @return array 绑定结果
     */
    public function bindDevice($userId, $deviceId, $nickname = '');
    
    /**
     * 解绑设备
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return array 解绑结果
     */
    public function unbindDevice($userId, $deviceId);
    
    /**
     * 更新设备昵称
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param string $nickname 设备昵称
     * @return array 更新结果
     */
    public function updateDeviceNickname($userId, $deviceId, $nickname);
    
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
}
?>