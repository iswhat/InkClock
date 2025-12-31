<?php
/**
 * 设备标签控制器
 */

class DeviceTagController extends BaseController {
    /**
     * 创建标签
     */
    public function createTag($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $result = $tagModel->createTag(
            $data['name'],
            $data['color'] ?? '#3498db',
            $user['id']
        );
        
        if ($result['success']) {
            $this->response::success('标签创建成功', array('tag_id' => $result['tag_id']));
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取用户的标签列表
     */
    public function getTags($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $tags = $tagModel->getTags($user['id']);
        
        $this->response::success('获取成功', $tags);
    }
    
    /**
     * 获取标签详情
     */
    public function getTag($params) {
        $tag_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_get_detail', array('tag_id' => $tag_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $tag = $tagModel->getTag($tag_id);
        
        if (!$tag) {
            $this->response::error('标签不存在', 404);
        }
        
        // 检查标签是否属于当前用户
        if ($tag['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $this->response::success('获取成功', $tag);
    }
    
    /**
     * 更新标签
     */
    public function updateTag($params) {
        $tag_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_update', array('tag_id' => $tag_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $tag = $tagModel->getTag($tag_id);
        
        if (!$tag) {
            $this->response::error('标签不存在', 404);
        }
        
        // 检查标签是否属于当前用户
        if ($tag['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $data = $this->parseRequestBody();
        $result = $tagModel->updateTag(
            $tag_id,
            $data['name'] ?? $tag['name'],
            $data['color'] ?? $tag['color']
        );
        
        if ($result['success']) {
            $this->response::success('标签更新成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 删除标签
     */
    public function deleteTag($params) {
        $tag_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_delete', array('tag_id' => $tag_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $tag = $tagModel->getTag($tag_id);
        
        if (!$tag) {
            $this->response::error('标签不存在', 404);
        }
        
        // 检查标签是否属于当前用户
        if ($tag['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $tagModel->deleteTag($tag_id);
        
        if ($result['success']) {
            $this->response::success('标签删除成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 为设备添加标签
     */
    public function addTagToDevice($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_add_to_device', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 检查设备是否属于当前用户
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        if (!$userModel->isDeviceOwnedByUser($user['id'], $data['device_id'])) {
            $this->response::forbidden();
        }
        
        // 检查标签是否属于当前用户
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $tag = $tagModel->getTag($data['tag_id']);
        if (!$tag || $tag['user_id'] !== $user['id']) {
            $this->response::error('标签不存在或无权限', 404);
        }
        
        $result = $tagModel->addTagToDevice($data['tag_id'], $data['device_id']);
        
        if ($result['success']) {
            $this->response::success('标签添加到设备成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 从设备移除标签
     */
    public function removeTagFromDevice($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_remove_from_device', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 检查设备是否属于当前用户
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        if (!$userModel->isDeviceOwnedByUser($user['id'], $data['device_id'])) {
            $this->response::forbidden();
        }
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $result = $tagModel->removeTagFromDevice($data['tag_id'], $data['device_id']);
        
        if ($result['success']) {
            $this->response::success('标签从设备移除成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 批量为设备添加标签
     */
    public function batchAddTagsToDevices($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_batch_add', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 检查所有设备是否属于当前用户
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        foreach ($data['device_ids'] as $deviceId) {
            if (!$userModel->isDeviceOwnedByUser($user['id'], $deviceId)) {
                $this->response::forbidden();
            }
        }
        
        // 检查所有标签是否属于当前用户
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        foreach ($data['tag_ids'] as $tagId) {
            $tag = $tagModel->getTag($tagId);
            if (!$tag || $tag['user_id'] !== $user['id']) {
                $this->response::error('标签不存在或无权限', 404);
            }
        }
        
        $result = $tagModel->batchAddTagsToDevices($data['tag_ids'], $data['device_ids']);
        
        if ($result['success']) {
            $this->response::success('批量添加标签成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 批量从设备移除标签
     */
    public function batchRemoveTagsFromDevices($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_batch_remove', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        // 检查所有设备是否属于当前用户
        require_once __DIR__ . '/../models/User.php';
        $userModel = new User();
        foreach ($data['device_ids'] as $deviceId) {
            if (!$userModel->isDeviceOwnedByUser($user['id'], $deviceId)) {
                $this->response::forbidden();
            }
        }
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $result = $tagModel->batchRemoveTagsFromDevices($data['tag_ids'], $data['device_ids']);
        
        if ($result['success']) {
            $this->response::success('批量移除标签成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取标签下的设备列表
     */
    public function getDevicesByTag($params) {
        $tag_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('device_tag_get_devices', array('tag_id' => $tag_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $tag = $tagModel->getTag($tag_id);
        
        if (!$tag) {
            $this->response::error('标签不存在', 404);
        }
        
        // 检查标签是否属于当前用户
        if ($tag['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $devices = $tagModel->getDevicesByTag($tag_id);
        
        $this->response::success('获取成功', $devices);
    }
    
    /**
     * 获取设备的标签列表
     */
    public function getTagsByDevice($params) {
        $deviceId = $params['deviceId'];
        $this->checkDevicePermission($deviceId);
        $this->logAction('device_tag_get_by_device', array('device_id' => $deviceId));
        
        require_once __DIR__ . '/../models/DeviceTag.php';
        $tagModel = new DeviceTag();
        $tags = $tagModel->getTagsByDevice($deviceId);
        
        $this->response::success('获取成功', $tags);
    }
}
?>