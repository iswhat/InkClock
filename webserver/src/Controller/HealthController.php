<?php
/**
 * 健康检查控制器
 * 用于监控系统状态和API可用性
 */

namespace InkClock\Controller;

class HealthController extends BaseController {
    /**
     * 健康检查
     */
    public function check($params) {
        $this->logAction('health_check');
        
        // 检查系统状态
        $systemStatus = $this->checkSystemStatus();
        
        // 检查数据库连接
        $dbStatus = $this->checkDatabaseStatus();
        
        // 检查API网关
        $gatewayStatus = $this->checkGatewayStatus();
        
        // 构建健康状态响应
        $status = $systemStatus['ok'] && $dbStatus['ok'] && $gatewayStatus['ok'] ? 'healthy' : 'unhealthy';
        $statusCode = $status === 'healthy' ? 200 : 503;
        
        $response = [
            'status' => $status,
            'timestamp' => time(),
            'checks' => [
                'system' => $systemStatus,
                'database' => $dbStatus,
                'gateway' => $gatewayStatus
            ],
            'metrics' => [
                'uptime' => $this->getSystemUptime(),
                'memory_usage' => $this->getMemoryUsage(),
                'load_avg' => $this->getLoadAverage()
            ]
        ];
        
        $this->response->json($response, $statusCode);
    }
    
    /**
     * 检查系统状态
     * @return array 系统状态
     */
    private function checkSystemStatus() {
        try {
            // 检查PHP版本
            $phpVersion = phpversion();
            
            // 检查磁盘空间
            $diskSpace = disk_free_space(__DIR__);
            $diskTotal = disk_total_space(__DIR__);
            $diskUsage = round(($diskTotal - $diskSpace) / $diskTotal * 100, 2);
            
            // 检查系统负载
            $loadAvg = $this->getLoadAverage();
            
            return [
                'ok' => true,
                'php_version' => $phpVersion,
                'disk_usage' => $diskUsage . '%',
                'disk_free' => $this->formatBytes($diskSpace),
                'load_avg' => $loadAvg
            ];
        } catch (\Exception $e) {
            return [
                'ok' => false,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * 检查数据库连接
     * @return array 数据库状态
     */
    private function checkDatabaseStatus() {
        try {
            if ($this->db) {
                try {
                    // 检查数据库对象是否有效
                    return [
                        'ok' => true,
                        'message' => '数据库连接对象已初始化'
                    ];
                } catch (\Exception $e) {
                    return [
                        'ok' => false,
                        'error' => $e->getMessage()
                    ];
                }
            } else {
                return [
                    'ok' => false,
                    'error' => '数据库连接未初始化'
                ];
            }
        } catch (\Exception $e) {
            return [
                'ok' => false,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * 检查API网关状态
     * @return array API网关状态
     */
    private function checkGatewayStatus() {
        try {
            // 检查API网关控制器是否存在
            $gatewayControllerExists = class_exists('InkClock\\Controller\\ApiGatewayController');
            
            return [
                'ok' => $gatewayControllerExists,
                'message' => $gatewayControllerExists ? 'API网关可用' : 'API网关控制器不存在'
            ];
        } catch (\Exception $e) {
            return [
                'ok' => false,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * 获取系统运行时间
     * @return string 系统运行时间
     */
    private function getSystemUptime() {
        if (file_exists('/proc/uptime')) {
            $uptime = trim(file_get_contents('/proc/uptime'));
            $uptime = explode(' ', $uptime)[0];
            return round($uptime, 2) . ' seconds';
        }
        return 'N/A';
    }
    
    /**
     * 获取内存使用情况
     * @return array 内存使用情况
     */
    private function getMemoryUsage() {
        return [
            'used' => $this->formatBytes(memory_get_usage()),
            'peak' => $this->formatBytes(memory_get_peak_usage()),
            'limit' => $this->formatBytes(ini_get('memory_limit') === '-1' ? PHP_INT_MAX : $this->convertToBytes(ini_get('memory_limit')))
        ];
    }
    
    /**
     * 获取系统负载
     * @return array 系统负载
     */
    private function getLoadAverage() {
        if (function_exists('sys_getloadavg')) {
            $load = sys_getloadavg();
            return [
                '1min' => $load[0],
                '5min' => $load[1],
                '15min' => $load[2]
            ];
        }
        return ['1min' => 'N/A', '5min' => 'N/A', '15min' => 'N/A'];
    }
    
    /**
     * 格式化字节数
     * @param int $bytes 字节数
     * @return string 格式化后的字符串
     */
    private function formatBytes($bytes) {
        $units = ['B', 'KB', 'MB', 'GB', 'TB'];
        $bytes = max($bytes, 0);
        $pow = floor(($bytes ? log($bytes) : 0) / log(1024));
        $pow = min($pow, count($units) - 1);
        $bytes /= (1 << (10 * $pow));
        return round($bytes, 2) . ' ' . $units[$pow];
    }
    
    /**
     * 转换内存限制字符串为字节数
     * @param string $val 内存限制字符串
     * @return int 字节数
     */
    private function convertToBytes($val) {
        $val = trim($val);
        $last = strtolower($val[strlen($val)-1]);
        $val = substr($val, 0, -1);
        switch($last) {
            case 'g':
                $val *= 1024;
            case 'm':
                $val *= 1024;
            case 'k':
                $val *= 1024;
        }
        return $val;
    }
}
?>