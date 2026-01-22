<?php
/**
 * 设备控制器
 */

namespace InkClock\Controller;

use InkClock\Model\Device;
use InkClock\Model\User;

class DeviceController extends BaseController {
    /**
     * 注册设备（支持两种模式：设备自注册和管理中心添加）
     */
    public function registerDevice($params) {
        $this->logAction('device_register');
        $data = $this->parseRequestBody();
        
        // 检查是否有API密钥，区分设备自注册和管理中心添加
        $hasApiKey = isset($_GET['api_key']) || isset($_POST['api_key']);
        $user = null;
        
        if ($hasApiKey) {
            // 管理中心添加设备模式
            // 验证用户权限，获取当前用户
            $user = $this->checkApiPermission(true);
            
            // 验证输入
            if (!isset($data['device_id']) || !isset($data['device_name'])) {
                $this->response->error('缺少必要参数', 400);
            }
            
            if (empty($data['device_id']) || empty($data['device_name'])) {
                $this->response->error('参数不能为空', 400);
            }
            
            // 使用device_name作为model，设置默认固件版本
            $model = $data['device_name'];
            $firmwareVersion = 'unknown';
        } else {
            // 设备自注册模式
            // 验证输入
            if (!isset($data['device_id']) || !isset($data['model']) || !isset($data['firmware_version'])) {
                $this->response->error('缺少必要参数', 400);
            }
            
            if (empty($data['device_id']) || empty($data['model']) || empty($data['firmware_version'])) {
                $this->response->error('参数不能为空', 400);
            }
            
            $model = $data['model'];
            $firmwareVersion = $data['firmware_version'];
        }
        
        // 验证设备ID格式（假设设备ID是字母数字组合）
        if (!preg_match('/^[a-zA-Z0-9_-]+$/', $data['device_id'])) {
            $this->response->error('设备ID格式无效', 400);
        }
        
        // 获取可选参数
        $macAddress = $data['mac_address'] ?? '';
        $extraInfo = $data['extra_info'] ?? [];
        
        // 如果有description参数，添加到extraInfo中
        if (isset($data['description'])) {
            $extraInfo['description'] = $data['description'];
        }
        
        $deviceModel = new Device($this->db);
        $result = $deviceModel->registerDevice($data['device_id'], $model, $firmwareVersion, $macAddress, $extraInfo);
        
        if ($result['success']) {
            // 如果是管理中心添加设备，自动绑定到当前用户
            if ($hasApiKey) {
                $userModel = new User($this->db);
                // 绑定设备到当前用户
                $bindResult = $userModel->bindDevice($user['id'], $data['device_id'], $data['device_name'] ?? '');
                if (!$bindResult['success'] && strpos($bindResult['error'], '已绑定') === false) {
                    // 绑定失败且不是因为已绑定，记录警告
                    $this->logAction('device_bind_warning', array('user_id' => $user['id'], 'device_id' => $data['device_id'], 'error' => $bindResult['error']));
                }
            }
            
            // 清除设备相关的缓存
            $this->cache->flushByTags(['devices']);
            $this->response->success($result['message'], $result['device_info']);
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
        
        // 尝试从缓存获取
        $cacheKey = 'devices:user:' . $user['id'];
        $cachedDevices = $this->cache->get($cacheKey);
        
        if ($cachedDevices) {
            $this->response->success('获取成功（缓存）', $cachedDevices);
            return;
        }
        
        $userModel = new User($this->db);
        $devices = $userModel->getUserDevices($user['id']);
        
        // 缓存结果，10分钟过期
        $this->cache->set($cacheKey, $devices, 600, ['devices', 'user:' . $user['id']]);
        
        $this->response->success('获取成功', $devices);
    }
    
    /**
     * 获取设备详情
     */
    public function getDevice($params) {
        $deviceId = $params['id'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('device_get_detail', array('device_id' => $deviceId));
        
        // 尝试从缓存获取
        $cacheKey = 'device:detail:' . $deviceId;
        $cachedDevice = $this->cache->get($cacheKey);
        
        if ($cachedDevice) {
            $this->response->success('获取成功（缓存）', $cachedDevice);
            return;
        }
        
        $deviceModel = new Device($this->db);
        $device = $deviceModel->getDevice($deviceId);
        
        if ($device) {
            // 缓存结果，5分钟过期
            $this->cache->set($cacheKey, $device, 300, ['devices', 'device:' . $deviceId]);
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
        
        // 清除设备相关的缓存
        $this->cache->flushByTags(['devices']);
        
        $this->response->success('设备删除成功');
    }
    
    /**
     * 添加设备（管理中心）
     */
    public function addDevice($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        $this->logAction('device_add', array('user_id' => $user['id'], 'device_data' => $data));
        
        // 验证输入
        if (!isset($data['device_id']) || !isset($data['device_name'])) {
            $this->response->error('缺少必要参数', 400);
        }
        
        if (empty($data['device_id']) || empty($data['device_name'])) {
            $this->response->error('参数不能为空', 400);
        }
        
        // 使用设备服务层添加设备
        $deviceModel = new Device($this->db);
        $result = $deviceModel->registerDevice(
            $data['device_id'],
            $data['device_name'],  // 使用device_name作为model
            'unknown',  // 默认固件版本
            $data['mac_address'] ?? '',
            ['description' => $data['description'] ?? '']
        );
        
        if ($result['success']) {
            // 如果有group_id，绑定设备到分组
            // 这里可以添加分组绑定逻辑
            
            $this->response->success('设备添加成功', $result['device_info']);
        } else {
            $this->response->error($result['error'], 400);
        }
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
    
    /**
     * 更新设备状态
     */
    public function updateDeviceStatus($params) {
        $this->logAction('device_update_status');
        $data = $this->parseRequestBody();
        
        // 验证必要参数
        if (!isset($data['device_id']) || !isset($data['connection_status'])) {
            $this->response->error('缺少必要参数', 400);
        }
        
        $deviceId = $data['device_id'];
        $connectionStatus = $data['connection_status'];
        
        // 更新设备状态
        $deviceModel = new Device($this->db);
        $stmt = $this->db->prepare("UPDATE devices SET connection_status = :connection_status, last_active = :last_active WHERE device_id = :device_id");
        $stmt->bindValue(':connection_status', $connectionStatus, SQLITE3_TEXT);
        $stmt->bindValue(':last_active', date('Y-m-d H:i:s'), SQLITE3_TEXT);
        $stmt->bindValue(':device_id', $deviceId, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        
        if ($result) {
            $this->response->success('设备状态更新成功');
        } else {
            $this->response->error('设备状态更新失败', 500);
        }
    }
}
?>