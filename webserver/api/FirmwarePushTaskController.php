<?php
/**
 * 固件推送任务控制器
 */

class FirmwarePushTaskController extends BaseController {
    /**
     * 创建固件推送任务
     */
    public function createPushTask($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_push_task_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/FirmwarePushTask.php';
        $pushTaskModel = new FirmwarePushTask();
        $result = $pushTaskModel->createPushTask(
            $data['firmware_id'],
            $data['target_type'],
            $data['target_ids'],
            $data['strategy'],
            $data['schedule_time'] ?? null,
            $data['description'] ?? '',
            $user['id']
        );
        
        if ($result['success']) {
            $this->response::success('推送任务创建成功', array('task_id' => $result['task_id']));
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取推送任务列表
     */
    public function getPushTasks($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_push_task_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/FirmwarePushTask.php';
        $pushTaskModel = new FirmwarePushTask();
        $tasks = $pushTaskModel->getPushTasks($user['id']);
        
        $this->response::success('获取成功', $tasks);
    }
    
    /**
     * 获取推送任务详情
     */
    public function getPushTask($params) {
        $task_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_push_task_get_detail', array('task_id' => $task_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/FirmwarePushTask.php';
        $pushTaskModel = new FirmwarePushTask();
        $task = $pushTaskModel->getPushTask($task_id);
        
        if (!$task) {
            $this->response::error('推送任务不存在', 404);
        }
        
        // 检查任务是否属于当前用户
        if ($task['created_by'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $this->response::success('获取成功', $task);
    }
    
    /**
     * 更新推送任务状态
     */
    public function updatePushTaskStatus($params) {
        $task_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_push_task_update_status', array('task_id' => $task_id, 'user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/FirmwarePushTask.php';
        $pushTaskModel = new FirmwarePushTask();
        $task = $pushTaskModel->getPushTask($task_id);
        
        if (!$task) {
            $this->response::error('推送任务不存在', 404);
        }
        
        // 检查任务是否属于当前用户
        if ($task['created_by'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $pushTaskModel->updatePushTaskStatus($task_id, $data['status']);
        
        if ($result['success']) {
            $this->response::success('推送任务状态更新成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 删除推送任务
     */
    public function deletePushTask($params) {
        $task_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_push_task_delete', array('task_id' => $task_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/FirmwarePushTask.php';
        $pushTaskModel = new FirmwarePushTask();
        $task = $pushTaskModel->getPushTask($task_id);
        
        if (!$task) {
            $this->response::error('推送任务不存在', 404);
        }
        
        // 检查任务是否属于当前用户
        if ($task['created_by'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $pushTaskModel->deletePushTask($task_id);
        
        if ($result['success']) {
            $this->response::success('推送任务删除成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 添加推送日志
     */
    public function addPushLog($params) {
        $this->logAction('firmware_push_task_add_log');
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/FirmwarePushTask.php';
        $pushTaskModel = new FirmwarePushTask();
        $result = $pushTaskModel->addPushLog(
            $data['task_id'],
            $data['device_id'],
            $data['status'],
            $data['message'] ?? '',
            $data['progress'] ?? 0
        );
        
        if ($result['success']) {
            $this->response::success('推送日志添加成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 批量添加推送日志
     */
    public function batchAddPushLogs($params) {
        $this->logAction('firmware_push_task_batch_add_log');
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/FirmwarePushTask.php';
        $pushTaskModel = new FirmwarePushTask();
        $result = $pushTaskModel->batchAddPushLogs($data['logs']);
        
        if ($result['success']) {
            $this->response::success('批量推送日志添加成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
}
?>