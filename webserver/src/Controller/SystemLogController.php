<?php
/**
 * 系统日志控制器
 */

namespace App\Controller;

use App\Model\SystemLog;

class SystemLogController extends BaseController {
    /**
     * 获取系统日志列表
     */
    public function getLogs() {
        $user = $this->checkApiPermission(true);
        $this->logAction('system_log_get_list', array('user_id' => $user['id']));
        
        $limit = isset($_GET['limit']) ? (int)$_GET['limit'] : 50;
        $offset = isset($_GET['offset']) ? (int)$_GET['offset'] : 0;
        $level = isset($_GET['level']) ? $_GET['level'] : null;
        $startTime = isset($_GET['start_time']) ? $_GET['start_time'] : null;
        $endTime = isset($_GET['end_time']) ? $_GET['end_time'] : null;
        
        $systemLogModel = new SystemLog($this->db);
        $logs = $systemLogModel->getLogs($limit, $offset, $level, $startTime, $endTime);
        
        $this->response->success('获取成功', $logs);
    }
    
    /**
     * 获取日志详情
     */
    public function getLog($params) {
        $user = $this->checkApiPermission(true);
        $logId = $params['id'];
        $this->logAction('system_log_get_detail', array('user_id' => $user['id'], 'log_id' => $logId));
        
        $systemLogModel = new SystemLog($this->db);
        $log = $systemLogModel->getLog($logId);
        
        if ($log) {
            $this->response->success('获取成功', $log);
        } else {
            $this->response->error('日志不存在', 404);
        }
    }
    
    /**
     * 清除日志
     */
    public function clearLogs() {
        $user = $this->checkApiPermission(true);
        $this->logAction('system_log_clear', array('user_id' => $user['id']));
        
        $systemLogModel = new SystemLog($this->db);
        $result = $systemLogModel->clearLogs();
        
        if ($result['success']) {
            $this->response->success('日志清除成功');
        } else {
            $this->response->error('日志清除失败', 400);
        }
    }
}
?>