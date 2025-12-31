<?php
/**
 * 固件控制器
 */

class FirmwareController extends BaseController {
    /**
     * 添加固件版本
     */
    public function addVersion($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_add', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/FirmwareVersion.php';
        $firmwareModel = new FirmwareVersion();
        $result = $firmwareModel->addVersion(
            $data['model'],
            $data['version'],
            $data['file_path'],
            $data['description'] ?? '',
            $data['changelog'] ?? '',
            $user['id']
        );
        
        if ($result['success']) {
            $this->response::success('固件版本添加成功', array('firmware_id' => $result['firmware_id']));
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取所有固件版本
     */
    public function getAllVersions($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_get_all', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/FirmwareVersion.php';
        $firmwareModel = new FirmwareVersion();
        $versions = $firmwareModel->getAllVersions();
        
        $this->response::success('获取成功', $versions);
    }
    
    /**
     * 获取设备型号的活跃固件版本
     */
    public function getActiveVersion($params) {
        $model = $params['model'];
        $this->logAction('firmware_get_active', array('model' => $model));
        
        require_once __DIR__ . '/../models/FirmwareVersion.php';
        $firmwareModel = new FirmwareVersion();
        $version = $firmwareModel->getActiveVersion($model);
        
        if ($version) {
            $this->response::success('获取成功', $version);
        } else {
            $this->response::error('未找到活跃固件版本', 404);
        }
    }
    
    /**
     * 获取固件版本详情
     */
    public function getVersion($params) {
        $firmwareId = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_get_detail', array('firmware_id' => $firmwareId, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/FirmwareVersion.php';
        $firmwareModel = new FirmwareVersion();
        $version = $firmwareModel->getVersion($firmwareId);
        
        if ($version) {
            $this->response::success('获取成功', $version);
        } else {
            $this->response::error('固件版本不存在', 404);
        }
    }
    
    /**
     * 更新固件版本
     */
    public function updateVersion($params) {
        $user = $this->checkApiPermission(true);
        $firmwareId = $params['id'];
        $this->logAction('firmware_update', array('firmware_id' => $firmwareId, 'user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/FirmwareVersion.php';
        $firmwareModel = new FirmwareVersion();
        $result = $firmwareModel->updateVersion(
            $firmwareId,
            $data['description'] ?? null,
            $data['changelog'] ?? null
        );
        
        if ($result['success']) {
            $this->response::success('固件版本更新成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 发布固件版本
     */
    public function publishVersion($params) {
        $user = $this->checkApiPermission(true);
        $firmwareId = $params['id'];
        $this->logAction('firmware_publish', array('firmware_id' => $firmwareId, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/FirmwareVersion.php';
        $firmwareModel = new FirmwareVersion();
        $result = $firmwareModel->publishVersion($firmwareId);
        
        if ($result['success']) {
            $this->response::success('固件版本发布成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 删除固件版本
     */
    public function deleteVersion($params) {
        $user = $this->checkApiPermission(true);
        $firmwareId = $params['id'];
        $this->logAction('firmware_delete', array('firmware_id' => $firmwareId, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/FirmwareVersion.php';
        $firmwareModel = new FirmwareVersion();
        $result = $firmwareModel->deleteVersion($firmwareId);
        
        if ($result['success']) {
            $this->response::success('固件版本删除成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
}
?>