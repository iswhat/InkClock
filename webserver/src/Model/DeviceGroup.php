<?php
/**
 * 设备分组模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class DeviceGroup extends BaseModel {
    
    public function createGroup($userId, $name, $description = '', $parentId = null) {
        $createdAt = date('Y-m-d H:i:s');
        
        $sql = "INSERT INTO device_groups (name, user_id, parent_id, created_at) VALUES (:name, :userId, :parentId, :createdAt)";
        $params = [
            'name' => $name,
            'userId' => $userId,
            'parentId' => $parentId,
            'createdAt' => $createdAt
        ];
        
        $result = $this->execute($sql, $params);
        $groupId = $this->lastInsertId();
        
        return [
            'success' => true,
            'group_id' => $groupId,
            'created_at' => $createdAt
        ];
    }
    
    public function getGroupsByUserId($userId, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM device_groups WHERE user_id = :userId ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = [
            'userId' => $userId,
            'limit' => $limit,
            'offset' => $offset
        ];
        
        $groups = $this->query($sql, $params);
        
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
    
    public function getGroups($userId, $limit = 50, $offset = 0) {
        return $this->getGroupsByUserId($userId, $limit, $offset);
    }
    
    private function getChildGroupCount($groupId) {
        $sql = "SELECT COUNT(*) as count FROM device_groups WHERE parent_id = :groupId";
        $params = ['groupId' => $groupId];
        $result = $this->query($sql, $params);
        if (!empty($result)) {
            return intval($result[0]['count']);
        }
        return 0;
    }
    
    public function getDeviceCountByGroupId($groupId) {
        $sql = "SELECT COUNT(*) as count FROM device_group_relations WHERE group_id = :groupId";
        $params = ['groupId' => $groupId];
        $result = $this->query($sql, $params);
        if (!empty($result)) {
            return intval($result[0]['count']);
        }
        return 0;
    }
    
    public function getGroupById($groupId, $userId = null) {
        $sql = "SELECT * FROM device_groups WHERE id = :groupId";
        $params = ['groupId' => $groupId];
        
        if ($userId) {
            $sql .= " AND user_id = :userId";
            $params['userId'] = $userId;
        }
        
        $result = $this->query($sql, $params);
        $group = !empty($result) ? $result[0] : null;
        
        if ($group) {
            $group['device_count'] = $this->getDeviceCountByGroupId($group['id']);
        }
        
        return $group;
    }
    
    public function getGroup($groupId) {
        return $this->getGroupById($groupId);
    }
    
    public function deleteGroup($groupId, $userId) {
        $sql = "DELETE FROM device_groups WHERE id = :groupId AND user_id = :userId";
        $params = [
            'groupId' => $groupId,
            'userId' => $userId
        ];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function addDeviceToGroup($groupId, $deviceId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $sql = "INSERT OR IGNORE INTO device_group_relations (group_id, device_id, created_at) VALUES (:groupId, :deviceId, :createdAt)";
        $params = [
            'groupId' => $groupId,
            'deviceId' => $deviceId,
            'createdAt' => $createdAt
        ];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function removeDeviceFromGroup($groupId, $deviceId) {
        $sql = "DELETE FROM device_group_relations WHERE group_id = :groupId AND device_id = :deviceId";
        $params = [
            'groupId' => $groupId,
            'deviceId' => $deviceId
        ];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function getDevicesByGroupId($groupId, $limit = 50, $offset = 0) {
        $sql = "SELECT d.* FROM devices d JOIN device_group_relations r ON d.device_id = r.device_id WHERE r.group_id = :groupId LIMIT :limit OFFSET :offset";
        $params = [
            'groupId' => $groupId,
            'limit' => $limit,
            'offset' => $offset
        ];
        return $this->query($sql, $params);
    }
    
    public function getGroupTree($userId) {
        $groups = $this->getGroupsByUserId($userId, 1000, 0);
        return $this->buildTree($groups);
    }
    
    private function buildTree($groups, $parentId = null) {
        $tree = [];
        foreach ($groups as $group) {
            if ($group['parent_id'] == $parentId) {
                $children = $this->buildTree($groups, $group['id']);
                if ($children) {
                    $group['children'] = $children;
                }
                $tree[] = $group;
            }
        }
        return $tree;
    }
}
?>
