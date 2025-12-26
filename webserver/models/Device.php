<?php
/**
 * 设备模型
 */
require_once '../config.php';

class Device {
    private $db;
    
    public function __construct() {
        $this->db = getDbConnection();
    }
    
    /**
     * 注册或更新设备信息
     */
    public function registerDevice($deviceInfo) {
        $deviceId = isset($deviceInfo['device_id']) ? $deviceInfo['device_id'] : generateDeviceId();
        $macAddress = isset($deviceInfo['mac_address']) ? $deviceInfo['mac_address'] : '';
        $ipAddress = $_SERVER['REMOTE_ADDR'];
        $ipv6Address = isset($_SERVER['HTTP_X_FORWARDED_FOR']) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : '';
        $model = isset($deviceInfo['model']) ? $deviceInfo['model'] : '';
        $firmwareVersion = isset($deviceInfo['firmware_version']) ? $deviceInfo['firmware_version'] : 'unknown';
        $lastActive = date('Y-m-d H:i:s');
        
        // 检查设备是否已存在
        $stmt = $this->db->prepare("SELECT id FROM devices WHERE device_id = ? OR mac_address = ?");
        $stmt->bind_param("ss", $deviceId, $macAddress);
        $stmt->execute();
        $result = $stmt->get_result();
        
        if ($result->num_rows > 0) {
            // 更新设备信息
            $stmt = $this->db->prepare("UPDATE devices SET ip_address = ?, ipv6_address = ?, model = ?, firmware_version = ?, last_active = ? WHERE device_id = ?");
            $stmt->bind_param("ssssss", $ipAddress, $ipv6Address, $model, $firmwareVersion, $lastActive, $deviceId);
            $stmt->execute();
            $affectedRows = $stmt->affected_rows;
        } else {
            // 插入新设备
            $stmt = $this->db->prepare("INSERT INTO devices (device_id, mac_address, ip_address, ipv6_address, model, firmware_version, created_at, last_active) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
            $createdAt = date('Y-m-d H:i:s');
            $stmt->bind_param("ssssssss", $deviceId, $macAddress, $ipAddress, $ipv6Address, $model, $firmwareVersion, $createdAt, $lastActive);
            $stmt->execute();
            $affectedRows = $stmt->affected_rows;
        }
        
        $stmt->close();
        
        return array(
            'success' => $affectedRows > 0,
            'device_id' => $deviceId
        );
    }
    
    /**
     * 获取设备信息
     */
    public function getDevice($deviceId) {
        $stmt = $this->db->prepare("SELECT * FROM devices WHERE device_id = ?");
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $device = $result->fetch_assoc();
        $stmt->close();
        return $device;
    }
    
    /**
     * 获取设备列表
     */
    public function getDevices($limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM devices ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bind_param("ii", $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $devices = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        return $devices;
    }
    
    /**
     * 更新设备最后活跃时间
     */
    public function updateLastActive($deviceId) {
        $lastActive = date('Y-m-d H:i:s');
        $stmt = $this->db->prepare("UPDATE devices SET last_active = ?, ip_address = ? WHERE device_id = ?");
        $ipAddress = $_SERVER['REMOTE_ADDR'];
        $stmt->bind_param("sss", $lastActive, $ipAddress, $deviceId);
        $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        return $affectedRows > 0;
    }
    
    /**
     * 删除设备
     */
    public function deleteDevice($deviceId) {
        $stmt = $this->db->prepare("DELETE FROM devices WHERE device_id = ?");
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        return $affectedRows > 0;
    }
    
    /**
     * 检查设备是否存在
     */
    public function deviceExists($deviceId) {
        $stmt = $this->db->prepare("SELECT id FROM devices WHERE device_id = ?");
        $stmt->bind_param("s", $deviceId);
        $stmt->execute();
        $result = $stmt->get_result();
        $exists = $result->num_rows > 0;
        $stmt->close();
        return $exists;
    }
}
?>