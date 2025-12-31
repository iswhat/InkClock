<?php
/**
 * 设备服务类
 */
require_once __DIR__ . '/../models/Device.php';
require_once __DIR__ . '/../models/User.php';
require_once __DIR__ . '/../utils/Logger.php';

class DeviceService {
    private $deviceModel;
    private $userModel;
    private $logger;
    
    public function __construct() {
        $this->deviceModel = new Device();
        $this->userModel = new User();
        $this->logger = Logger::getInstance();
    }
    
    /**
     * 注册新设备
     */
    public function registerDevice($deviceInfo) {
        $this->logger->info('设备注册请求', $deviceInfo);
        
        // 验证设备信息
        if (!$this->validateDeviceInfo($deviceInfo)) {
            $this->logger->warning('设备注册信息验证失败', $deviceInfo);
            return ['success' => false, 'error' => '无效的设备信息'];
        }
        
        // 调用模型进行注册
        $result = $this->deviceModel->registerDevice($deviceInfo);
        
        if ($result['success']) {
            $this->logger->info('设备注册成功', ['device_id' => $deviceInfo['device_id']]);
        } else {
            $this->logger->warning('设备注册失败', ['device_id' => $deviceInfo['device_id'], 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 获取设备列表
     */
    public function getDeviceList($userId, $filters = []) {
        $this->logger->info('获取设备列表请求', ['user_id' => $userId, 'filters' => $filters]);
        
        // 普通用户只能查看自己的设备
        $devices = $this->userModel->getUserDevices($userId);
        
        $this->logger->info('获取设备列表成功', ['count' => count($devices)]);
        
        return ['success' => true, 'devices' => $devices];
    }
    
    /**
     * 获取设备详情
     */
    public function getDeviceDetail($userId, $deviceId) {
        $this->logger->info('获取设备详情请求', ['user_id' => $userId, 'device_id' => $deviceId]);
        
        // 验证设备所有权
        if (!$this->userModel->isDeviceOwnedByUser($userId, $deviceId)) {
            $this->logger->warning('设备所有权验证失败', ['user_id' => $userId, 'device_id' => $deviceId]);
            return ['success' => false, 'error' => '无权访问该设备'];
        }
        
        $device = $this->deviceModel->getDevice($deviceId);
        
        if ($device) {
            $this->logger->info('获取设备详情成功', ['device_id' => $deviceId]);
            return ['success' => true, 'device' => $device];
        } else {
            $this->logger->warning('设备不存在', ['device_id' => $deviceId]);
            return ['success' => false, 'error' => '设备不存在'];
        }
    }
    
    /**
     * 绑定设备到用户
     */
    public function bindDevice($userId, $deviceId, $nickname = '') {
        $this->logger->info('绑定设备请求', ['user_id' => $userId, 'device_id' => $deviceId, 'nickname' => $nickname]);
        
        $result = $this->userModel->bindDevice($userId, $deviceId, $nickname);
        
        if ($result['success']) {
            $this->logger->info('设备绑定成功', ['user_id' => $userId, 'device_id' => $deviceId]);
        } else {
            $this->logger->warning('设备绑定失败', ['user_id' => $userId, 'device_id' => $deviceId, 'error' => $result['error']]);
        }
        
        return $result;
    }
    
    /**
     * 解绑设备
     */
    public function unbindDevice($userId, $deviceId) {
        $this->logger->info('解绑设备请求', ['user_id' => $userId, 'device_id' => $deviceId]);
        
        $result = $this->userModel->unbindDevice($userId, $deviceId);
        
        if ($result['success']) {
            $this->logger->info('设备解绑成功', ['user_id' => $userId, 'device_id' => $deviceId]);
        } else {
            $this->logger->warning('设备解绑失败', ['user_id' => $userId, 'device_id' => $deviceId]);
        }
        
        return $result;
    }
    
    /**
     * 更新设备信息
     */
    public function updateDevice($userId, $deviceId, $deviceInfo) {
        $this->logger->info('更新设备信息请求', ['user_id' => $userId, 'device_id' => $deviceId, 'info' => $deviceInfo]);
        
        // 验证设备所有权
        if (!$this->userModel->isDeviceOwnedByUser($userId, $deviceId)) {
            $this->logger->warning('设备所有权验证失败', ['user_id' => $userId, 'device_id' => $deviceId]);
            return ['success' => false, 'error' => '无权修改该设备'];
        }
        
        // 更新设备昵称
        if (isset($deviceInfo['nickname'])) {
            $result = $this->userModel->updateDeviceNickname($userId, $deviceId, $deviceInfo['nickname']);
        } else {
            // 更新设备其他信息
            $result = $this->deviceModel->updateDevice($deviceId, $deviceInfo);
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
     */
    public function getDeviceStatus($deviceId) {
        $this->logger->info('获取设备在线状态请求', ['device_id' => $deviceId]);
        
        $device = $this->deviceModel->getDevice($deviceId);
        
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
        
        return ['success' => true, 'status' => $status];
    }
}
?>