<?php
/**
 * 设备标签模型
 */
require_once __DIR__ . '/../utils/Database.php';

class DeviceTag {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
    }
    
    /**
     * 创建标签
     */
    public function createTag($userId, $name, $color = '#3498db') {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO device_tags (name, color, user_id, created_at) VALUES (?, ?, ?, ?)");
        $stmt->bind_param("ssis", $name, $color, $userId, $createdAt);
        $stmt->execute();
        
        $tagId = $this->db->insert_id;
        $stmt->close();
        
        return array(
            'success' => true,
            'tag_id' => $tagId,
            'created_at' => $createdAt
        );
    }
    
    /**
     * 获取用户的标签列表
     */
    public function getTagsByUserId($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM device_tags WHERE user_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bind_param("iii", $userId, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $tags = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
        // 为每个标签添加设备数量
        foreach ($tags as &$tag) {
            $tag['device_count'] = $this->getDeviceCountByTagId($tag['id']);
        }
        
        return $tags;
    }
    
    /**
     * 获取标签的设备数量
     */
    public function getDeviceCountByTagId($tagId) {
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM device_tag_relations WHERE tag_id = ?");
        $stmt->bind_param("i", $tagId);
        $stmt->execute();
        $result = $stmt->get_result();
        $count = $result->fetch_assoc()['count'];
        $stmt->close();
        return $count;
    }
    
    /**
     * 获取标签详情
     */
    public function getTagById($tagId, $userId = null) {
        $query = "SELECT * FROM device_tags WHERE id = ?";
        $params = array($tagId);
        $types = "i";
        
        if ($userId) {
            $query .= " AND user_id = ?";
            $params[] = $userId;
            $types .= "i";
        }
        
        $stmt = $this->db->prepare($query);
        $stmt->bind_param($types, ...$params);
        $stmt->execute();
        $result = $stmt->get_result();
        $tag = $result->fetch_assoc();
        $stmt->close();
        
        if ($tag) {
            $tag['device_count'] = $this->getDeviceCountByTagId($tag['id']);
        }
        
        return $tag;
    }
    
    /**
     * 更新标签信息
     */
    public function updateTag($tagId, $userId, $name, $color = '#3498db') {
        $stmt = $this->db->prepare("UPDATE device_tags SET name = ?, color = ? WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ssii", $name, $color, $tagId, $userId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 删除标签
     */
    public function deleteTag($tagId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM device_tags WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ii", $tagId, $userId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 给设备添加标签
     */
    public function addTagToDevice($tagId, $deviceId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT IGNORE INTO device_tag_relations (tag_id, device_id, created_at) VALUES (?, ?, ?)");
        $stmt->bind_param("iss", $tagId, $deviceId, $createdAt);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 从设备移除标签
     */
    public function removeTagFromDevice($tagId, $deviceId) {
        $stmt = $this->db->prepare("DELETE FROM device_tag_relations WHERE tag_id = ? AND device_id = ?");
        $stmt->bind_param("is", $tagId, $deviceId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 获取设备的标签列表
     */
    public function getTagsByDeviceId($deviceId) {
        $query = "SELECT t.* FROM device_tags t ".
                 "JOIN device_tag_relations r ON t.id = r.tag_id ".
                 "WHERE r.device_id = ? ".
                 "ORDER BY r.created_at DESC";
        
        $stmt = $this->db->prepare($query);
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $tags = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
        return $tags;
    }
    
    /**
     * 获取标签的设备列表
     */
    public function getDevicesByTagId($tagId, $limit = 50, $offset = 0) {
        $query = "SELECT d.* FROM devices d ".
                 "JOIN device_tag_relations r ON d.device_id = r.device_id ".
                 "WHERE r.tag_id = ? ".
                 "ORDER BY r.created_at DESC LIMIT ? OFFSET ?";
        
        $stmt = $this->db->prepare($query);
        $stmt->bind_param("iii", $tagId, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $devices = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
        return $devices;
    }
    
    /**
     * 批量给设备添加标签
     */
    public function batchAddTagsToDevices($tagId, $deviceIds) {
        $successCount = 0;
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT IGNORE INTO device_tag_relations (tag_id, device_id, created_at) VALUES (?, ?, ?)");
        
        foreach ($deviceIds as $deviceId) {
            $stmt->bind_param("iss", $tagId, $deviceId, $createdAt);
            if ($stmt->execute()) {
                if ($stmt->affected_rows > 0) {
                    $successCount++;
                }
            }
        }
        
        $stmt->close();
        
        return array(
            'success' => true,
            'success_count' => $successCount,
            'total_count' => count($deviceIds)
        );
    }
    
    /**
     * 批量从设备移除标签
     */
    public function batchRemoveTagsFromDevices($tagId, $deviceIds) {
        $successCount = 0;
        
        $stmt = $this->db->prepare("DELETE FROM device_tag_relations WHERE tag_id = ? AND device_id = ?");
        
        foreach ($deviceIds as $deviceId) {
            $stmt->bind_param("is", $tagId, $deviceId);
            if ($stmt->execute()) {
                if ($stmt->affected_rows > 0) {
                    $successCount++;
                }
            }
        }
        
        $stmt->close();
        
        return array(
            'success' => true,
            'success_count' => $successCount,
            'total_count' => count($deviceIds)
        );
    }
}
?>