<?php
/**
 * 设备分组控制器
 */

namespace App\Controller;

use App\Model\DeviceGroup;

class DeviceGroupController extends BaseController {
    /**
     * 获取设备分组列表
     */
    public function getGroups() {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_get_list', array('user_id' => $user['id']));
        
        $deviceGroupModel = new DeviceGroup($this->db);
        $groups = $deviceGroupModel->getGroupsByUserId($user['id']);
        
        $this->response->success('获取成功', $groups);
    }
    
    /**
     * 创建设备分组
     */
    public function createGroup() {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_group_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        $deviceGroupModel = new DeviceGroup($this->db);
        $result = $deviceGroupModel->createGroup($data['name'], $user['id']);
        
        if ($result['success']) {
            $this->response->success('创建成功', array('group_id' => $result['group_id']));
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 更新设备分组
     */
    public function updateGroup($params) {
        $user = $this->checkApiPermission(true);
        $groupId = $params['id'];
        $this->logAction('device_group_update', array('user_id' => $user['id'], 'group_id' => $groupId));
        
        $data = $this->parseRequestBody();
        
        $deviceGroupModel = new DeviceGroup($this->db);
        $result = $deviceGroupModel->updateGroup($groupId, $user['id'], $data['name']);
        
        if ($result['success']) {
            $this->response->success('更新成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 删除设备分组
     */
    public function deleteGroup($params) {
        $user = $this->checkApiPermission(true);
        $groupId = $params['id'];
        $this->logAction('device_group_delete', array('user_id' => $user['id'], 'group_id' => $groupId));
        
        $deviceGroupModel = new DeviceGroup($this->db);
        $result = $deviceGroupModel->deleteGroup($groupId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 添加设备到分组
     */
    public function addDeviceToGroup($params) {
        $user = $this->checkApiPermission(true);
        $groupId = $params['id'];
        $this->logAction('device_group_add_device', array('user_id' => $user['id'], 'group_id' => $groupId));
        
        $data = $this->parseRequestBody();
        
        $deviceGroupModel = new DeviceGroup($this->db);
        $result = $deviceGroupModel->addDeviceToGroup($groupId, $user['id'], $data['device_id']);
        
        if ($result['success']) {
            $this->response->success('添加成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 从分组中移除设备
     */
    public function removeDeviceFromGroup($params) {
        $user = $this->checkApiPermission(true);
        $groupId = $params['id'];
        $deviceId = $params['device_id'];
        $this->logAction('device_group_remove_device', array('user_id' => $user['id'], 'group_id' => $groupId, 'device_id' => $deviceId));
        
        $deviceGroupModel = new DeviceGroup($this->db);
        $result = $deviceGroupModel->removeDeviceFromGroup($groupId, $user['id'], $deviceId);
        
        if ($result['success']) {
            $this->response->success('移除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
}
?>