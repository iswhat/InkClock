<?php
/**
 * 固件推送任务控制器
 */

namespace InkClock\Controller;

use InkClock\Model\FirmwarePushTask;

class FirmwarePushTaskController extends BaseController {
    /**
     * 获取固件推送任务列表
     */
    public function getTasks($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_push_task_get_list', array('user_id' => $user['id']));
        
        $limit = isset($_GET['limit']) ? (int)$_GET['limit'] : 50;
        $offset = isset($_GET['offset']) ? (int)$_GET['offset'] : 0;
        
        $pushTaskModel = new FirmwarePushTask($this->db);
        $tasks = $pushTaskModel->getPushTasks($user['id'], $limit, $offset);
        
        $this->response->success('获取成功', $tasks);
    }
    
    /**
     * 获取固件推送任务详情
     */
    public function getTask($params) {
        $user = $this->checkApiPermission(true);
        $taskId = $params['id'];
        $this->logAction('firmware_push_task_get_detail', array('user_id' => $user['id'], 'task_id' => $taskId));
        
        $pushTaskModel = new FirmwarePushTask($this->db);
        $task = $pushTaskModel->getPushTaskById($taskId, $user['id']);
        
        if ($task) {
            $this->response->success('获取成功', $task);
        } else {
            $this->response->error('任务不存在', 404);
        }
    }
    
    /**
     * 创建固件推送任务
     */
    public function createTask($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('firmware_push_task_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        $pushTaskModel = new FirmwarePushTask($this->db);
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
            $this->response->success('创建成功', array('task_id' => $result['task_id']));
        } else {
            $this->response->error('创建失败', 400);
        }
    }
    
    /**
     * 删除固件推送任务
     */
    public function deleteTask($params) {
        $user = $this->checkApiPermission(true);
        $taskId = $params['id'];
        $this->logAction('firmware_push_task_delete', array('user_id' => $user['id'], 'task_id' => $taskId));
        
        $pushTaskModel = new FirmwarePushTask($this->db);
        $result = $pushTaskModel->deletePushTask($taskId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error('删除失败', 400);
        }
    }
}
?>