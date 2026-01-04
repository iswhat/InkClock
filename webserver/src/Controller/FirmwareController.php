<?php
/**
 * 固件控制器
 */

namespace InkClock\Controller;

use InkClock\Model\FirmwareVersion;

class FirmwareController extends BaseController {
    /**
     * 获取固件版本列表
     */
    public function getVersions($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_get_versions', array('user_id' => $user['id']));
        
        $model = isset($_GET['model']) ? $_GET['model'] : null;
        $limit = isset($_GET['limit']) ? (int)$_GET['limit'] : 50;
        $offset = isset($_GET['offset']) ? (int)$_GET['offset'] : 0;
        
        $firmwareModel = new FirmwareVersion($this->db);
        $versions = $model ? $firmwareModel->getVersionsByModel($model, $limit, $offset) : $firmwareModel->getAllVersions($limit, $offset);
        
        $this->response->success('获取成功', $versions);
    }
    
    /**
     * 获取活跃固件版本
     */
    public function getActiveVersion($params) {
        $this->logAction('firmware_get_active_version');
        
        $model = $params['model'];
        
        $firmwareModel = new FirmwareVersion($this->db);
        $version = $firmwareModel->getActiveVersion($model);
        
        if ($version) {
            $this->response->success('获取成功', $version);
        } else {
            $this->response->error('没有找到活跃的固件版本', 404);
        }
    }
    
    /**
     * 获取固件版本详情
     */
    public function getVersion($params) {
        $user = $this->checkApiPermission(true);
        $id = $params['id'];
        $this->logAction('firmware_get_version', array('user_id' => $user['id'], 'version_id' => $id));
        
        $firmwareModel = new FirmwareVersion($this->db);
        $version = $firmwareModel->getVersion($id);
        
        if ($version) {
            $this->response->success('获取成功', $version);
        } else {
            $this->response->error('固件版本不存在', 404);
        }
    }
    
    /**
     * 添加固件版本
     */
    public function addVersion($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_add_version', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        $firmwareModel = new FirmwareVersion($this->db);
        $result = $firmwareModel->addVersion($data['model'], $data['version'], $data['file_path'], $data['description'], $data['changelog'], $user['id']);
        
        if ($result['success']) {
            $this->response->success('添加成功', array('firmware_id' => $result['firmware_id']));
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 更新固件版本
     */
    public function updateVersion($params) {
        $user = $this->checkApiPermission(true);
        $id = $params['id'];
        $this->logAction('firmware_update_version', array('user_id' => $user['id'], 'version_id' => $id));
        
        $data = $this->parseRequestBody();
        
        $firmwareModel = new FirmwareVersion($this->db);
        $result = $firmwareModel->updateVersion($id, $data);
        
        if ($result['success']) {
            $this->response->success('更新成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 删除固件版本
     */
    public function deleteVersion($params) {
        $user = $this->checkApiPermission(true);
        $id = $params['id'];
        $this->logAction('firmware_delete_version', array('user_id' => $user['id'], 'version_id' => $id));
        
        $firmwareModel = new FirmwareVersion($this->db);
        $result = $firmwareModel->deleteVersion($id);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 发布固件版本
     */
    public function publishVersion($params) {
        $user = $this->checkApiPermission(true);
        $id = $params['id'];
        $this->logAction('firmware_publish_version', array('user_id' => $user['id'], 'version_id' => $id));
        
        $firmwareModel = new FirmwareVersion($this->db);
        $result = $firmwareModel->publishVersion($id);
        
        if ($result['success']) {
            $this->response->success('发布成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
}
?>