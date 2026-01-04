<?php
/**
 * 状态控制器
 */

namespace InkClock\Controller;

class StatusController extends BaseController {
    /**
     * 获取系统状态
     */
    public function getStatus() {
        $this->logAction('status_get');
        
        $status = array(
            'status' => 'ok',
            'timestamp' => time(),
            'version' => '1.0.0',
            'services' => array(
                'database' => $this->checkDatabaseConnection(),
                'memory' => $this->checkMemoryUsage(),
                'disk' => $this->checkDiskSpace()
            )
        );
        
        $this->response->success('获取成功', $status);
    }
    
    /**
     * 检查数据库连接
     */
    private function checkDatabaseConnection() {
        try {
            // 使用现有的数据库连接进行简单查询
            $result = $this->db->query("SELECT 1");
            return $result !== false ? 'ok' : 'error';
        } catch (\Exception) {
            return 'error';
        }
    }
    
    /**
     * 检查内存使用情况
     */
    private function checkMemoryUsage() {
        $usage = memory_get_usage(true);
        $limit = memory_get_peak_usage(true);
        
        return array(
            'current' => $usage,
            'peak' => $limit,
            'unit' => 'bytes'
        );
    }
    
    /**
     * 检查磁盘空间
     */
    private function checkDiskSpace() {
        $disk = disk_free_space('.');
        $total = disk_total_space('.');
        $used = $total - $disk;
        $percent = $total > 0 ? round(($used / $total) * 100) : 0;
        
        return array(
            'total' => $total,
            'used' => $used,
            'free' => $disk,
            'percent' => $percent
        );
    }
}
?>