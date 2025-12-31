<?php
/**
 * 设备控制器
 */

class DeviceController extends BaseController {
    /**
     * 注册设备
     */
    public function registerDevice($params) {
        $this->logAction('device_register');
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/Device.php';
        $deviceModel = new Device();
        $result = $deviceModel->registerDevice($data['device_id'], $data['model'], $data['firmware_version']);
        
        if ($result['success']) {
            $this->response::success('设备注册成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取设备列表
     */
    public function getDevices($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        $devices = $userModel->getUserDevices($user['id']);
        
        $this->response::success('获取成功', $devices);
    }
    
    /**
     * 获取设备详情
     */
    public function getDevice($params) {
        $deviceId = $params['id'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('device_get_detail', array('device_id' => $deviceId));
        
        require_once __DIR__ . '/../models/Device.php';
        $deviceModel = new Device();
        $device = $deviceModel->getDevice($deviceId);
        
        if ($device) {
            $this->response::success('获取成功', $device);
        } else {
            $this->response::error('设备不存在', 404);
        }
    }
    
    /**
     * 删除设备
     */
    public function deleteDevice($params) {
        $user = $this->checkApiPermission(true);
        $deviceId = $params['id'];
        $this->logAction('device_delete', array('user_id' => $user['id'], 'device_id' => $deviceId));
        
        // 验证用户是否拥有该设备
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        if (!$userModel->isDeviceOwnedByUser($user['id'], $deviceId)) {
            $this->response::forbidden();
        }
        
        // 先解绑设备
        $userModel->unbindDevice($user['id'], $deviceId);
        
        // 删除设备（如果没有其他用户绑定）
        require_once __DIR__ . '/../models/Device.php';
        $deviceModel = new Device();
        $deviceModel->deleteDevice($deviceId);
        
        $this->response::success('设备删除成功');
    }
}
?>