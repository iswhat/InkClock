<?php
/**
 * 系统日志模型
 */

namespace InkClock\Model;

class SystemLog {
    private $db;
    
    /**
     * 构造函数
     * @param \SQLite3 $db 数据库连接
     */
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 记录系统日志
     */
    public function log($level, $category, $message, $userId = null, $deviceId = null) {
        $createdAt = date('Y-m-d H:i:s');
        $ipAddress = $_SERVER['REMOTE_ADDR'] ?? '';
        
        $stmt = $this->db->prepare("INSERT INTO system_logs (level, category, message, user_id, device_id, ip_address, created_at) VALUES (:level, :category, :message, :userId, :deviceId, :ipAddress, :createdAt)");
        $stmt->bindValue(':level', $level, SQLITE3_TEXT);
        $stmt->bindValue(':category', $category, SQLITE3_TEXT);
        $stmt->bindValue(':message', $message, SQLITE3_TEXT);
        $stmt->bindValue(':userId', $userId, SQLITE3_INTEGER);
        $stmt->bindValue(':deviceId', $deviceId, SQLITE3_TEXT);
        $stmt->bindValue(':ipAddress', $ipAddress, SQLITE3_TEXT);
        $stmt->bindValue(':createdAt', $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        $logId = $this->db->lastInsertRowID();
        
        return array('success' => $result !== false, 'log_id' => $logId);
    }
    
    /**
     * 获取系统日志列表
     */
    public function getLogs($limit = 50, $offset = 0, $level = null, $startTime = null, $endTime = null) {
        $query = "SELECT * FROM system_logs WHERE 1=1";
        $params = array();
        $conditions = array();
        
        // 应用过滤条件
        if ($level) {
            $conditions[] = "level = :level";
            $params[':level'] = $level;
        }
        
        if ($startTime) {
            $conditions[] = "created_at >= :startTime";
            $params[':startTime'] = $startTime;
        }
        
        if ($endTime) {
            $conditions[] = "created_at <= :endTime";
            $params[':endTime'] = $endTime;
        }
        
        // 添加条件
        if (!empty($conditions)) {
            $query .= " AND " . implode(" AND ", $conditions);
        }
        
        // 排序和分页
        $query .= " ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params[':limit'] = $limit;
        $params[':offset'] = $offset;
        
        // 执行查询
        $stmt = $this->db->prepare($query);
        
        // 绑定参数
        foreach ($params as $key => $value) {
            if (is_int($value)) {
                $stmt->bindValue($key, $value, SQLITE3_INTEGER);
            } else {
                $stmt->bindValue($key, $value, SQLITE3_TEXT);
            }
        }
        
        $result = $stmt->execute();
        $logs = array();
        
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $logs[] = $row;
        }
        
        return $logs;
    }
    
    /**
     * 获取单个日志详情
     */
    public function getLog($logId) {
        $stmt = $this->db->prepare("SELECT * FROM system_logs WHERE id = :logId");
        $stmt->bindValue(':logId', $logId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        return $result->fetchArray(SQLITE3_ASSOC);
    }
    
    /**
     * 清除日志
     */
    public function clearLogs() {
        $result = $this->db->exec("DELETE FROM system_logs");
        return array('success' => $result !== false);
    }
    
    /**
     * 获取日志数量统计
     */
    public function getLogCount($filters = array()) {
        $query = "SELECT COUNT(*) as count FROM system_logs WHERE 1=1";
        $params = array();
        $conditions = array();
        
        // 应用过滤条件
        if (isset($filters['level'])) {
            $conditions[] = "level = :level";
            $params[':level'] = $filters['level'];
        }
        
        if (isset($filters['category'])) {
            $conditions[] = "category = :category";
            $params[':category'] = $filters['category'];
        }
        
        if (isset($filters['user_id'])) {
            $conditions[] = "user_id = :userId";
            $params[':userId'] = $filters['user_id'];
        }
        
        if (isset($filters['device_id'])) {
            $conditions[] = "device_id = :deviceId";
            $params[':deviceId'] = $filters['device_id'];
        }
        
        if (isset($filters['start_time'])) {
            $conditions[] = "created_at >= :startTime";
            $params[':startTime'] = $filters['start_time'];
        }
        
        if (isset($filters['end_time'])) {
            $conditions[] = "created_at <= :endTime";
            $params[':endTime'] = $filters['end_time'];
        }
        
        // 添加条件
        if (!empty($conditions)) {
            $query .= " AND " . implode(" AND ", $conditions);
        }
        
        // 执行查询
        $stmt = $this->db->prepare($query);
        
        // 绑定参数
        foreach ($params as $key => $value) {
            if (is_int($value)) {
                $stmt->bindValue($key, $value, SQLITE3_INTEGER);
            } else {
                $stmt->bindValue($key, $value, SQLITE3_TEXT);
            }
        }
        
        $result = $stmt->execute();
        $row = $result->fetchArray(SQLITE3_ASSOC);
        
        return $row['count'];
    }
    
    /**
     * 删除旧日志（按时间）
     */
    public function deleteOldLogs($days) {
        $cutoffDate = date('Y-m-d H:i:s', strtotime("-$days days"));
        
        $stmt = $this->db->prepare("DELETE FROM system_logs WHERE created_at < :cutoffDate");
        $stmt->bindValue(':cutoffDate', $cutoffDate, SQLITE3_TEXT);
        $result = $stmt->execute();
        $affectedRows = $this->db->changes();
        
        return array('success' => $result !== false, 'deleted_count' => $affectedRows);
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
