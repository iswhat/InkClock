<?php
/**
 * 系统日志控制器
 */

class SystemLogController extends BaseController {
    /**
     * 获取系统日志列表
     */
    public function getLogs($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('system_log_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/SystemLog.php';
        $logModel = new SystemLog();
        $logs = $logModel->getLogs(
            $this->getQueryParams(array('level', 'start_time', 'end_time', 'module')),
            $user['id']
        );
        
        $this->response::success('获取成功', $logs);
    }
    
    /**
     * 删除旧日志
     */
    public function deleteOldLogs($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('system_log_delete_old', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/SystemLog.php';
        $logModel = new SystemLog();
        $result = $logModel->deleteOldLogs(
            $this->getQueryParams(array('days'))['days'] ?? 30
        );
        
        if ($result['success']) {
            $this->response::success('旧日志删除成功', array('deleted_count' => $result['deleted_count']));
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 记录系统日志
     */
    public function log($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('system_log_write', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/SystemLog.php';
        $logModel = new SystemLog();
        $result = $logModel->addLog(
            $data['level'],
            $data['message'],
            $data['module'] ?? 'system',
            $data['data'] ?? array(),
            $user['id']
        );
        
        if ($result['success']) {
            $this->response::success('日志记录成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
}
?>