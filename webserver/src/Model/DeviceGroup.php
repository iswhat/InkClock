<?php
/**
 * 设备分组模型
 */

namespace App\Model;

class DeviceGroup {
    private $db;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     */
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 创建设备分组
     */
    public function createGroup($userId, $name, $description = '') {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO device_groups (name, description, user_id, created_at) VALUES (:name, :description, :userId, :createdAt)");
        $stmt->bindValue(':name', $name, SQLITE3_TEXT);
        $stmt->bindValue(':description', $description, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $groupId = $this->db->lastInsertRowID();
        
        return array(
            'success' => $result !== false,
            'group_id' => $groupId,
            'created_at' => $createdAt
        );
    }
    
    /**
     * 获取用户的设备分组列表
     */
    public function getGroupsByUserId($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM device_groups WHERE user_id = :userId ORDER BY created_at DESC LIMIT :limit OFFSET :offset");
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':limit', $limit, SQLITE3_INTEGER);
        $stmt->bindValue(':offset', $offset, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $groups = array();
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $groups[] = $row;
        }
        
        // 为每个分组添加设备数量
        foreach ($groups as &$group) {
            $group['device_count'] = $this->getDeviceCountByGroupId($group['id']);
        }
        
        return $groups;
    }
    
    /**
     * 获取用户的设备分组列表（控制器调用的方法名）
     */
    public function getGroups($userId, $limit = 50, $offset = 0) {
        return $this->getGroupsByUserId($userId, $limit, $offset);
    }
    
    /**
     * 获取分组的设备数量
     */
    public function getDeviceCountByGroupId($groupId) {
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM device_group_relations WHERE group_id = :groupId");
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $row = $result->fetchArray(SQLITE3_ASSOC);
        
        return $row['count'];
    }
    
    /**
     * 获取分组详情
     */
    public function getGroupById($groupId, $userId = null) {
        $query = "SELECT * FROM device_groups WHERE id = :groupId";
        $params = array(':groupId' => $groupId);
        
        if ($userId) {
            $query .= " AND user_id = :userId";
            $params[':userId'] = $userId;
        }
        
        $stmt = $this->db->prepare($query);
        
        foreach ($params as $key => $value) {
            $stmt->bindValue($key, $value, is_int($value) ? SQLITE3_INTEGER : SQLITE3_TEXT);
        }
        
        $result = $stmt->execute();
        $group = $result->fetchArray(SQLITE3_ASSOC);
        
        if ($group) {
            $group['device_count'] = $this->getDeviceCountByGroupId($group['id']);
        }
        
        return $group;
    }
    
    /**
     * 获取分组详情（控制器调用的方法名）
     */
    public function getGroup($groupId) {
        return $this->getGroupById($groupId);
    }
    
    /**
     * 更新分组信息
     */
    public function updateGroup($groupId, $userId, $name, $description = '') {
        $stmt = $this->db->prepare("UPDATE device_groups SET name = :name, description = :description WHERE id = :groupId AND user_id = :userId");
        $stmt->bindValue(':name', $name, SQLITE3_TEXT);
        $stmt->bindValue(':description', $description, SQLITE3_TEXT);
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 删除分组
     */
    public function deleteGroup($groupId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM device_groups WHERE id = :groupId AND user_id = :userId");
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 添加设备到分组
     */
    public function addDeviceToGroup($groupId, $deviceId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT OR IGNORE INTO device_group_relations (group_id, device_id, created_at) VALUES (:groupId, :deviceId, :createdAt)");
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 从分组中移除设备
     */
    public function removeDeviceFromGroup($groupId, $deviceId) {
        $stmt = $this->db->prepare("DELETE FROM device_group_relations WHERE group_id = :groupId AND device_id = :deviceId");
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 获取分组中的设备列表
     */
    public function getDevicesByGroupId($groupId, $limit = 50, $offset = 0) {
        $query = "SELECT d.* FROM devices d ".
                 "JOIN device_group_relations r ON d.device_id = r.device_id ".
                 "WHERE r.group_id = :groupId ".
                 "ORDER BY r.created_at DESC LIMIT :limit OFFSET :offset";
        
        $stmt = $this->db->prepare($query);
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
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
     * 获取分组中的设备列表（控制器调用的方法名）
     */
    public function getDevicesByGroup($groupId, $limit = 50, $offset = 0) {
        return $this->getDevicesByGroupId($groupId, $limit, $offset);
    }
    
    /**
     * 获取设备所属的分组列表
     */
    public function getGroupsByDeviceId($deviceId) {
        $query = "SELECT g.* FROM device_groups g ".
                 "JOIN device_group_relations r ON g.id = r.group_id ".
                 "WHERE r.device_id = :deviceId ".
                 "ORDER BY r.created_at DESC";
        
        $stmt = $this->db->prepare($query);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $groups = array();
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $groups[] = $row;
        }
        
        return $groups;
    }
}
