<?php
/**
 * 设备服务接口
 */

namespace InkClock\Interfaces;

interface DeviceServiceInterface {
    /**
     * 注册设备
     * @param array $deviceInfo 设备信息
     * @return array 注册结果
     */
    public function registerDevice($deviceInfo);
    
    /**
     * 获取设备信息
     * @param string $deviceId 设备ID
     * @return array 设备信息
     */
    public function getDeviceInfo($deviceId);
    
    /**
     * 更新设备状态
     * @param string $deviceId 设备ID
     * @param array $statusInfo 状态信息
     * @return array 更新结果
     */
    public function updateDeviceStatus($deviceId, $statusInfo);
    
    /**
     * 获取用户的所有设备
     * @param int $userId 用户ID
     * @return array 设备列表
     */
    public function getUserDevices($userId);
    
    /**
     * 绑定设备到用户
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param string $nickname 设备昵称
     * @return array 绑定结果
     */
    public function bindDeviceToUser($userId, $deviceId, $nickname = '');
    
    /**
     * 解绑设备
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return array 解绑结果
     */
    public function unbindDevice($userId, $deviceId);
    
    /**
     * 获取所有设备
     * @param array $filters 过滤条件
     * @return array 设备列表
     */
    public function getAllDevices($filters = []);
    
    /**
     * 删除设备
     * @param string $deviceId 设备ID
     * @return array 删除结果
     */
    public function deleteDevice($deviceId);
}
?>