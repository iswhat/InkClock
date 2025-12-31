<?php
/**
 * 固件版本模型类
 */
require_once __DIR__ . '/../utils/Database.php';

class FirmwareVersion {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
    }
    
    /**
     * 添加固件版本
     * @return array 操作结果
     */
    public function addVersion($model, $version, $file_path, $description = '', $changelog = '', $user_id = 0) {
        // 简化实现，只保存必要字段
        $sql = "INSERT INTO firmware_versions (version, device_model, file_path, release_notes, is_active, created_at) 
                VALUES (?, ?, ?, ?, 0, NOW())";
        
        $stmt = $this->db->prepare($sql);
        if (!$stmt) {
            return array('success' => false, 'error' => 'SQL准备失败: ' . $this->db->error);
        }
        
        $stmt->bind_param('ssss', $version, $model, $file_path, $description);
        
        if ($stmt->execute()) {
            return array('success' => true, 'firmware_id' => $this->db->insert_id);
        } else {
            return array('success' => false, 'error' => '添加固件版本失败: ' . $stmt->error);
        }
    }
    
    /**
     * 获取所有固件版本
     * @param int $limit 限制数量
     * @param int $offset 偏移量
     * @return array 固件版本列表
     */
    public function getAllVersions($limit = 50, $offset = 0) {
        $sql = "SELECT * FROM firmware_versions ORDER BY created_at DESC LIMIT ? OFFSET ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $versions = array();
        while ($row = $result->fetch_assoc()) {
            $versions[] = $row;
        }
        
        return $versions;
    }
    
    /**
     * 根据设备型号获取固件版本列表
     * @param string $model 设备型号
     * @param int $limit 限制数量
     * @param int $offset 偏移量
     * @return array 固件版本列表
     */
    public function getVersionsByModel($model, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM firmware_versions WHERE device_model = ? ORDER BY created_at DESC LIMIT ? OFFSET ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('sii', $model, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $versions = array();
        while ($row = $result->fetch_assoc()) {
            $versions[] = $row;
        }
        
        return $versions;
    }
    
    /**
     * 获取活跃的固件版本
     * @param string $model 设备型号
     * @return array 活跃的固件版本
     */
    public function getActiveVersion($model) {
        $sql = "SELECT * FROM firmware_versions WHERE device_model = ? AND is_active = 1 LIMIT 1";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('s', $model);
        $stmt->execute();
        $result = $stmt->get_result();
        
        return $result->fetch_assoc();
    }
    
    /**
     * 获取指定版本
     * @param int $id 版本ID
     * @return array 固件版本信息
     */
    public function getVersion($id) {
        $sql = "SELECT * FROM firmware_versions WHERE id = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $id);
        $stmt->execute();
        $result = $stmt->get_result();
        
        return $result->fetch_assoc();
    }
    
    /**
     * 更新固件版本
     * @param int $id 版本ID
     * @param array $data 固件版本数据
     * @return array 操作结果
     */
    public function updateVersion($id, $data) {
        $version = $this->getVersion($id);
        if (!$version) {
            return array('error' => '固件版本不存在');
        }
        
        $updateFields = array();
        $updateValues = array();
        
        if (isset($data['version'])) {
            $updateFields[] = "version = ?";
            $updateValues[] = $data['version'];
        }
        if (isset($data['device_model'])) {
            $updateFields[] = "device_model = ?";
            $updateValues[] = $data['device_model'];
        }
        if (isset($data['filename'])) {
            $updateFields[] = "filename = ?";
            $updateValues[] = $data['filename'];
        }
        if (isset($data['file_path'])) {
            $updateFields[] = "file_path = ?";
            $updateValues[] = $data['file_path'];
        }
        if (isset($data['file_size'])) {
            $updateFields[] = "file_size = ?";
            $updateValues[] = $data['file_size'];
        }
        if (isset($data['sha256'])) {
            $updateFields[] = "sha256 = ?";
            $updateValues[] = $data['sha256'];
        }
        if (isset($data['signature'])) {
            $updateFields[] = "signature = ?";
            $updateValues[] = $data['signature'];
        }
        if (isset($data['public_key'])) {
            $updateFields[] = "public_key = ?";
            $updateValues[] = $data['public_key'];
        }
        if (isset($data['release_notes'])) {
            $updateFields[] = "release_notes = ?";
            $updateValues[] = $data['release_notes'];
        }
        if (isset($data['is_active'])) {
            $updateFields[] = "is_active = ?";
            $updateValues[] = (int)$data['is_active'];
        }
        if (isset($data['is_forced'])) {
            $updateFields[] = "is_forced = ?";
            $updateValues[] = (int)$data['is_forced'];
        }
        if (isset($data['published_at'])) {
            $updateFields[] = "published_at = ?";
            $updateValues[] = $data['published_at'];
        }
        
        if (empty($updateFields)) {
            return array('error' => '没有需要更新的字段');
        }
        
        $sql = "UPDATE firmware_versions SET " . implode(', ', $updateFields) . " WHERE id = ?";
        $updateValues[] = $id;
        
        $types = str_repeat('s', count($updateValues) - 1) . 'i';
        $stmt = $this->db->prepare($sql);
        
        // 使用call_user_func_array绑定参数
        $stmt->bind_param($types, ...$updateValues);
        
        if ($stmt->execute()) {
            // 如果设为活跃版本，将其他版本设为非活跃
            if (isset($data['is_active']) && $data['is_active']) {
                $this->setOnlyActiveVersion($version['device_model'], $id);
            }
            
            return array('success' => true);
        } else {
            return array('error' => '更新固件版本失败: ' . $stmt->error);
        }
    }
    
    /**
     * 删除固件版本
     * @param int $id 版本ID
     * @return array 操作结果
     */
    public function deleteVersion($id) {
        $sql = "DELETE FROM firmware_versions WHERE id = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $id);
        
        if ($stmt->execute()) {
            return array('success' => true);
        } else {
            return array('error' => '删除固件版本失败: ' . $stmt->error);
        }
    }
    
    /**
     * 设置只有一个活跃版本
     * @param string $model 设备型号
     * @param int $activeId 活跃版本ID
     * @return bool 操作结果
     */
    private function setOnlyActiveVersion($model, $activeId) {
        $sql = "UPDATE firmware_versions SET is_active = CASE WHEN id = ? THEN 1 ELSE 0 END WHERE device_model = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('is', $activeId, $model);
        return $stmt->execute();
    }
    
    /**
     * 发布固件版本
     * @param int $id 版本ID
     * @return array 操作结果
     */
    public function publishVersion($id) {
        $sql = "UPDATE firmware_versions SET is_active = 1, published_at = NOW() WHERE id = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $id);
        
        if ($stmt->execute()) {
            // 获取该版本的设备型号
            $version = $this->getVersion($id);
            if ($version) {
                $this->setOnlyActiveVersion($version['device_model'], $id);
            }
            return array('success' => true);
        } else {
            return array('error' => '发布固件版本失败: ' . $stmt->error);
        }
    }
}
