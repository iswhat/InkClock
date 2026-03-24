<?php
/**
 * 设备模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class Device extends BaseModel {
    
    public function registerDevice($deviceId, $model = '', $firmwareVersion = 'unknown', $macAddress = '', $extraInfo = []) {
        try {
            $ipAddress = $_SERVER['REMOTE_ADDR'] ?? '';
            $ipv6Address = isset($_SERVER['HTTP_X_FORWARDED_FOR']) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : '';
            $lastActive = date('Y-m-d H:i:s');
            $connectionStatus = 1;
            
            $existing = $this->querySingle("SELECT id FROM devices WHERE device_id = :device_id", ['device_id' => $deviceId]);
            
            if ($existing) {
                $sql = "UPDATE devices SET ip_address = :ipAddress, ipv6_address = :ipv6Address, model = :model, firmware_version = :firmwareVersion, mac_address = :macAddress, last_active = :lastActive, connection_status = :connectionStatus WHERE device_id = :deviceId";
                $params = [
                    'ipAddress' => $ipAddress,
                    'ipv6Address' => $ipv6Address,
                    'model' => $model,
                    'firmwareVersion' => $firmwareVersion,
                    'macAddress' => $macAddress,
                    'lastActive' => $lastActive,
                    'connectionStatus' => $connectionStatus,
                    'deviceId' => $deviceId
                ];
                $this->execute($sql, $params);
            } else {
                $createdAt = date('Y-m-d H:i:s');
                $sql = "INSERT INTO devices (device_id, mac_address, ip_address, ipv6_address, model, firmware_version, created_at, last_active, connection_status) VALUES (:deviceId, :macAddress, :ipAddress, :ipv6Address, :model, :firmwareVersion, :createdAt, :lastActive, :connectionStatus)";
                $params = [
                    'deviceId' => $deviceId,
                    'macAddress' => $macAddress,
                    'ipAddress' => $ipAddress,
                    'ipv6Address' => $ipv6Address,
                    'model' => $model,
                    'firmwareVersion' => $firmwareVersion,
                    'createdAt' => $createdAt,
                    'lastActive' => $lastActive,
                    'connectionStatus' => $connectionStatus
                ];
                $this->execute($sql, $params);
            }
            
            $deviceInfo = $this->getDevice($deviceId);
            
            return [
                'success' => true,
                'device_id' => $deviceId,
                'message' => $existing ? '设备信息更新成功' : '设备注册成功',
                'device_info' => $deviceInfo
            ];
        } catch (\Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage(),
                'device_id' => $deviceId
            ];
        }
    }
    
    public function getDevice($deviceId) {
        $sql = "SELECT * FROM devices WHERE device_id = :deviceId";
        $params = ['deviceId' => $deviceId];
        $result = $this->query($sql, $params);
        return !empty($result) ? $result[0] : null;
    }
    
    public function getDevices($limit = 50, $offset = 0, $filters = [], $sortBy = 'created_at', $sortOrder = 'DESC') {
        $whereClause = '';
        $params = [];
        
        if (!empty($filters)) {
            $conditions = [];
            if (isset($filters['model'])) {
                $conditions[] = 'model = :model';
                $params['model'] = $filters['model'];
            }
            if (isset($filters['connection_status'])) {
                $conditions[] = 'connection_status = :connection_status';
                $params['connection_status'] = $filters['connection_status'];
            }
            if (isset($filters['search'])) {
                $conditions[] = '(device_id LIKE :search OR model LIKE :search OR nickname LIKE :search)';
                $params['search'] = '%' . $filters['search'] . '%';
            }
            if (!empty($conditions)) {
                $whereClause = 'WHERE ' . implode(' AND ', $conditions);
            }
        }
        
        $validSortFields = ['created_at', 'last_active', 'device_id', 'model', 'connection_status'];
        if (!in_array($sortBy, $validSortFields)) {
            $sortBy = 'created_at';
        }
        
        $sortOrder = strtoupper($sortOrder) === 'ASC' ? 'ASC' : 'DESC';
        
        $sql = "SELECT * FROM devices {$whereClause} ORDER BY {$sortBy} {$sortOrder} LIMIT :limit OFFSET :offset";
        $params['limit'] = $limit;
        $params['offset'] = $offset;
        
        return $this->query($sql, $params);
    }
    
    public function getDevicesCount($filters = []) {
        $whereClause = '';
        $params = [];
        
        if (!empty($filters)) {
            $conditions = [];
            if (isset($filters['model'])) {
                $conditions[] = 'model = :model';
                $params['model'] = $filters['model'];
            }
            if (isset($filters['connection_status'])) {
                $conditions[] = 'connection_status = :connection_status';
                $params['connection_status'] = $filters['connection_status'];
            }
            if (isset($filters['search'])) {
                $conditions[] = '(device_id LIKE :search OR model LIKE :search OR nickname LIKE :search)';
                $params['search'] = '%' . $filters['search'] . '%';
            }
            if (!empty($conditions)) {
                $whereClause = 'WHERE ' . implode(' AND ', $conditions);
            }
        }
        
        $sql = "SELECT COUNT(*) as count FROM devices {$whereClause}";
        $result = $this->query($sql, $params);
        if (!empty($result)) {
            return intval($result[0]['count']);
        }
        return 0;
    }
    
    public function updateDevice($deviceId, $data) {
        $updateFields = [];
        $params = [];
        
        $allowedFields = ['nickname', 'model', 'firmware_version', 'ip_address', 'ipv6_address', 'mac_address', 'connection_status', 'extra_info'];
        
        foreach ($allowedFields as $field) {
            if (isset($data[$field])) {
                $updateFields[] = "{$field} = :{$field}";
                $params[$field] = $data[$field];
            }
        }
        
        if (empty($updateFields)) {
            return ['success' => false, 'error' => '没有需要更新的字段'];
        }
        
        $params['deviceId'] = $deviceId;
        $sql = "UPDATE devices SET " . implode(', ', $updateFields) . " WHERE device_id = :deviceId";
        
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function deleteDevice($deviceId) {
        $sql = "DELETE FROM devices WHERE device_id = :deviceId";
        $params = ['deviceId' => $deviceId];
        $result = $this->execute($sql, $params);
        return $result !== false;
    }
    
    public function deviceExists($deviceId) {
        $result = $this->querySingle("SELECT id FROM devices WHERE device_id = :deviceId", ['deviceId' => $deviceId]);
        return $result !== null;
    }
    
    public function updateDeviceStatus($deviceId, $status, $lastActive = null) {
        $lastActive = $lastActive ?? date('Y-m-d H:i:s');
        $sql = "UPDATE devices SET connection_status = :status, last_active = :lastActive WHERE device_id = :deviceId";
        $params = [
            'status' => $status ? 1 : 0,
            'lastActive' => $lastActive,
            'deviceId' => $deviceId
        ];
        return $this->execute($sql, $params) !== false;
    }
}
?>
