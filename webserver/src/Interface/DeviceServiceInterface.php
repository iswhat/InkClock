<?php
/**
 * 设备服务接口
 */

namespace App\Interface;

interface DeviceServiceInterface {
    /**
     * 注册新设备
     * @param array $deviceInfo 设备信息
     * @return array 注册结果
     */
    public function registerDevice($deviceInfo);
    
    /**
     * 获取设备列表
     * @param int $userId 用户ID
     * @param array $filters 过滤条件
     * @return array 设备列表
     */
    public function getDeviceList($userId, $filters = []);
    
    /**
     * 获取设备详情
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return array 设备详情
     */
    public function getDeviceDetail($userId, $deviceId);
    
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
     * 更新设备信息
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param array $deviceInfo 设备信息
     * @return array 更新结果
     */
    public function updateDevice($userId, $deviceId, $deviceInfo);
    
    /**
     * 获取设备在线状态
     * @param string $deviceId 设备ID
     * @return array 设备状态
     */
    public function getDeviceStatus($deviceId);
}
?>