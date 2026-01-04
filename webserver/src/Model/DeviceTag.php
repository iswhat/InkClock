<?php
/**
 * 设备标签模型
 */

namespace InkClock\Model;

class DeviceTag {
    private $db;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     */
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 创建标签
     */
    public function createTag($userId, $name, $color = '#3498db') {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO device_tags (name, color, user_id, created_at) VALUES (:name, :color, :userId, :createdAt)");
        $stmt->bindValue(':name', $name, SQLITE3_TEXT);
        $stmt->bindValue(':color', $color, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $tagId = $this->db->lastInsertRowID();
        
        return array(
            'success' => $result !== false,
            'tag_id' => $tagId,
            'created_at' => $createdAt
        );
    }
    
    /**
     * 获取用户的标签列表
     */
    public function getTagsByUserId($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM device_tags WHERE user_id = :userId ORDER BY created_at DESC LIMIT :limit OFFSET :offset");
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':limit', $limit, SQLITE3_INTEGER);
        $stmt->bindValue(':offset', $offset, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $tags = array();
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $tags[] = $row;
        }
        
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
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM device_tag_relations WHERE tag_id = :tagId");
        $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $row = $result->fetchArray(SQLITE3_ASSOC);
        
        return $row['count'];
    }
    
    /**
     * 获取标签详情
     */
    public function getTagById($tagId, $userId = null) {
        $query = "SELECT * FROM device_tags WHERE id = :tagId";
        $params = array(':tagId' => $tagId);
        
        if ($userId) {
            $query .= " AND user_id = :userId";
            $params[':userId'] = $userId;
        }
        
        $stmt = $this->db->prepare($query);
        
        foreach ($params as $key => $value) {
            $stmt->bindValue($key, $value, is_int($value) ? SQLITE3_INTEGER : SQLITE3_TEXT);
        }
        
        $result = $stmt->execute();
        $tag = $result->fetchArray(SQLITE3_ASSOC);
        
        if ($tag) {
            $tag['device_count'] = $this->getDeviceCountByTagId($tag['id']);
        }
        
        return $tag;
    }
    
    /**
     * 更新标签信息
     */
    public function updateTag($tagId, $userId, $name, $color = '#3498db') {
        $stmt = $this->db->prepare("UPDATE device_tags SET name = :name, color = :color WHERE id = :tagId AND user_id = :userId");
        $stmt->bindValue(':name', $name, SQLITE3_TEXT);
        $stmt->bindValue(':color', $color, SQLITE3_TEXT);
        $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 删除标签
     */
    public function deleteTag($tagId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM device_tags WHERE id = :tagId AND user_id = :userId");
        $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 给设备添加标签
     */
    public function addTagToDevice($tagId, $deviceId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT OR IGNORE INTO device_tag_relations (tag_id, device_id, created_at) VALUES (:tagId, :deviceId, :createdAt)");
        $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 从设备移除标签
     */
    public function removeTagFromDevice($tagId, $deviceId) {
        $stmt = $this->db->prepare("DELETE FROM device_tag_relations WHERE tag_id = :tagId AND device_id = :deviceId");
        $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 获取设备的标签列表
     */
    public function getTagsByDeviceId($deviceId) {
        $query = "SELECT t.* FROM device_tags t ".
                 "JOIN device_tag_relations r ON t.id = r.tag_id ".
                 "WHERE r.device_id = :deviceId ".
                 "ORDER BY r.created_at DESC";
        
        $stmt = $this->db->prepare($query);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $tags = array();
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $tags[] = $row;
        }
        
        return $tags;
    }
    
    /**
     * 获取标签的设备列表
     */
    public function getDevicesByTagId($tagId, $limit = 50, $offset = 0) {
        $query = "SELECT d.* FROM devices d ".
                 "JOIN device_tag_relations r ON d.device_id = r.device_id ".
                 "WHERE r.tag_id = :tagId ".
                 "ORDER BY r.created_at DESC LIMIT :limit OFFSET :offset";
        
        $stmt = $this->db->prepare($query);
        $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
        $stmt->bindValue(':limit', $limit, SQLITE3_INTEGER);
        $stmt->bindValue(':offset', $offset, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $devices = array();
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $devices[] = $row;
        }
        
        return $devices;
    }
    
    /**
     * 批量给设备添加标签
     */
    public function batchAddTagsToDevices($tagId, $deviceIds) {
        $successCount = 0;
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT OR IGNORE INTO device_tag_relations (tag_id, device_id, created_at) VALUES (:tagId, :deviceId, :createdAt)");
        
        foreach ($deviceIds as $deviceId) {
            $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
            $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
            
            if ($stmt->execute()) {
                if ($this->db->changes() > 0) {
                    $successCount++;
                }
            }
        }
        
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
        
        $stmt = $this->db->prepare("DELETE FROM device_tag_relations WHERE tag_id = :tagId AND device_id = :deviceId");
        
        foreach ($deviceIds as $deviceId) {
            $stmt->bindValue(':tagId', $tagId, SQLITE3_INTEGER);
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
            
            if ($stmt->execute()) {
                if ($this->db->changes() > 0) {
                    $successCount++;
                }
            }
        }
        
        return array(
            'success' => true,
            'success_count' => $successCount,
            'total_count' => count($deviceIds)
        );
    }
}
