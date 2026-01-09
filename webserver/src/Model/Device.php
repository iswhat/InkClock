<?php
/**
 * 设备模型
 */

namespace InkClock\Model;

class Device {
    private $db;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     */
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 注册或更新设备信息
     * @param string $deviceId 设备ID
     * @param string $model 设备型号
     * @param string $firmwareVersion 固件版本
     * @param string $macAddress MAC地址（可选）
     * @param array $extraInfo 额外信息（可选）
     * @return array 注册结果
     */
    public function registerDevice($deviceId, $model = '', $firmwareVersion = 'unknown', $macAddress = '', $extraInfo = []) {
        try {
            // 开始事务
            $this->db->exec('BEGIN TRANSACTION');
            
            $ipAddress = $_SERVER['REMOTE_ADDR'] ?? '';
            $ipv6Address = isset($_SERVER['HTTP_X_FORWARDED_FOR']) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : '';
            $lastActive = date('Y-m-d H:i:s');
            $connectionStatus = 1; // 默认为在线状态
            
            // 检查设备是否已存在
            $stmt = $this->db->prepare("SELECT id FROM devices WHERE device_id = :deviceId");
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
            $result = $stmt->execute();
            $exists = $result->fetchArray(SQLITE3_ASSOC) !== false;
            $result->finalize();
            $stmt->close();
            
            if ($exists) {
                // 更新设备信息
                $stmt = $this->db->prepare("UPDATE devices SET ip_address = :ipAddress, ipv6_address = :ipv6Address, model = :model, firmware_version = :firmwareVersion, mac_address = :macAddress, last_active = :lastActive, connection_status = :connectionStatus WHERE device_id = :deviceId");
                $stmt->bindValue(':ipAddress', $ipAddress, SQLITE3_TEXT);
                $stmt->bindValue(':ipv6Address', $ipv6Address, SQLITE3_TEXT);
                $stmt->bindValue(':model', $model, SQLITE3_TEXT);
                $stmt->bindValue(':firmwareVersion', $firmwareVersion, SQLITE3_TEXT);
                $stmt->bindValue(':macAddress', $macAddress, SQLITE3_TEXT);
                $stmt->bindValue(':lastActive', $lastActive, SQLITE3_TEXT);
                $stmt->bindValue(':connectionStatus', $connectionStatus, SQLITE3_INTEGER);
                $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
            } else {
                // 插入新设备
                $createdAt = date('Y-m-d H:i:s');
                $stmt = $this->db->prepare("INSERT INTO devices (device_id, mac_address, ip_address, ipv6_address, model, firmware_version, created_at, last_active, connection_status) VALUES (:deviceId, :macAddress, :ipAddress, :ipv6Address, :model, :firmwareVersion, :createdAt, :lastActive, :connectionStatus)");
                $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
                $stmt->bindValue(':macAddress', $macAddress, SQLITE3_TEXT);
                $stmt->bindValue(':ipAddress', $ipAddress, SQLITE3_TEXT);
                $stmt->bindValue(':ipv6Address', $ipv6Address, SQLITE3_TEXT);
                $stmt->bindValue(':model', $model, SQLITE3_TEXT);
                $stmt->bindValue(':firmwareVersion', $firmwareVersion, SQLITE3_TEXT);
                $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
                $stmt->bindValue(':lastActive', $lastActive, SQLITE3_TEXT);
                $stmt->bindValue(':connectionStatus', $connectionStatus, SQLITE3_INTEGER);
            }
            
            $result = $stmt->execute();
            if (!$result) {
                throw new \Exception('设备注册失败: ' . $this->db->lastErrorMsg());
            }
            
            // 提交事务
            $this->db->exec('COMMIT');
            
            // 获取设备完整信息
            $deviceInfo = $this->getDevice($deviceId);
            
            return [
                'success' => true,
                'device_id' => $deviceId,
                'message' => $exists ? '设备信息更新成功' : '设备注册成功',
                'device_info' => $deviceInfo
            ];
        } catch (\Exception $e) {
            // 回滚事务
            $this->db->exec('ROLLBACK');
            
            return [
                'success' => false,
                'error' => $e->getMessage(),
                'device_id' => $deviceId
            ];
        }
    }
    
    /**
     * 获取设备信息
     * @param string $deviceId 设备ID
     * @return array|null 设备信息
     */
    public function getDevice($deviceId) {
        $stmt = $this->db->prepare("SELECT * FROM devices WHERE device_id = :deviceId");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        $device = $result->fetchArray(SQLITE3_ASSOC);
        $result->finalize();
        $stmt->close();
        return $device;
    }
    
    /**
     * 获取设备列表
     * @param int $limit 限制数量
     * @param int $offset 偏移量
     * @param array $filters 过滤条件
     * @param string $sortBy 排序字段
     * @param string $sortOrder 排序方向
     * @return array 设备列表
     */
    public function getDevices($limit = 50, $offset = 0, $filters = [], $sortBy = 'created_at', $sortOrder = 'DESC') {
        // 构建查询条件
        $whereClause = '';
        $params = [];
        
        if (!empty($filters)) {
            $conditions = [];
            if (isset($filters['model'])) {
                $conditions[] = 'model = :model';
                $params[':model'] = $filters['model'];
            }
            if (isset($filters['connection_status'])) {
                $conditions[] = 'connection_status = :connection_status';
                $params[':connection_status'] = $filters['connection_status'];
            }
            if (isset($filters['search'])) {
                $conditions[] = '(device_id LIKE :search OR model LIKE :search OR nickname LIKE :search)';
                $params[':search'] = '%' . $filters['search'] . '%';
            }
            if (!empty($conditions)) {
                $whereClause = 'WHERE ' . implode(' AND ', $conditions);
            }
        }
        
        // 验证排序字段
        $validSortFields = ['created_at', 'last_active', 'device_id', 'model', 'connection_status'];
        if (!in_array($sortBy, $validSortFields)) {
            $sortBy = 'created_at';
        }
        
        // 验证排序方向
        $sortOrder = strtoupper($sortOrder) === 'ASC' ? 'ASC' : 'DESC';
        
        // 构建SQL查询
        $sql = "SELECT * FROM devices {$whereClause} ORDER BY {$sortBy} {$sortOrder} LIMIT :limit OFFSET :offset";
        $stmt = $this->db->prepare($sql);
        
        // 绑定参数
        $stmt->bindValue(':limit', $limit, SQLITE3_INTEGER);
        $stmt->bindValue(':offset', $offset, SQLITE3_INTEGER);
        
        foreach ($params as $key => $value) {
            $stmt->bindValue($key, $value, SQLITE3_TEXT);
        }
        
        $result = $stmt->execute();
        
        $devices = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $devices[] = $row;
        }
        $result->finalize();
        $stmt->close();
        
        return $devices;
    }
    
    /**
     * 获取设备总数
     * @param array $filters 过滤条件
     * @return int 设备总数
     */
    public function getDevicesCount($filters = []) {
        // 构建查询条件
        $whereClause = '';
        $params = [];
        
        if (!empty($filters)) {
            $conditions = [];
            if (isset($filters['model'])) {
                $conditions[] = 'model = :model';
                $params[':model'] = $filters['model'];
            }
            if (isset($filters['connection_status'])) {
                $conditions[] = 'connection_status = :connection_status';
                $params[':connection_status'] = $filters['connection_status'];
            }
            if (isset($filters['search'])) {
                $conditions[] = '(device_id LIKE :search OR model LIKE :search OR nickname LIKE :search)';
                $params[':search'] = '%' . $filters['search'] . '%';
            }
            if (!empty($conditions)) {
                $whereClause = 'WHERE ' . implode(' AND ', $conditions);
            }
        }
        
        // 构建SQL查询
        $sql = "SELECT COUNT(*) as count FROM devices {$whereClause}";
        $stmt = $this->db->prepare($sql);
        
        // 绑定参数
        foreach ($params as $key => $value) {
            $stmt->bindValue($key, $value, SQLITE3_TEXT);
        }
        
        $result = $stmt->execute();
        $count = $result->fetchArray(SQLITE3_ASSOC)['count'];
        $result->finalize();
        $stmt->close();
        
        return $count;
    }
    
    /**
     * 更新设备最后活跃时间
     * @param string $deviceId 设备ID
     * @return bool 更新结果
     */
    public function updateLastActive($deviceId) {
        $lastActive = date('Y-m-d H:i:s');
        $ipAddress = $_SERVER['REMOTE_ADDR'];
        
        $stmt = $this->db->prepare("UPDATE devices SET last_active = :lastActive, ip_address = :ipAddress WHERE device_id = :deviceId");
        $stmt->bindValue(':lastActive', $lastActive, SQLITE3_TEXT);
        $stmt->bindValue(':ipAddress', $ipAddress, SQLITE3_TEXT);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        $stmt->close();
        
        return $result !== false;
    }
    
    /**
     * 删除设备
     * @param string $deviceId 设备ID
     * @return bool 删除结果
     */
    public function deleteDevice($deviceId) {
        $stmt = $this->db->prepare("DELETE FROM devices WHERE device_id = :deviceId");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        
        $result = $stmt->execute();
        $stmt->close();
        
        return $result !== false;
    }
    
    /**
     * 检查设备是否存在
     * @param string $deviceId 设备ID
     * @return bool 是否存在
     */
    public function deviceExists($deviceId) {
        $stmt = $this->db->prepare("SELECT id FROM devices WHERE device_id = :deviceId");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        $exists = $result->fetchArray(SQLITE3_ASSOC) !== false;
        $result->finalize();
        $stmt->close();
        
        return $exists;
    }
    
    /**
     * 更新设备信息
     * @param string $deviceId 设备ID
     * @param array $deviceInfo 设备信息
     * @return array 更新结果
     */
    public function updateDevice($deviceId, $deviceInfo) {
        // 构建更新字段
        $updateFields = [];
        $updateValues = [];
        
        if (isset($deviceInfo['model'])) {
            $updateFields[] = "model = :model";
            $updateValues[':model'] = $deviceInfo['model'];
        }
        
        if (isset($deviceInfo['firmware_version'])) {
            $updateFields[] = "firmware_version = :firmwareVersion";
            $updateValues[':firmwareVersion'] = $deviceInfo['firmware_version'];
        }
        
        if (isset($deviceInfo['mac_address'])) {
            $updateFields[] = "mac_address = :macAddress";
            $updateValues[':macAddress'] = $deviceInfo['mac_address'];
        }
        
        if (empty($updateFields)) {
            return ['success' => true, 'message' => '没有需要更新的字段'];
        }
        
        $updateFields[] = "last_active = :lastActive";
        $updateValues[':lastActive'] = date('Y-m-d H:i:s');
        $updateValues[':deviceId'] = $deviceId;
        
        $sql = "UPDATE devices SET " . implode(', ', $updateFields) . " WHERE device_id = :deviceId";
        $stmt = $this->db->prepare($sql);
        
        foreach ($updateValues as $key => $value) {
            $stmt->bindValue($key, $value, SQLITE3_TEXT);
        }
        
        $result = $stmt->execute();
        $stmt->close();
        
        return [
            'success' => $result !== false
        ];
    }
}