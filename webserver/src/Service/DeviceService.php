<?php
/**
 * 设备服务类
 */

namespace InkClock\Service;

use InkClock\Utils\Logger;
use InkClock\Utils\Cache;
use InkClock\Model\Device;
use InkClock\Model\User;
use InkClock\Interfaces\DeviceServiceInterface;

class DeviceService implements DeviceServiceInterface {
    private $db;
    private $logger;
    private $cache;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     * @param Logger $logger 日志服务
     * @param Cache $cache 缓存服务
     */
    public function __construct($db, Logger $logger, Cache $cache = null) {
        $this->db = $db;
        $this->logger = $logger;
        $this->cache = $cache;
    }
    
    /**
     * 注册新设备
     * @param array $deviceInfo 设备信息
     * @return array 注册结果
     */
    public function registerDevice($deviceInfo) {
        $this->logger->info('设备注册请求', $deviceInfo);
        
        // 验证设备信息
        if (!$this->validateDeviceInfo($deviceInfo)) {
            $this->logger->warning('设备注册信息验证失败', $deviceInfo);
            return ['success' => false, 'error' => '无效的设备信息'];
        }
        
        // 调用模型进行注册
        $deviceModel = new Device($this->db);
        $result = $deviceModel->registerDevice($deviceInfo);
        
        if ($result['success']) {
            $this->logger->info('设备注册成功', ['device_id' => $deviceInfo['device_id']]);
        } else {
            $this->logger->warning('设备注册失败', ['device_id' => $deviceInfo['device_id'], 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 获取设备列表
     * @param int $userId 用户ID
     * @param array $filters 过滤条件
     * @return array 设备列表
     */
    public function getDeviceList($userId, $filters = []) {
        return $this->getUserDevices($userId);
    }
    
    /**
     * 获取设备信息
     * @param string $deviceId 设备ID
     * @return array 设备信息
     */
    public function getDeviceInfo($deviceId) {
        $this->logger->info('获取设备信息请求', ['device_id' => $deviceId]);
        
        // 生成缓存键
        $cacheKey = "device_detail:{$deviceId}";
        
        // 尝试从缓存获取
        $cachedDevice = $this->cache->get($cacheKey);
        if ($cachedDevice) {
            $this->logger->info('从缓存获取设备信息', ['device_id' => $deviceId]);
            return ['success' => true, 'device' => $cachedDevice];
        }
        
        $deviceModel = new Device($this->db);
        $device = $deviceModel->getDevice($deviceId);
        
        if ($device) {
            $this->logger->info('获取设备信息成功', ['device_id' => $deviceId]);
            
            // 缓存设备详情，有效期5分钟
            $this->cache->set($cacheKey, $device, 300);
            
            return ['success' => true, 'device' => $device];
        } else {
            $this->logger->warning('设备不存在', ['device_id' => $deviceId]);
            return ['success' => false, 'error' => '设备不存在'];
        }
    }
    
    /**
     * 更新设备状态
     * @param string $deviceId 设备ID
     * @param array $statusInfo 状态信息
     * @return array 更新结果
     */
    public function updateDeviceStatus($deviceId, $statusInfo) {
        $this->logger->info('更新设备状态请求', ['device_id' => $deviceId, 'status' => $statusInfo]);
        
        $deviceModel = new Device($this->db);
        $result = $deviceModel->updateDevice($deviceId, $statusInfo);
        
        if ($result['success']) {
            $this->logger->info('设备状态更新成功', ['device_id' => $deviceId]);
        } else {
            $this->logger->warning('设备状态更新失败', ['device_id' => $deviceId]);
        }
        
        return $result;
    }
    
    /**
     * 获取用户的所有设备
     * @param int $userId 用户ID
     * @return array 设备列表
     */
    public function getUserDevices($userId) {
        $this->logger->info('获取用户设备列表请求', ['user_id' => $userId]);
        
        // 生成缓存键
        $cacheKey = "device_list:user:{$userId}";
        
        // 尝试从缓存获取
        $cachedDevices = $this->cache->get($cacheKey);
        if ($cachedDevices) {
            $this->logger->info('从缓存获取用户设备列表', ['user_id' => $userId, 'count' => count($cachedDevices)]);
            return ['success' => true, 'devices' => $cachedDevices];
        }
        
        $userModel = new User($this->db);
        $devices = $userModel->getUserDevices($userId);
        
        $this->logger->info('获取用户设备列表成功', ['count' => count($devices)]);
        
        // 缓存设备列表，有效期10分钟
        $this->cache->set($cacheKey, $devices, 600);
        
        return ['success' => true, 'devices' => $devices];
    }
    
    /**
     * 绑定设备到用户
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param string $nickname 设备昵称
     * @return array 绑定结果
     */
    public function bindDeviceToUser($userId, $deviceId, $nickname = '') {
        return $this->bindDevice($userId, $deviceId, $nickname);
    }
    
    /**
     * 绑定设备到用户
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param string $nickname 设备昵称
     * @return array 绑定结果
     */
    public function bindDevice($userId, $deviceId, $nickname = '') {
        $this->logger->info('绑定设备请求', ['user_id' => $userId, 'device_id' => $deviceId, 'nickname' => $nickname]);
        
        $userModel = new User($this->db);
        $result = $userModel->bindDevice($userId, $deviceId, $nickname);
        
        if ($result['success']) {
            $this->logger->info('设备绑定成功', ['user_id' => $userId, 'device_id' => $deviceId]);
        } else {
            $this->logger->warning('设备绑定失败', ['user_id' => $userId, 'device_id' => $deviceId, 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 获取所有设备
     * @param array $filters 过滤条件
     * @return array 设备列表
     */
    public function getAllDevices($filters = []) {
        $this->logger->info('获取所有设备请求', ['filters' => $filters]);
        
        $deviceModel = new Device($this->db);
        $devices = $deviceModel->getAllDevices($filters);
        
        $this->logger->info('获取所有设备成功', ['count' => count($devices)]);
        
        return ['success' => true, 'devices' => $devices];
    }
    
    /**
     * 删除设备
     * @param string $deviceId 设备ID
     * @return array 删除结果
     */
    public function deleteDevice($deviceId) {
        $this->logger->info('删除设备请求', ['device_id' => $deviceId]);
        
        $deviceModel = new Device($this->db);
        $result = $deviceModel->deleteDevice($deviceId);
        
        if ($result['success']) {
            $this->logger->info('设备删除成功', ['device_id' => $deviceId]);
        } else {
            $this->logger->warning('设备删除失败', ['device_id' => $deviceId]);
        }
        
        return $result;
    }
    
    /**
     * 解绑设备
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return array 解绑结果
     */
    public function unbindDevice($userId, $deviceId) {
        $this->logger->info('解绑设备请求', ['user_id' => $userId, 'device_id' => $deviceId]);
        
        $userModel = new User($this->db);
        $result = $userModel->unbindDevice($userId, $deviceId);
        
        if ($result['success']) {
            $this->logger->info('设备解绑成功', ['user_id' => $userId, 'device_id' => $deviceId]);
        } else {
            $this->logger->warning('设备解绑失败', ['user_id' => $userId, 'device_id' => $deviceId]);
        }
        
        return $result;
    }
    
    /**
     * 更新设备信息
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @param array $deviceInfo 设备信息
     * @return array 更新结果
     */
    public function updateDevice($userId, $deviceId, $deviceInfo) {
        $this->logger->info('更新设备信息请求', ['user_id' => $userId, 'device_id' => $deviceId, 'info' => $deviceInfo]);
        
        $userModel = new User($this->db);
        
        // 验证设备所有权
        if (!$userModel->isDeviceOwnedByUser($userId, $deviceId)) {
            $this->logger->warning('设备所有权验证失败', ['user_id' => $userId, 'device_id' => $deviceId]);
            return ['success' => false, 'error' => '无权修改该设备'];
        }
        
        // 更新设备信息
        if (isset($deviceInfo['nickname'])) {
            // 更新设备昵称
            $result = $userModel->updateDeviceNickname($userId, $deviceId, $deviceInfo['nickname']);
        } else {
            // 更新设备其他信息
            $deviceModel = new Device($this->db);
            $result = $deviceModel->updateDevice($deviceId, $deviceInfo);
        }
        
        if ($result['success']) {
            $this->logger->info('设备信息更新成功', ['device_id' => $deviceId]);
        } else {
            $this->logger->warning('设备信息更新失败', ['device_id' => $deviceId]);
        }
        
        return $result;
    }
    
    /**
     * 验证设备信息
     * @param array $deviceInfo 设备信息
     * @return bool 验证结果
     */
    private function validateDeviceInfo($deviceInfo) {
        // 验证设备ID
        if (empty($deviceInfo['device_id']) || strlen($deviceInfo['device_id']) < 6) {
            return false;
        }
        
        // 验证设备型号
        if (empty($deviceInfo['model'])) {
            return false;
        }
        
        return true;
    }
    
    /**
     * 获取设备在线状态
     * @param string $deviceId 设备ID
     * @return array 设备状态
     */
    public function getDeviceStatus($deviceId) {
        $this->logger->info('获取设备在线状态请求', ['device_id' => $deviceId]);
        
        // 生成缓存键
        $cacheKey = "device_status:{$deviceId}";
        
        // 尝试从缓存获取
        $cachedStatus = $this->cache->get($cacheKey);
        if ($cachedStatus) {
            $this->logger->info('从缓存获取设备状态', ['device_id' => $deviceId]);
            return ['success' => true, 'status' => $cachedStatus];
        }
        
        $deviceModel = new Device($this->db);
        $device = $deviceModel->getDevice($deviceId);
        
        if (!$device) {
            return ['success' => false, 'error' => '设备不存在'];
        }
        
        // 检查设备是否在线（最后活跃时间在5分钟内）
        $lastActive = strtotime($device['last_active']);
        $now = time();
        $isOnline = ($now - $lastActive) < 300; // 5分钟内活跃为在线
        
        $status = [
            'device_id' => $deviceId,
            'status' => $isOnline ? 'online' : 'offline',
            'last_active' => $device['last_active'],
            'firmware_version' => $device['firmware_version']
        ];
        
        $this->logger->info('获取设备在线状态成功', $status);
        
        // 缓存设备状态，有效期1分钟
        $this->cache->set($cacheKey, $status, 60);
        
        return ['success' => true, 'status' => $status];
    }
    
    /**
     * 获取用户设备数量
     * @param int $userId 用户ID
     * @return int 设备数量
     */
    public function getDeviceCount($userId) {
        try {
            $userModel = new User($this->db);
            $devices = $userModel->getUserDevices($userId);
            return count($devices);
        } catch (\Exception $e) {
            $this->logger->error('获取用户设备数量失败', ['user_id' => $userId, 'error' => $e->getMessage()]);
            return 0;
        }
    }
}