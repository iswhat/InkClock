<?php
/**
 * 状态控制器
 */

namespace InkClock\Controller;

class StatusController {
    /**
     * 获取系统状态
     */
    public function getStatus($params = array()) {
        $status = array(
            'status' => 'ok',
            'timestamp' => time(),
            'version' => '1.0.0',
            'services' => array(
                'memory' => $this->checkMemoryUsage(),
                'disk' => $this->checkDiskSpace(),
                'database' => $this->checkDatabaseConnection()
            )
        );
        
        return array(
            'success' => true,
            'message' => '获取成功',
            'data' => $status
        );
    }
    
    /**
     * 检查数据库连接
     */
    private function checkDatabaseConnection() {
        try {
            // 检查SQLite3扩展是否启用
            if (!class_exists('SQLite3')) {
                return 'sqlite3_extension_missing';
            }
            
            // 尝试创建简单的内存数据库连接
            $db = new \SQLite3(':memory:');
            $result = $db->query("SELECT 1");
            $db->close();
            return $result !== false ? 'ok' : 'error';
        } catch (\Exception $e) {
            return 'error: ' . $e->getMessage();
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