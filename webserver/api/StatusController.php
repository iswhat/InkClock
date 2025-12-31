<?php
/**
 * 状态检查控制器
 */

require_once __DIR__ . '/BaseController.php';

class StatusController extends BaseController {
    /**
     * 获取系统状态
     */
    public function getStatus($params) {
        $this->logAction('status_check');
        
        // 获取系统信息
        $status = array(
            'status' => 'ok',
            'time' => date('Y-m-d H:i:s'),
            'version' => 'v1.0',
            'environment' => 'development'
        );
        
        // 获取数据库信息
        $result = $this->db->query("SELECT DATABASE() as db_name, VERSION() as db_version");
        if ($result && $row = $result->fetch_assoc()) {
            $status['database'] = array(
                'name' => $row['db_name'],
                'version' => $row['db_version']
            );
        }
        
        // 获取PHP信息
        $status['php'] = array(
            'version' => PHP_VERSION,
            'extensions' => get_loaded_extensions()
        );
        
        $this->response::success($status);
    }
}
?>