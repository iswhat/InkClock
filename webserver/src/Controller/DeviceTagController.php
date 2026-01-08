<?php
/**
 * 设备标签控制器
 */

namespace InkClock\Controller;

use InkClock\Model\DeviceTag;

class DeviceTagController extends BaseController {
    /**
     * 获取设备标签列表
     */
    public function getTags($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_get_list', array('user_id' => $user['id']));
        
        $deviceTagModel = new DeviceTag($this->db);
        $tags = $deviceTagModel->getTagsByUserId($user['id']);
        
        $this->response->success('获取成功', $tags);
    }
    
    /**
     * 创建设备标签
     */
    public function createTag($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->createTag($data['name'], $data['color'] ?? '#3498db', $user['id']);
        
        if ($result['success']) {
            $this->response->success('创建成功', array('tag_id' => $result['tag_id']));
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 更新设备标签
     */
    public function updateTag($params) {
        $user = $this->checkApiPermission(true);
        $tagId = $params['id'];
        $this->logAction('device_tag_update', array('user_id' => $user['id'], 'tag_id' => $tagId));
        
        $data = $this->parseRequestBody();
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->updateTag($tagId, $user['id'], $data['name'], $data['color']);
        
        if ($result['success']) {
            $this->response->success('更新成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 删除设备标签
     */
    public function deleteTag($params) {
        $user = $this->checkApiPermission(true);
        $tagId = $params['id'];
        $this->logAction('device_tag_delete', array('user_id' => $user['id'], 'tag_id' => $tagId));
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->deleteTag($tagId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 添加设备到标签
     */
    public function addTagToDevice($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        $tagId = $data['tag_id'];
        $deviceId = $data['device_id'];
        $this->logAction('device_tag_add_device', array('user_id' => $user['id'], 'tag_id' => $tagId, 'device_id' => $deviceId));
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->addTagToDevice($tagId, $deviceId);
        
        if ($result['success']) {
            $this->response->success('添加成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 从标签中移除设备
     */
    public function removeTagFromDevice($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        $tagId = $data['tag_id'];
        $deviceId = $data['device_id'];
        $this->logAction('device_tag_remove_device', array('user_id' => $user['id'], 'tag_id' => $tagId, 'device_id' => $deviceId));
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->removeTagFromDevice($tagId, $deviceId);
        
        if ($result['success']) {
            $this->response->success('移除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 批量添加标签到设备
     */
    public function batchAddTagsToDevices($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        $tagIds = $data['tag_ids'];
        $deviceIds = $data['device_ids'];
        $this->logAction('device_tag_batch_add', array('user_id' => $user['id'], 'tag_ids' => implode(',', $tagIds), 'device_ids' => implode(',', $deviceIds)));
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->batchAddTagsToDevices($tagIds, $deviceIds);
        
        if ($result['success']) {
            $this->response->success('批量添加成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 批量从设备中移除标签
     */
    public function batchRemoveTagsFromDevices($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        $tagIds = $data['tag_ids'];
        $deviceIds = $data['device_ids'];
        $this->logAction('device_tag_batch_remove', array('user_id' => $user['id'], 'tag_ids' => implode(',', $tagIds), 'device_ids' => implode(',', $deviceIds)));
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->batchRemoveTagsFromDevices($tagIds, $deviceIds);
        
        if ($result['success']) {
            $this->response->success('批量移除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 获取标签下的设备
     */
    public function getDevicesByTag($params) {
        $user = $this->checkApiPermission(true);
        $tagId = $params['id'];
        $this->logAction('device_tag_get_devices', array('user_id' => $user['id'], 'tag_id' => $tagId));
        
        $deviceTagModel = new DeviceTag($this->db);
        $devices = $deviceTagModel->getDevicesByTagId($tagId);
        
        $this->response->success('获取成功', $devices);
    }
    
    /**
     * 获取设备的标签
     */
    public function getTagsByDevice($params) {
        $user = $this->checkApiPermission(true);
        $deviceId = $params['deviceId'];
        $this->logAction('device_tag_get_tags', array('user_id' => $user['id'], 'device_id' => $deviceId));
        
        $deviceTagModel = new DeviceTag($this->db);
        $tags = $deviceTagModel->getTagsByDeviceId($deviceId);
        
        $this->response->success('获取成功', $tags);
    }
}
?>