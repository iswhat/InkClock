<?php
/**
 * 设备分组控制器
 */

class DeviceGroupController extends BaseController {
    /**
     * 创建设备分组
     */
    public function createGroup($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $result = $groupModel->createGroup(
            $data['name'],
            $data['description'] ?? '',
            $user['id']
        );
        
        if ($result['success']) {
            $this->response::success('分组创建成功', array('group_id' => $result['group_id']));
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取用户的设备分组列表
     */
    public function getGroups($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $groups = $groupModel->getGroups($user['id']);
        
        $this->response::success('获取成功', $groups);
    }
    
    /**
     * 获取分组详情
     */
    public function getGroup($params) {
        $group_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_get_detail', array('group_id' => $group_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $group = $groupModel->getGroup($group_id);
        
        if (!$group) {
            $this->response::error('分组不存在', 404);
        }
        
        // 检查分组是否属于当前用户
        if ($group['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $this->response::success('获取成功', $group);
    }
    
    /**
     * 更新分组信息
     */
    public function updateGroup($params) {
        $group_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_update', array('group_id' => $group_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $group = $groupModel->getGroup($group_id);
        
        if (!$group) {
            $this->response::error('分组不存在', 404);
        }
        
        // 检查分组是否属于当前用户
        if ($group['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $data = $this->parseRequestBody();
        $result = $groupModel->updateGroup(
            $group_id,
            $data['name'] ?? $group['name'],
            $data['description'] ?? $group['description']
        );
        
        if ($result['success']) {
            $this->response::success('分组更新成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 删除分组
     */
    public function deleteGroup($params) {
        $group_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_delete', array('group_id' => $group_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $group = $groupModel->getGroup($group_id);
        
        if (!$group) {
            $this->response::error('分组不存在', 404);
        }
        
        // 检查分组是否属于当前用户
        if ($group['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $groupModel->deleteGroup($group_id);
        
        if ($result['success']) {
            $this->response::success('分组删除成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 将设备添加到分组
     */
    public function addDeviceToGroup($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_add_device', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 检查设备是否属于当前用户
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        if (!$userModel->isDeviceOwnedByUser($user['id'], $data['device_id'])) {
            $this->response::forbidden();
        }
        
        // 检查分组是否属于当前用户
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $group = $groupModel->getGroup($data['group_id']);
        if (!$group || $group['user_id'] !== $user['id']) {
            $this->response::error('分组不存在或无权限', 404);
        }
        
        $result = $groupModel->addDeviceToGroup($data['group_id'], $data['device_id']);
        
        if ($result['success']) {
            $this->response::success('设备添加到分组成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 从分组中移除设备
     */
    public function removeDeviceFromGroup($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_remove_device', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 检查分组是否属于当前用户
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $group = $groupModel->getGroup($data['group_id']);
        if (!$group || $group['user_id'] !== $user['id']) {
            $this->response::error('分组不存在或无权限', 404);
        }
        
        $result = $groupModel->removeDeviceFromGroup($data['group_id'], $data['device_id']);
        
        if ($result['success']) {
            $this->response::success('设备从分组移除成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取分组下的设备列表
     */
    public function getDevicesByGroup($params) {
        $group_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_get_devices', array('group_id' => $group_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceGroup.php';
        $groupModel = new DeviceGroup();
        $group = $groupModel->getGroup($group_id);
        
        if (!$group) {
            $this->response::error('分组不存在', 404);
        }
        
        // 检查分组是否属于当前用户
        if ($group['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $devices = $groupModel->getDevicesByGroup($group_id);
        
        $this->response::success('获取成功', $devices);
    }
}
?>