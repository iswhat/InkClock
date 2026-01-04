<?php
/**
 * 设备标签控制器
 */

namespace App\Controller;

use App\Model\DeviceTag;

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
     * 绑定设备到标签
     */
    public function bindDevice($params) {
        $user = $this->checkApiPermission(true);
        $tagId = $params['id'];
        $this->logAction('device_tag_bind_device', array('user_id' => $user['id'], 'tag_id' => $tagId));
        
        $data = $this->parseRequestBody();
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->bindDeviceToTag($tagId, $data['device_id'], $user['id']);
        
        if ($result['success']) {
            $this->response->success('绑定成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 解绑设备标签
     */
    public function unbindDevice($params) {
        $user = $this->checkApiPermission(true);
        $tagId = $params['id'];
        $deviceId = $params['device_id'];
        $this->logAction('device_tag_unbind_device', array('user_id' => $user['id'], 'tag_id' => $tagId, 'device_id' => $deviceId));
        
        $deviceTagModel = new DeviceTag($this->db);
        $result = $deviceTagModel->unbindDeviceFromTag($tagId, $deviceId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('解绑成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
}
?>