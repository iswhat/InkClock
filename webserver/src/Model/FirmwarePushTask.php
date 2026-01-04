<?php
/**
 * 固件推送任务模型
 */
namespace InkClock\Model;

use InkClock\Utils\Database;

class FirmwarePushTask {
    private $db;
    
    public function __construct($db) {
        $this->db = $db;
    }
    
    /**
     * 创建固件推送任务（控制器调用的方法）
     */
    public function createPushTask($firmwareId, $targetType, $targetIds, $strategy, $scheduleTime = null, $description = '', $userId) {
        $createdAt = date('Y-m-d H:i:s');
        $status = 'pending';
        $progress = 0;
        $totalDevices = 0;
        $successCount = 0;
        $failedCount = 0;
        
        // 计算目标设备数量
        if ($targetType == 'all') {
            // 所有设备
            $totalDevices = $this->getTotalDevicesCount($userId);
        } elseif ($targetType == 'group' && $targetIds) {
            // 按分组推送
            $totalDevices = $this->getDevicesCountByGroups($targetIds);
        } elseif ($targetType == 'device_list' && $targetIds) {
            // 按设备列表推送
            $totalDevices = count($targetIds);
        }
        
        $stmt = $this->db->prepare("INSERT INTO firmware_push_tasks (firmware_id, user_id, target_type, target_ids, status, progress, total_devices, success_count, failed_count, created_at, schedule_time, description, strategy) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        $stmt->bindValue(1, $firmwareId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $stmt->bindValue(3, $targetType, SQLITE3_TEXT);
        $stmt->bindValue(4, json_encode($targetIds), SQLITE3_TEXT);
        $stmt->bindValue(5, $status, SQLITE3_TEXT);
        $stmt->bindValue(6, $progress, SQLITE3_INTEGER);
        $stmt->bindValue(7, $totalDevices, SQLITE3_INTEGER);
        $stmt->bindValue(8, $successCount, SQLITE3_INTEGER);
        $stmt->bindValue(9, $failedCount, SQLITE3_INTEGER);
        $stmt->bindValue(10, $createdAt, SQLITE3_TEXT);
        $stmt->bindValue(11, $scheduleTime, $scheduleTime ? SQLITE3_TEXT : SQLITE3_NULL);
        $stmt->bindValue(12, $description, SQLITE3_TEXT);
        $stmt->bindValue(13, $strategy, SQLITE3_TEXT);
        $stmt->execute();
        
        $taskId = $this->db->lastInsertRowID();
        $stmt->close();
        
        return array(
            'success' => true,
            'task_id' => $taskId
        );
    }
    
    /**
     * 获取用户的所有设备数量
     */
    private function getTotalDevicesCount($userId) {
        $query = "SELECT COUNT(*) as count FROM devices d ".
                 "JOIN user_devices ud ON d.device_id = ud.device_id ".
                 "WHERE ud.user_id = ?";
        
        $stmt = $this->db->prepare($query);
        $stmt->bindValue(1, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $count = $result->fetchArray(SQLITE3_ASSOC)['count'];
        $stmt->close();
        return $count;
    }
    
    /**
     * 获取多个分组的设备总数
     */
    private function getDevicesCountByGroups($groupIds) {
        if (empty($groupIds)) {
            return 0;
        }
        
        $placeholders = implode(',', array_fill(0, count($groupIds), '?'));
        $query = "SELECT COUNT(DISTINCT device_id) as count FROM device_group_relations WHERE group_id IN ($placeholders)";
        
        $stmt = $this->db->prepare($query);
        for ($i = 0; $i < count($groupIds); $i++) {
            $stmt->bindValue($i + 1, $groupIds[$i], SQLITE3_INTEGER);
        }
        $result = $stmt->execute();
        $count = $result->fetchArray(SQLITE3_ASSOC)['count'];
        $stmt->close();
        return $count;
    }
    
    /**
     * 获取推送任务列表
     */
    public function getPushTasksByUserId($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM firmware_push_tasks WHERE user_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bindValue(1, $userId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $limit, SQLITE3_INTEGER);
        $stmt->bindValue(3, $offset, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $tasks = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $tasks[] = $row;
        }
        $stmt->close();
        
        return $tasks;
    }
    
    /**
     * 获取推送任务列表（控制器调用的方法名）
     */
    public function getPushTasks($userId, $limit = 50, $offset = 0) {
        return $this->getPushTasksByUserId($userId, $limit, $offset);
    }
    
    /**
     * 获取推送任务详情
     */
    public function getPushTaskById($taskId, $userId = null) {
        $query = "SELECT * FROM firmware_push_tasks WHERE id = ?";
        $params = array($taskId);
        
        if ($userId) {
            $query .= " AND user_id = ?";
            $params[] = $userId;
        }
        
        $stmt = $this->db->prepare($query);
        for ($i = 0; $i < count($params); $i++) {
            $stmt->bindValue($i + 1, $params[$i], SQLITE3_INTEGER);
        }
        $result = $stmt->execute();
        $task = $result->fetchArray(SQLITE3_ASSOC);
        $stmt->close();
        
        if ($task) {
            // 获取任务的日志信息
            $task['logs'] = $this->getPushLogsByTaskId($taskId);
        }
        
        return $task;
    }
    
    /**
     * 更新推送任务状态
     */
    public function updatePushTaskStatus($taskId, $status, $progress = null, $successCount = null, $failedCount = null) {
        $query = "UPDATE firmware_push_tasks SET status = ?";
        $params = array($status);
        $types = [SQLITE3_TEXT];
        
        if ($progress !== null) {
            $query .= ", progress = ?";
            $params[] = $progress;
            $types[] = SQLITE3_INTEGER;
        }
        
        if ($successCount !== null) {
            $query .= ", success_count = ?";
            $params[] = $successCount;
            $types[] = SQLITE3_INTEGER;
        }
        
        if ($failedCount !== null) {
            $query .= ", failed_count = ?";
            $params[] = $failedCount;
            $types[] = SQLITE3_INTEGER;
        }
        
        if ($status == 'running' && $progress == 0) {
            // 任务开始运行
            $query .= ", started_at = ?";
            $params[] = date('Y-m-d H:i:s');
            $types[] = SQLITE3_TEXT;
        } elseif ($status == 'completed' || $status == 'failed') {
            // 任务完成或失败
            $query .= ", completed_at = ?";
            $params[] = date('Y-m-d H:i:s');
            $types[] = SQLITE3_TEXT;
        }
        
        $query .= " WHERE id = ?";
        $params[] = $taskId;
        $types[] = SQLITE3_INTEGER;
        
        $stmt = $this->db->prepare($query);
        for ($i = 0; $i < count($params); $i++) {
            $stmt->bindValue($i + 1, $params[$i], $types[$i]);
        }
        $result = $stmt->execute();
        
        $affectedRows = $result !== false ? 1 : 0;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 添加推送日志
     */
    public function addPushLog($taskId, $deviceId, $status, $errorMessage = null) {
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO firmware_push_logs (push_task_id, device_id, status, error_message, created_at) VALUES (?, ?, ?, ?, ?)");
        $stmt->bindValue(1, $taskId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $deviceId, SQLITE3_INTEGER);
        $stmt->bindValue(3, $status, SQLITE3_TEXT);
        $stmt->bindValue(4, $errorMessage, $errorMessage ? SQLITE3_TEXT : SQLITE3_NULL);
        $stmt->bindValue(5, $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        
        $logId = $this->db->lastInsertRowID();
        $stmt->close();
        
        // 更新任务统计信息
        $this->updateTaskStats($taskId, $status);
        
        return array(
            'success' => true,
            'log_id' => $logId,
            'created_at' => $createdAt
        );
    }
    
    /**
     * 更新任务统计信息
     */
    private function updateTaskStats($taskId, $status) {
        // 获取当前任务统计
        $task = $this->getPushTaskById($taskId);
        
        if (!$task) {
            return;
        }
        
        // 更新统计信息
        $successCount = $task['success_count'];
        $failedCount = $task['failed_count'];
        
        if ($status == 'success') {
            $successCount++;
        } elseif ($status == 'failed') {
            $failedCount++;
        }
        
        // 计算进度
        $totalDevices = $task['total_devices'];
        $progress = $totalDevices > 0 ? round(($successCount + $failedCount) / $totalDevices * 100) : 100;
        
        // 更新任务状态
        $this->updatePushTaskStatus($taskId, $task['status'], $progress, $successCount, $failedCount);
    }
    
    /**
     * 获取推送任务日志
     */
    public function getPushLogsByTaskId($taskId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM firmware_push_logs WHERE push_task_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bindValue(1, $taskId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $limit, SQLITE3_INTEGER);
        $stmt->bindValue(3, $offset, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $logs = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $logs[] = $row;
        }
        $stmt->close();
        
        return $logs;
    }
    
    /**
     * 删除推送任务
     */
    public function deletePushTask($taskId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM firmware_push_tasks WHERE id = ? AND user_id = ?");
        $stmt->bindValue(1, $taskId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        
        $affectedRows = $result !== false ? 1 : 0;
        $stmt->close();
        
        return array('success' => $affectedRows > 0);
    }
    
    /**
     * 批量添加推送日志
     */
    public function batchAddPushLogs($taskId, $logs) {
        $successCount = 0;
        $createdAt = date('Y-m-d H:i:s');
        
        $stmt = $this->db->prepare("INSERT INTO firmware_push_logs (push_task_id, device_id, status, error_message, created_at) VALUES (?, ?, ?, ?, ?)");
        
        foreach ($logs as $log) {
            $deviceId = $log['device_id'];
            $status = $log['status'];
            $errorMessage = isset($log['error_message']) ? $log['error_message'] : null;
            
            $stmt->reset();
            $stmt->bindValue(1, $taskId, SQLITE3_INTEGER);
            $stmt->bindValue(2, $deviceId, SQLITE3_INTEGER);
            $stmt->bindValue(3, $status, SQLITE3_TEXT);
            $stmt->bindValue(4, $errorMessage, $errorMessage ? SQLITE3_TEXT : SQLITE3_NULL);
            $stmt->bindValue(5, $createdAt, SQLITE3_TEXT);
            if ($stmt->execute()) {
                $successCount++;
            }
        }
        
        $stmt->close();
        
        // 更新任务统计信息
        foreach ($logs as $log) {
            $this->updateTaskStats($taskId, $log['status']);
        }
        
        return array(
            'success' => true,
            'success_count' => $successCount,
            'total_count' => count($logs)
        );
    }
}
?>