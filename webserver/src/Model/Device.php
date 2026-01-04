<?php
/**
 * 设备模型
 */

namespace App\Model;

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
     * @return array 注册结果
     */
    public function registerDevice($deviceId, $model = '', $firmwareVersion = 'unknown') {
        $macAddress = ''; // 暂时为空，可在后续扩展中添加
        $ipAddress = $_SERVER['REMOTE_ADDR'];
        $ipv6Address = isset($_SERVER['HTTP_X_FORWARDED_FOR']) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : '';
        $lastActive = date('Y-m-d H:i:s');
        
        // 检查设备是否已存在
        $stmt = $this->db->prepare("SELECT id FROM devices WHERE device_id = :deviceId");
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $result = $stmt->execute();
        $exists = $result->fetchArray(SQLITE3_ASSOC) !== false;
        $result->finalize();
        $stmt->close();
        
        if ($exists) {
            // 更新设备信息
            $stmt = $this->db->prepare("UPDATE devices SET ip_address = :ipAddress, ipv6_address = :ipv6Address, model = :model, firmware_version = :firmwareVersion, last_active = :lastActive WHERE device_id = :deviceId");
            $stmt->bindValue(':ipAddress', $ipAddress, SQLITE3_TEXT);
            $stmt->bindValue(':ipv6Address', $ipv6Address, SQLITE3_TEXT);
            $stmt->bindValue(':model', $model, SQLITE3_TEXT);
            $stmt->bindValue(':firmwareVersion', $firmwareVersion, SQLITE3_TEXT);
            $stmt->bindValue(':lastActive', $lastActive, SQLITE3_TEXT);
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        } else {
            // 插入新设备
            $createdAt = date('Y-m-d H:i:s');
            $stmt = $this->db->prepare("INSERT INTO devices (device_id, mac_address, ip_address, ipv6_address, model, firmware_version, created_at, last_active) VALUES (:deviceId, :macAddress, :ipAddress, :ipv6Address, :model, :firmwareVersion, :createdAt, :lastActive)");
            $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
            $stmt->bindValue(':macAddress', $macAddress, SQLITE3_TEXT);
            $stmt->bindValue(':ipAddress', $ipAddress, SQLITE3_TEXT);
            $stmt->bindValue(':ipv6Address', $ipv6Address, SQLITE3_TEXT);
            $stmt->bindValue(':model', $model, SQLITE3_TEXT);
            $stmt->bindValue(':firmwareVersion', $firmwareVersion, SQLITE3_TEXT);
            $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
            $stmt->bindValue(':lastActive', $lastActive, SQLITE3_TEXT);
        }
        
        $result = $stmt->execute();
        $stmt->close();
        
        return [
            'success' => $result !== false,
            'device_id' => $deviceId
        ];
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
     * @return array 设备列表
     */
    public function getDevices($limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM devices ORDER BY created_at DESC LIMIT :limit OFFSET :offset");
        $stmt->bindValue(':limit', $limit, SQLITE3_INTEGER);
        $stmt->bindValue(':offset', $offset, SQLITE3_INTEGER);
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