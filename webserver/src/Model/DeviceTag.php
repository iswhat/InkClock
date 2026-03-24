<?php
/**
 * 设备标签模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class DeviceTag extends BaseModel {
    
    public function createTag($userId, $name, $color = '#3498db') {
        $createdAt = date('Y-m-d H:i:s');
        
        $sql = "INSERT INTO device_tags (name, color, user_id, created_at) VALUES (:name, :color, :userId, :createdAt)";
        $params = [
            'name' => $name,
            'color' => $color,
            'userId' => $userId,
            'createdAt' => $createdAt
        ];
        
        $result = $this->execute($sql, $params);
        $tagId = $this->lastInsertId();
        
        return [
            'success' => $result !== false,
            'tag_id' => $tagId,
            'created_at' => $createdAt
        ];
    }
    
    public function getTagsByUserId($userId, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM device_tags WHERE user_id = :userId ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = ['userId' => $userId, 'limit' => $limit, 'offset' => $offset];
        
        $tags = $this->query($sql, $params);
        
        foreach ($tags as &$tag) {
            $tag['device_count'] = $this->getDeviceCountByTagId($tag['id']);
        }
        
        return $tags;
    }
    
    public function getDeviceCountByTagId($tagId) {
        $sql = "SELECT COUNT(*) as count FROM device_tag_relations WHERE tag_id = :tagId";
        $params = ['tagId' => $tagId];
        $result = $this->query($sql, $params);
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
    
    public function getTagById($tagId, $userId = null) {
        $sql = "SELECT * FROM device_tags WHERE id = :tagId";
        $params = ['tagId' => $tagId];
        
        if ($userId) {
            $sql .= " AND user_id = :userId";
            $params['userId'] = $userId;
        }
        
        $result = $this->query($sql, $params);
        $tag = !empty($result) ? $result[0] : null;
        
        if ($tag) {
            $tag['device_count'] = $this->getDeviceCountByTagId($tag['id']);
        }
        
        return $tag;
    }
    
    public function updateTag($tagId, $userId, $name, $color = '#3498db') {
        $sql = "UPDATE device_tags SET name = :name, color = :color WHERE id = :tagId AND user_id = :userId";
        $params = [
            'name' => $name,
            'color' => $color,
            'tagId' => $tagId,
            'userId' => $userId
        ];
        
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function deleteTag($tagId, $userId) {
        $sql = "DELETE FROM device_tags WHERE id = :tagId AND user_id = :userId";
        $params = ['tagId' => $tagId, 'userId' => $userId];
        $result = $this->execute($sql, $params);
        
        return ['success' => $result !== false];
    }
    
    public function addTagToDevice($tagId, $deviceId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $sql = "INSERT OR IGNORE INTO device_tag_relations (tag_id, device_id, created_at) VALUES (:tagId, :deviceId, :createdAt)";
        $params = [
            'tagId' => $tagId,
            'deviceId' => $deviceId,
            'createdAt' => $createdAt
        ];
        
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function removeTagFromDevice($tagId, $deviceId) {
        $sql = "DELETE FROM device_tag_relations WHERE tag_id = :tagId AND device_id = :deviceId";
        $params = ['tagId' => $tagId, 'deviceId' => $deviceId];
        $result = $this->execute($sql, $params);
        
        return ['success' => $result !== false];
    }
    
    public function getTagsByDeviceId($deviceId) {
        $sql = "SELECT t.* FROM device_tags t JOIN device_tag_relations r ON t.id = r.tag_id WHERE r.device_id = :deviceId";
        $params = ['deviceId' => $deviceId];
        return $this->query($sql, $params);
    }
    
    public function getDevicesByTagId($tagId, $limit = 50, $offset = 0) {
        $sql = "SELECT d.* FROM devices d JOIN device_tag_relations r ON d.device_id = r.device_id WHERE r.tag_id = :tagId LIMIT :limit OFFSET :offset";
        $params = ['tagId' => $tagId, 'limit' => $limit, 'offset' => $offset];
        return $this->query($sql, $params);
    }
    
    public function clearDeviceTags($deviceId) {
        $sql = "DELETE FROM device_tag_relations WHERE device_id = :deviceId";
        $params = ['deviceId' => $deviceId];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function setDeviceTags($deviceId, $tagIds) {
        $this->clearDeviceTags($deviceId);
        
        foreach ($tagIds as $tagId) {
            $this->addTagToDevice($tagId, $deviceId);
        }
        
        return ['success' => true];
    }
    
    public function getTagCount($userId) {
        $sql = "SELECT COUNT(*) as count FROM device_tags WHERE user_id = :userId";
        $params = ['userId' => $userId];
        $result = $this->query($sql, $params);
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
}
?>
