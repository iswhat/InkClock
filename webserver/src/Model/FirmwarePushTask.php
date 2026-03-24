<?php
/**
 * 固件推送任务模型
 */

namespace InkClock\Model;

use InkClock\Model\BaseModel;

class FirmwarePushTask extends BaseModel {
    
    public function createPushTask($firmwareId, $targetType, $targetIds, $strategy, $scheduleTime = null, $description = '', $userId) {
        $createdAt = date('Y-m-d H:i:s');
        $status = 'pending';
        $progress = 0;
        $totalDevices = 0;
        $successCount = 0;
        $failedCount = 0;
        
        if ($targetType == 'all') {
            $totalDevices = $this->getTotalDevicesCount($userId);
        } elseif ($targetType == 'group' && $targetIds) {
            $totalDevices = $this->getDevicesCountByGroups($targetIds);
        } elseif ($targetType == 'device_list' && $targetIds) {
            $totalDevices = count($targetIds);
        }
        
        $sql = "INSERT INTO firmware_push_tasks (firmware_id, user_id, target_type, target_ids, status, progress, total_devices, success_count, failed_count, created_at, schedule_time, description, strategy) VALUES (:firmwareId, :userId, :targetType, :targetIds, :status, :progress, :totalDevices, :successCount, :failedCount, :createdAt, :scheduleTime, :description, :strategy)";
        $params = [
            'firmwareId' => $firmwareId,
            'userId' => $userId,
            'targetType' => $targetType,
            'targetIds' => json_encode($targetIds),
            'status' => $status,
            'progress' => $progress,
            'totalDevices' => $totalDevices,
            'successCount' => $successCount,
            'failedCount' => $failedCount,
            'createdAt' => $createdAt,
            'scheduleTime' => $scheduleTime,
            'description' => $description,
            'strategy' => $strategy
        ];
        
        $result = $this->execute($sql, $params);
        $taskId = $this->lastInsertId();
        
        return ['success' => $result !== false, 'task_id' => $taskId];
    }
    
    private function getTotalDevicesCount($userId) {
        $sql = "SELECT COUNT(*) as count FROM devices d JOIN user_devices ud ON d.device_id = ud.device_id WHERE ud.user_id = :userId";
        $params = ['userId' => $userId];
        $result = $this->query($sql, $params);
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
    
    private function getDevicesCountByGroups($groupIds) {
        if (empty($groupIds)) {
            return 0;
        }
        
        $placeholders = implode(',', array_fill(0, count($groupIds), '?'));
        $sql = "SELECT COUNT(DISTINCT device_id) as count FROM device_group_relations WHERE group_id IN ($placeholders)";
        
        $params = [];
        foreach ($groupIds as $i => $groupId) {
            $params['group' . $i] = $groupId;
        }
        
        $result = $this->query($sql, $params);
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
    
    public function getPushTasksByUserId($userId, $limit = 50, $offset = 0) {
        $sql = "SELECT * FROM firmware_push_tasks WHERE user_id = :userId ORDER BY created_at DESC LIMIT :limit OFFSET :offset";
        $params = ['userId' => $userId, 'limit' => $limit, 'offset' => $offset];
        return $this->query($sql, $params);
    }
    
    public function getPushTasks($userId, $limit = 50, $offset = 0) {
        return $this->getPushTasksByUserId($userId, $limit, $offset);
    }
    
    public function getPushTaskById($taskId, $userId = null) {
        $sql = "SELECT * FROM firmware_push_tasks WHERE id = :taskId";
        $params = ['taskId' => $taskId];
        
        if ($userId) {
            $sql .= " AND user_id = :userId";
            $params['userId'] = $userId;
        }
        
        $result = $this->query($sql, $params);
        $task = !empty($result) ? $result[0] : null;
        
        if ($task) {
            $task['logs'] = $this->getPushLogsByTaskId($taskId);
        }
        
        return $task;
    }
    
    public function updateTaskStatus($taskId, $status, $progress = null, $successCount = null, $failedCount = null) {
        $sql = "UPDATE firmware_push_tasks SET status = :status";
        $params = ['status' => $status, 'taskId' => $taskId];
        
        if ($progress !== null) {
            $sql .= ", progress = :progress";
            $params['progress'] = $progress;
        }
        
        if ($successCount !== null) {
            $sql .= ", success_count = :successCount";
            $params['successCount'] = $successCount;
        }
        
        if ($failedCount !== null) {
            $sql .= ", failed_count = :failedCount";
            $params['failedCount'] = $failedCount;
        }
        
        $sql .= " WHERE id = :taskId";
        
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function deleteTask($taskId, $userId) {
        $sql = "DELETE FROM firmware_push_tasks WHERE id = :taskId AND user_id = :userId";
        $params = ['taskId' => $taskId, 'userId' => $userId];
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function getPushLogsByTaskId($taskId) {
        $sql = "SELECT * FROM firmware_push_logs WHERE task_id = :taskId ORDER BY created_at DESC";
        $params = ['taskId' => $taskId];
        return $this->query($sql, $params);
    }
    
    public function addPushLog($taskId, $deviceId, $status, $message = '') {
        $createdAt = date('Y-m-d H:i:s');
        
        $sql = "INSERT INTO firmware_push_logs (task_id, device_id, status, message, created_at) VALUES (:taskId, :deviceId, :status, :message, :createdAt)";
        $params = [
            'taskId' => $taskId,
            'deviceId' => $deviceId,
            'status' => $status,
            'message' => $message,
            'createdAt' => $createdAt
        ];
        
        $result = $this->execute($sql, $params);
        return ['success' => $result !== false];
    }
    
    public function getPendingTasks() {
        $sql = "SELECT * FROM firmware_push_tasks WHERE status = 'pending' AND (schedule_time IS NULL OR schedule_time <= :now) ORDER BY created_at ASC";
        $params = ['now' => date('Y-m-d H:i:s')];
        return $this->query($sql, $params);
    }
    
    public function getRunningTasks() {
        $sql = "SELECT * FROM firmware_push_tasks WHERE status = 'running' ORDER BY created_at ASC";
        return $this->query($sql);
    }
    
    public function getTaskCount($userId) {
        $sql = "SELECT COUNT(*) as count FROM firmware_push_tasks WHERE user_id = :userId";
        $params = ['userId' => $userId];
        $result = $this->query($sql, $params);
        return !empty($result) ? intval($result[0]['count']) : 0;
    }
}
?>
