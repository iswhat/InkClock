<?php
/**
 * 系统日志模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class SystemLog extends BaseModel {
    
    public function log($level, $category, $message, $userId = null, $deviceId = null) {
        $createdAt = date('Y-m-d H:i:s');
        $ipAddress = $_SERVER['REMOTE_ADDR'] ?? '';
        
        $sql = "INSERT INTO system_logs (level, category, message, user_id, device_id, ip_address, created_at) VALUES (:level, :category, :message, :userId, :deviceId, :ipAddress, :createdAt)";
        $params = [
            'level' => $level,
            'category' => $category,
            'message' => $message,
            'userId' => $userId,
            'deviceId' => $deviceId,
            'ipAddress' => $ipAddress,
            'createdAt' => $createdAt
        ];
        
        $result = $this->execute($sql, $params);
        $logId = $this->lastInsertId();
        
        return ['success' => $result !== false, 'log_id' => $logId];
    }
    
    public function getLogs($limit = 50, $offset = 0, $level = null, $startTime = null, $endTime = null) {
        $sql = "SELECT * FROM system_logs WHERE 1=1";
        $params = [];
        
        if ($level) {
            $sql .= " AND level = :level";
            $params['level'] = $level;
        }
        
        if ($startTime) {
            $sql .= " AND created_at >= :startTime";
            $params['startTime'] = $startTime;
        }
        
        if ($endTime) {
            $sql .= " AND created_at <= :endTime";
            $params['endTime'] = $endTime;
        }
        
        $sql .= " ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params['limit'] = $limit;
        $params['offset'] = $offset;
        
        return $this->query($sql, $params);
    }
    
    public function getLog($logId) {
        $sql = "SELECT * FROM system_logs WHERE id = :logId";
        $params = ['logId' => $logId];
        $result = $this->query($sql, $params);
        return !empty($result) ? $result[0] : null;
    }
    
    public function clearLogs() {
        $result = $this->execute("DELETE FROM system_logs");
        return ['success' => $result !== false];
    }
    
    public function getLogCount($filters = []) {
        $sql = "SELECT COUNT(*) as count FROM system_logs WHERE 1=1";
        $params = [];
        
        if (isset($filters['level'])) {
            $sql .= " AND level = :level";
            $params['level'] = $filters['level'];
        }
        
        if (isset($filters['category'])) {
            $sql .= " AND category = :category";
            $params['category'] = $filters['category'];
        }
        
        if (isset($filters['user_id'])) {
            $sql .= " AND user_id = :userId";
            $params['userId'] = $filters['user_id'];
        }
        
        if (isset($filters['device_id'])) {
            $sql .= " AND device_id = :deviceId";
            $params['deviceId'] = $filters['device_id'];
        }
        
        if (isset($filters['start_time'])) {
            $sql .= " AND created_at >= :startTime";
            $params['startTime'] = $filters['start_time'];
        }
        
        if (isset($filters['end_time'])) {
            $sql .= " AND created_at <= :endTime";
            $params['endTime'] = $filters['end_time'];
        }
        
        $result = $this->query($sql, $params);
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
    
    public function getLogsByUserId($userId, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM system_logs WHERE user_id = :userId ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = ['userId' => $userId, 'limit' => $limit, 'offset' => $offset];
        return $this->query($sql, $params);
    }
    
    public function getLogsByDeviceId($deviceId, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM system_logs WHERE device_id = :deviceId ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = ['deviceId' => $deviceId, 'limit' => $limit, 'offset' => $offset];
        return $this->query($sql, $params);
    }
    
    public function getLogStats() {
        $stats = [];
        
        $result = $this->query("SELECT level, COUNT(*) as count FROM system_logs GROUP BY level");
        foreach ($result as $row) {
            $stats[$row['level']] = intval($row['count']);
        }
        
        return $stats;
    }
    
    public function deleteOldLogs($days = 30) {
        $expireDate = date('Y-m-d H:i:s', strtotime("-$days days"));
        $sql = "DELETE FROM system_logs WHERE created_at < :expireDate";
        $params = ['expireDate' => $expireDate];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
}
?>
