<?php
/**
 * 设备分组模型
 */

namespace InkClock\Model;

class DeviceGroup {
    private $db;
    
    /**
     * 构造函�?
     * @param \SQLite3 $db 数据库连�?
     */
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 创建设备分组
     */
    public function createGroup($userId, $name, $parentId = null) {
        $createdAt = date('Y-m-d H:i:s');
        
        // 检查父分组是否存在且属于当前用�?
        if ($parentId) {
            $parentGroup = $this->getGroupById($parentId, $userId);
            if (!$parentGroup) {
                return array('success' => false, 'error' => '父分组不存在或不属于当前用户');
            }
            
            // 检查是否会形成循环依赖
            if ($this->checkCircularDependency($parentId, $parentId)) {
                return array('success' => false, 'error' => '不能将分组设置为自身或其子分组的子分�?);
            }
        }
        
        $stmt = $this->db->prepare("INSERT INTO device_groups (name, user_id, parent_id, created_at) VALUES (:name, :userId, :parentId, :createdAt)");
        $stmt->bindValue(':name', $name, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':parentId', $parentId, $parentId ? SQLITE3_INTEGER : SQLITE3_NULL);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        $stmt->execute();
        
        $groupId = $this->db->lastInsertRowID();
        
        return array(
            'success' => true,
            'group_id' => $groupId,
            'created_at' => $createdAt
        );
    }
    
    /**
     * 检查循环依�?
     */
    private function checkCircularDependency($groupId, $parentId) {
        // 获取父分�?
        $parentGroup = $this->getGroupById($parentId);
        if (!$parentGroup) {
            return false;
        }
        
        // 如果父分组的父分组是当前分组，则形成循环依赖
        if ($parentGroup['parent_id'] == $groupId) {
            return true;
        }
        
        // 递归检查父分组的父分组
        if ($parentGroup['parent_id']) {
            return $this->checkCircularDependency($groupId, $parentGroup['parent_id']);
        }
        
        return false;
    }
    
    /**
     * 获取用户的设备分组列�?
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
        
        // 为每个分组添加设备数量和父分组信�?
        foreach ($groups as &$group) {
            $group['device_count'] = $this->getDeviceCountByGroupId($group['id']);
            $group['child_count'] = $this->getChildGroupCount($group['id']);
            
            if ($group['parent_id']) {
                $parentGroup = $this->getGroupById($group['parent_id']);
                $group['parent_name'] = $parentGroup['name'] ?? null;
            } else {
                $group['parent_name'] = null;
            }
        }
        
        return $groups;
    }
    
    /**
     * 获取分组的子分组数量
     */
    private function getChildGroupCount($groupId) {
        $stmt = $this->db->prepare("SELECT COUNT(*) as count FROM device_groups WHERE parent_id = :groupId");
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $row = $result->fetchArray(SQLITE3_ASSOC);
        
        return $row['count'];
    }
    
    /**
     * 获取用户的设备分组列表（控制器调用的方法名）
     */
    public function getGroups($userId, $limit = 50, $offset = 0) {
        return $this->getGroupsByUserId($userId, $limit, $offset);
    }
    
    /**
     * 获取分组的设备数�?
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
     * 获取分组详情（控制器调用的方法名�?
     */
    public function getGroup($groupId) {
        return $this->getGroupById($groupId);
    }
    
    /**
     * 更新分组信息
     */
    public function updateGroup($groupId, $userId, $name, $description = '') {
    
    /**
     * 删除分组
     */
    public function deleteGroup($groupId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM device_groups WHERE id = :groupId AND user_id = :userId");
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 添加设备到分�?
     */
    public function addDeviceToGroup($groupId, $deviceId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT OR IGNORE INTO device_group_relations (group_id, device_id, created_at) VALUES (:groupId, :deviceId, :createdAt)");
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        $stmt->execute();
        
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
        $stmt->execute();
        
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
     * 获取分组中的设备列表（控制器调用的方法名�?
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
    
    /**
     * 获取分组树结�?
     */
    public function getGroupTree($userId) {
        // 获取用户的所有分�?
        $allGroups = $this->getGroupsByUserId($userId, 1000, 0);
        
        // 构建分组ID到分组的映射
        $groupMap = array();
        foreach ($allGroups as $group) {
            $groupMap[$group['id']] = $group;
            $groupMap[$group['id']]['children'] = array();
        }
        
        // 构建树形结构
        $tree = array();
        foreach ($groupMap as $groupId => $group) {
            if (empty($group['parent_id'])) {
                // 根分�?
                $tree[] = &$groupMap[$groupId];
            } else if (isset($groupMap[$group['parent_id']])) {
                // 子分�?
                $groupMap[$group['parent_id']]['children'][] = &$groupMap[$groupId];
            }
        }
        
        return $tree;
    }
    
    /**
     * 更新分组信息
     */
    public function updateGroup($groupId, $userId, $name, $parentId = null) {
        // 检查父分组是否存在且属于当前用�?
        if ($parentId) {
            $parentGroup = $this->getGroupById($parentId, $userId);
            if (!$parentGroup) {
                return array('success' => false, 'error' => '父分组不存在或不属于当前用户');
            }
            
            // 检查是否会形成循环依赖
            if ($this->checkCircularDependency($groupId, $parentId)) {
                return array('success' => false, 'error' => '不能将分组设置为自身或其子分组的子分�?);
            }
        }
        
        $stmt = $this->db->prepare("UPDATE device_groups SET name = :name, parent_id = :parentId WHERE id = :groupId AND user_id = :userId");
        $stmt->bindValue(':name', $name, SQLITE3_TEXT);
        $stmt->bindValue(':parentId', $parentId, $parentId ? SQLITE3_INTEGER : SQLITE3_NULL);
        $stmt->bindValue(':groupId', $groupId, SQLITE3_INTEGER);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $affectedRows = $this->db->changes();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 获取子分组列�?
     */
    public function getChildGroups($groupId, $userId = null) {
        $query = "SELECT * FROM device_groups WHERE parent_id = :groupId";
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
        
        $groups = array();
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $row['device_count'] = $this->getDeviceCountByGroupId($row['id']);
            $row['child_count'] = $this->getChildGroupCount($row['id']);
            $groups[] = $row;
        }
        
        return $groups;
    }
}
