<?php
/**
 * 固件版本模型类
 */
namespace InkClock\Model;

use InkClock\Utils\Database;

class FirmwareVersion {
    private $db;
    
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 添加固件版本
     * @return array 操作结果
     */
    public function addVersion($model, $version, $file_path, $description = '', $changelog = '', $user_id = 0) {
        $createdAt = date('Y-m-d H:i:s');
        
        $sql = "INSERT INTO firmware_versions (version, device_model, file_path, release_notes, is_active, created_at) 
                VALUES (:version, :model, :file_path, :description, 0, :created_at)";
        
        $params = [
            'version' => $version,
            'model' => $model,
            'file_path' => $file_path,
            'description' => $description,
            'created_at' => $createdAt
        ];
        
        $result = $this->db->execute($sql, $params);
        if ($result) {
            return array('success' => true, 'firmware_id' => $this->db->lastInsertId());
        } else {
            return array('success' => false, 'error' => '添加固件版本失败');
        }
    }
    
    /**
     * 获取所有固件版本
     * @param int $limit 限制数量
     * @param int $offset 偏移量
     * @return array 固件版本列表
     */
    public function getAllVersions($limit = 50, $offset = 0) {
        $sql = "SELECT * FROM firmware_versions ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = ['limit' => $limit, 'offset' => $offset];
        return $this->db->query($sql, $params);
    }
    
    /**
     * 根据设备型号获取固件版本列表
     * @param string $model 设备型号
     * @param int $limit 限制数量
     * @param int $offset 偏移量
     * @return array 固件版本列表
     */
    public function getVersionsByModel($model, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM firmware_versions WHERE device_model = :model ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = ['model' => $model, 'limit' => $limit, 'offset' => $offset];
        return $this->db->query($sql, $params);
    }
    
    /**
     * 获取活跃的固件版本
     * @param string $model 设备型号
     * @return array 活跃的固件版本
     */
    public function getActiveVersion($model) {
        $sql = "SELECT * FROM firmware_versions WHERE device_model = :model AND is_active = 1 LIMIT 1";
        $params = ['model' => $model];
        $result = $this->db->query($sql, $params);
        return !empty($result) ? $result[0] : null;
    }
    
    /**
     * 获取指定版本
     * @param int $id 版本ID
     * @return array 固件版本信息
     */
    public function getVersion($id) {
        $sql = "SELECT * FROM firmware_versions WHERE id = :id";
        $params = ['id' => $id];
        $result = $this->db->query($sql, $params);
        return !empty($result) ? $result[0] : null;
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
        $params = array();
        
        if (isset($data['version'])) {
            $updateFields[] = "version = :version";
            $params['version'] = $data['version'];
        }
        if (isset($data['device_model'])) {
            $updateFields[] = "device_model = :device_model";
            $params['device_model'] = $data['device_model'];
        }
        if (isset($data['filename'])) {
            $updateFields[] = "filename = :filename";
            $params['filename'] = $data['filename'];
        }
        if (isset($data['file_path'])) {
            $updateFields[] = "file_path = :file_path";
            $params['file_path'] = $data['file_path'];
        }
        if (isset($data['file_size'])) {
            $updateFields[] = "file_size = :file_size";
            $params['file_size'] = (int)$data['file_size'];
        }
        if (isset($data['sha256'])) {
            $updateFields[] = "sha256 = :sha256";
            $params['sha256'] = $data['sha256'];
        }
        if (isset($data['signature'])) {
            $updateFields[] = "signature = :signature";
            $params['signature'] = $data['signature'];
        }
        if (isset($data['public_key'])) {
            $updateFields[] = "public_key = :public_key";
            $params['public_key'] = $data['public_key'];
        }
        if (isset($data['release_notes'])) {
            $updateFields[] = "release_notes = :release_notes";
            $params['release_notes'] = $data['release_notes'];
        }
        if (isset($data['is_active'])) {
            $updateFields[] = "is_active = :is_active";
            $params['is_active'] = (int)$data['is_active'];
        }
        if (isset($data['is_forced'])) {
            $updateFields[] = "is_forced = :is_forced";
            $params['is_forced'] = (int)$data['is_forced'];
        }
        if (isset($data['published_at'])) {
            $updateFields[] = "published_at = :published_at";
            $params['published_at'] = $data['published_at'];
        }
        
        if (empty($updateFields)) {
            return array('error' => '没有需要更新的字段');
        }
        
        $params['id'] = $id;
        $sql = "UPDATE firmware_versions SET " . implode(', ', $updateFields) . " WHERE id = :id";
        
        $result = $this->db->execute($sql, $params);
        if ($result) {
            // 如果设为活跃版本，将其他版本设为非活跃
            if (isset($data['is_active']) && $data['is_active']) {
                $this->setOnlyActiveVersion($version['device_model'], $id);
            }
            return array('success' => true);
        } else {
            return array('error' => '更新固件版本失败');
        }
    }
    
    /**
     * 删除固件版本
     * @param int $id 版本ID
     * @return array 操作结果
     */
    public function deleteVersion($id) {
        $sql = "DELETE FROM firmware_versions WHERE id = :id";
        $params = ['id' => $id];
        $result = $this->db->execute($sql, $params);
        if ($result) {
            return array('success' => true);
        } else {
            return array('error' => '删除固件版本失败');
        }
    }
    
    /**
     * 设置只有一个活跃版本
     * @param string $model 设备型号
     * @param int $activeId 活跃版本ID
     * @return bool 操作结果
     */
    private function setOnlyActiveVersion($model, $activeId) {
        $sql = "UPDATE firmware_versions SET is_active = CASE WHEN id = :active_id THEN 1 ELSE 0 END WHERE device_model = :model";
        $params = ['active_id' => $activeId, 'model' => $model];
        return $this->db->execute($sql, $params) !== false;
    }
    
    /**
     * 发布固件版本
     * @param int $id 版本ID
     * @return array 操作结果
     */
    public function publishVersion($id) {
        $publishedAt = date('Y-m-d H:i:s');
        $sql = "UPDATE firmware_versions SET is_active = 1, published_at = :published_at WHERE id = :id";
        $params = ['published_at' => $publishedAt, 'id' => $id];
        
        $result = $this->db->execute($sql, $params);
        if ($result) {
            // 获取该版本的设备型号
            $version = $this->getVersion($id);
            if ($version) {
                $this->setOnlyActiveVersion($version['device_model'], $id);
            }
            return array('success' => true);
        } else {
            return array('error' => '发布固件版本失败');
        }
    }
}
?>
