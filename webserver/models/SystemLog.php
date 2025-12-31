<?php
/**
 * 系统日志模型
 */
require_once __DIR__ . '/../utils/Database.php';

class SystemLog {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
    }
    
    /**
     * 记录系统日志
     */
    public function log($level, $category, $message, $userId = null, $deviceId = null) {
        $createdAt = date('Y-m-d H:i:s');
        $ipAddress = $_SERVER['REMOTE_ADDR'];
        
        $stmt = $this->db->prepare("INSERT INTO system_logs (level, category, message, user_id, device_id, ip_address, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)");
        $stmt->bind_param("sssssss", $level, $category, $message, $userId, $deviceId, $ipAddress, $createdAt);
        $result = $stmt->execute();
        $logId = $this->db->insert_id;
        $stmt->close();
        
        return array('success' => $result, 'log_id' => $logId);
    }
    
    /**
     * 获取系统日志列表
     */
    public function getLogs($filters = array(), $limit = 50, $offset = 0) {
        $query = "SELECT * FROM system_logs WHERE 1=1";
        $params = array();
        $types = "";
        
        // 应用过滤条件
        if (isset($filters['level'])) {
            $query .= " AND level = ?";
            $params[] = $filters['level'];
            $types .= "s";
        }
        
        if (isset($filters['category'])) {
            $query .= " AND category = ?";
            $params[] = $filters['category'];
            $types .= "s";
        }
        
        if (isset($filters['user_id'])) {
            $query .= " AND user_id = ?";
            $params[] = $filters['user_id'];
            $types .= "i";
        }
        
        if (isset($filters['device_id'])) {
            $query .= " AND device_id = ?";
            $params[] = $filters['device_id'];
            $types .= "s";
        }
        
        if (isset($filters['start_time'])) {
            $query .= " AND created_at >= ?";
            $params[] = $filters['start_time'];
            $types .= "s";
        }
        
        if (isset($filters['end_time'])) {
            $query .= " AND created_at <= ?";
            $params[] = $filters['end_time'];
            $types .= "s";
        }
        
        // 排序和分页
        $query .= " ORDER BY created_at DESC LIMIT ? OFFSET ?";
        $params[] = $limit;
        $params[] = $offset;
        $types .= "ii";
        
        // 执行查询
        $stmt = $this->db->prepare($query);
        if (!empty($params)) {
            $stmt->bind_param($types, ...$params);
        }
        $stmt->execute();
        $result = $stmt->get_result();
        $logs = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        
        return $logs;
    }
    
    /**
     * 获取日志数量统计
     */
    public function getLogCount($filters = array()) {
        $query = "SELECT COUNT(*) as count FROM system_logs WHERE 1=1";
        $params = array();
        $types = "";
        
        // 应用过滤条件
        if (isset($filters['level'])) {
            $query .= " AND level = ?";
            $params[] = $filters['level'];
            $types .= "s";
        }
        
        if (isset($filters['category'])) {
            $query .= " AND category = ?";
            $params[] = $filters['category'];
            $types .= "s";
        }
        
        if (isset($filters['user_id'])) {
            $query .= " AND user_id = ?";
            $params[] = $filters['user_id'];
            $types .= "i";
        }
        
        if (isset($filters['device_id'])) {
            $query .= " AND device_id = ?";
            $params[] = $filters['device_id'];
            $types .= "s";
        }
        
        if (isset($filters['start_time'])) {
            $query .= " AND created_at >= ?";
            $params[] = $filters['start_time'];
            $types .= "s";
        }
        
        if (isset($filters['end_time'])) {
            $query .= " AND created_at <= ?";
            $params[] = $filters['end_time'];
            $types .= "s";
        }
        
        // 执行查询
        $stmt = $this->db->prepare($query);
        if (!empty($params)) {
            $stmt->bind_param($types, ...$params);
        }
        $stmt->execute();
        $result = $stmt->get_result();
        $count = $result->fetch_assoc()['count'];
        $stmt->close();
        
        return $count;
    }
    
    /**
     * 删除旧日志（按时间）
     */
    public function deleteOldLogs($days) {
        $cutoffDate = date('Y-m-d H:i:s', strtotime("-$days days"));
        
        $stmt = $this->db->prepare("DELETE FROM system_logs WHERE created_at < ?");
        $stmt->bind_param("s", $cutoffDate);
        $result = $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $result, 'deleted_count' => $affectedRows);
    }
    
    /**
     * 记录info级别的日志
     */
    public function info($category, $message, $userId = null, $deviceId = null) {
        return $this->log('info', $category, $message, $userId, $deviceId);
    }
    
    /**
     * 记录warning级别的日志
     */
    public function warning($category, $message, $userId = null, $deviceId = null) {
        return $this->log('warning', $category, $message, $userId, $deviceId);
    }
    
    /**
     * 记录error级别的日志
     */
    public function error($category, $message, $userId = null, $deviceId = null) {
        return $this->log('error', $category, $message, $userId, $deviceId);
    }
    
    /**
     * 记录critical级别的日志
     */
    public function critical($category, $message, $userId = null, $deviceId = null) {
        return $this->log('critical', $category, $message, $userId, $deviceId);
    }
}
?>