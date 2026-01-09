<?php
/**
 * 设备控制器
 */

namespace InkClock\Controller;

use InkClock\Model\Device;
use InkClock\Model\User;

class DeviceController extends BaseController {
    /**
     * 注册设备
     */
    public function registerDevice($params) {
        $this->logAction('device_register');
        $data = $this->parseRequestBody();
        
        // 验证输入
        if (!isset($data['device_id']) || !isset($data['model']) || !isset($data['firmware_version'])) {
            $this->response->error('缺少必要参数', 400);
        }
        
        if (empty($data['device_id']) || empty($data['model']) || empty($data['firmware_version'])) {
            $this->response->error('参数不能为空', 400);
        }
        
        // 验证设备ID格式（假设设备ID是字母数字组合）
        if (!preg_match('/^[a-zA-Z0-9_-]+$/', $data['device_id'])) {
            $this->response->error('设备ID格式无效', 400);
        }
        
        $deviceModel = new Device($this->db);
        $result = $deviceModel->registerDevice($data['device_id'], $data['model'], $data['firmware_version']);
        
        if ($result['success']) {
            $this->response->success('设备注册成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 获取设备列表
     */
    public function getDevices($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_get_list', array('user_id' => $user['id']));
        
        $userModel = new User($this->db);
        $devices = $userModel->getUserDevices($user['id']);
        
        $this->response->success('获取成功', $devices);
    }
    
    /**
     * 获取设备详情
     */
    public function getDevice($params) {
        $deviceId = $params['id'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('device_get_detail', array('device_id' => $deviceId));
        
        $deviceModel = new Device($this->db);
        $device = $deviceModel->getDevice($deviceId);
        
        if ($device) {
            $this->response->success('获取成功', $device);
        } else {
            $this->response->error('设备不存在', 404);
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
        $userModel = new User($this->db);
        if (!$userModel->isDeviceOwnedByUser($user['id'], $deviceId)) {
            $this->response->forbidden();
        }
        
        // 先解绑设备
        $userModel->unbindDevice($user['id'], $deviceId);
        
        // 删除设备（如果没有其他用户绑定）
        $deviceModel = new Device($this->db);
        $deviceModel->deleteDevice($deviceId);
        
        $this->response->success('设备删除成功');
    }
    
    /**
     * 刷新设备
     */
    public function refreshDevice($params) {
        $this->logAction('device_refresh');
        
        return [
            'success' => true,
            'message' => '设备刷新成功',
            'timestamp' => time()
        ];
    }
}
?>