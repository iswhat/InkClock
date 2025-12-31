<?php
/**
 * 设备分组模型
 */
require_once __DIR__ . '/../utils/Database.php';

class DeviceGroup {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
    }
    
    /**
     * 创建设备分组
     */
    public function createGroup($userId, $name, $description = '') {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO device_groups (name, description, user_id, created_at) VALUES (?, ?, ?, ?)");
        $stmt->bind_param("ssis", $name, $description, $userId, $createdAt);
        $stmt->execute();
        
        $groupId = $this->db->insert_id;
        $stmt->close();
        
        return array(
            'success' => true,
            'group_id' => $groupId,
            'created_at' => $createdAt
        );
    }
    
    /**
     * 获取用户的设备分组列表
     */
    public function getGroupsByUserId($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM device_groups WHERE user_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bind_param("iii", $userId, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $groups = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
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
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM device_group_relations WHERE group_id = ?");
        $stmt->bind_param("i", $groupId);
        $stmt->execute();
        $result = $stmt->get_result();
        $count = $result->fetch_assoc()['count'];
        $stmt->close();
        return $count;
    }
    
    /**
     * 获取分组详情
     */
    public function getGroupById($groupId, $userId = null) {
        $query = "SELECT * FROM device_groups WHERE id = ?";
        $params = array($groupId);
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
        $group = $result->fetch_assoc();
        $stmt->close();
        
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
        $stmt = $this->db->prepare("UPDATE device_groups SET name = ?, description = ? WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ssii", $name, $description, $groupId, $userId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 删除分组
     */
    public function deleteGroup($groupId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM device_groups WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ii", $groupId, $userId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 添加设备到分组
     */
    public function addDeviceToGroup($groupId, $deviceId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT IGNORE INTO device_group_relations (group_id, device_id, created_at) VALUES (?, ?, ?)");
        $stmt->bind_param("iss", $groupId, $deviceId, $createdAt);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 从分组中移除设备
     */
    public function removeDeviceFromGroup($groupId, $deviceId) {
        $stmt = $this->db->prepare("DELETE FROM device_group_relations WHERE group_id = ? AND device_id = ?");
        $stmt->bind_param("is", $groupId, $deviceId);
        $stmt->execute();
        
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 获取分组中的设备列表
     */
    public function getDevicesByGroupId($groupId, $limit = 50, $offset = 0) {
        $query = "SELECT d.* FROM devices d ".
                 "JOIN device_group_relations r ON d.device_id = r.device_id ".
                 "WHERE r.group_id = ? ".
                 "ORDER BY r.created_at DESC LIMIT ? OFFSET ?";
        
        $stmt = $this->db->prepare($query);
        $stmt->bind_param("iii", $groupId, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $devices = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
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
                 "WHERE r.device_id = ? ".
                 "ORDER BY r.created_at DESC";
        
        $stmt = $this->db->prepare($query);
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $groups = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
        return $groups;
    }
}
?>