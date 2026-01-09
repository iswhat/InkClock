<?php
/**
 * 系统控制器
 */

namespace InkClock\Controller;

class SystemController extends BaseController {
    /**
     * 获取系统设置
     */
    public function getSettings($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $this->logAction('get_system_settings', array('admin_id' => $user['id']));
        
        // 从配置文件或数据库获取系统设置
        $settings = $this->getSystemSettings();
        
        $this->response->success('获取成功', $settings);
    }
    
    /**
     * 更新系统设置
     */
    public function updateSettings($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $data = $this->parseRequestBody();
        
        $this->logAction('update_system_settings', array('admin_id' => $user['id'], 'settings' => $data));
        
        // 验证输入
        if (!isset($data['system_name']) || !isset($data['system_domain']) || !isset($data['system_port'])) {
            $this->response->error('缺少必要参数', 400);
        }
        
        // 更新系统设置
        $result = $this->saveSystemSettings($data);
        
        if ($result) {
            $this->response->success('更新成功');
        } else {
            $this->response->error('更新失败', 500);
        }
    }
    
    /**
     * 重置系统设置为默认值
     */
    public function resetSettings($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $this->logAction('reset_system_settings', array('admin_id' => $user['id']));
        
        // 重置系统设置为默认值
        $result = $this->resetSystemSettingsToDefault();
        
        if ($result) {
            $this->response->success('重置成功');
        } else {
            $this->response->error('重置失败', 500);
        }
    }
    
    /**
     * 获取系统信息
     */
    public function getSystemInfo($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $this->logAction('get_system_info', array('admin_id' => $user['id']));
        
        // 获取系统信息
        $info = $this->getSystemInfoData();
        
        $this->response->success('获取成功', $info);
    }
    
    /**
     * 备份数据库
     */
    public function backupDatabase($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $this->logAction('backup_database', array('admin_id' => $user['id']));
        
        // 备份数据库
        try {
            $dbPath = __DIR__ . '/../../data/database.db';
            
            if (!file_exists($dbPath)) {
                $this->response->error('数据库文件不存在', 404);
            }
            
            // 读取数据库文件
            $dbContent = file_get_contents($dbPath);
            
            // 设置响应头
            header('Content-Type: application/octet-stream');
            header('Content-Disposition: attachment; filename="inkclock_backup_' . date('Y-m-d') . '.db"');
            header('Content-Length: ' . strlen($dbContent));
            
            // 输出数据库内容
            echo $dbContent;
            exit;
        } catch (\Exception $e) {
            $this->response->error('备份失败: ' . $e->getMessage(), 500);
        }
    }
    
    /**
     * 重启系统
     */
    public function restartSystem($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $this->logAction('restart_system', array('admin_id' => $user['id']));
        
        // 这里只是模拟重启，实际环境中可能需要执行不同的操作
        // 例如重启PHP-FPM、Apache或Nginx等服务
        
        $this->response->success('重启命令已发送，请等待系统重启完成');
    }
    
    /**
     * 清理缓存
     */
    public function clearCache($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $this->logAction('clear_cache', array('admin_id' => $user['id']));
        
        // 清理缓存
        try {
            $cachePath = __DIR__ . '/../../data/cache';
            
            if (is_dir($cachePath)) {
                $this->deleteDirectory($cachePath);
                mkdir($cachePath, 0755, true);
            }
            
            $this->response->success('缓存清理成功');
        } catch (\Exception $e) {
            $this->response->error('清理失败: ' . $e->getMessage(), 500);
        }
    }
    
    /**
     * 刷新系统
     */
    public function refreshSystem($params) {
        $this->logAction('system_refresh');
        
        return [
            'success' => true,
            'message' => '系统刷新成功',
            'timestamp' => time()
        ];
    }
    
    /**
     * 获取系统设置
     * @return array 系统设置
     */
    private function getSystemSettings() {
        // 从配置文件或数据库获取系统设置
        // 这里使用默认值，实际环境中应该从配置文件或数据库读取
        return array(
            'system_name' => 'InkClock',
            'system_domain' => 'localhost',
            'system_port' => 8000,
            'admin_email' => '',
            'api_timeout' => 30,
            'max_devices_per_user' => 10,
            'session_timeout' => 30,
            'log_level' => 'info'
        );
    }
    
    /**
     * 保存系统设置
     * @param array $settings 系统设置
     * @return bool 保存结果
     */
    private function saveSystemSettings($settings) {
        // 保存系统设置到配置文件或数据库
        // 这里只是模拟保存，实际环境中应该保存到配置文件或数据库
        return true;
    }
    
    /**
     * 重置系统设置为默认值
     * @return bool 重置结果
     */
    private function resetSystemSettingsToDefault() {
        // 重置系统设置为默认值
        // 这里只是模拟重置，实际环境中应该重置配置文件或数据库中的设置
        return true;
    }
    
    /**
     * 获取系统信息
     * @return array 系统信息
     */
    private function getSystemInfoData() {
        return array(
            'system_version' => '1.0.0',
            'database_type' => 'SQLite',
            'php_version' => PHP_VERSION,
            'uptime' => '0 天',
            'server_time' => date('Y-m-d H:i:s'),
            'server_software' => $_SERVER['SERVER_SOFTWARE'] ?? '',
            'os' => PHP_OS
        );
    }
    
    /**
     * 删除目录
     * @param string $dir 目录路径
     */
    private function deleteDirectory($dir) {
        if (!is_dir($dir)) {
            return;
        }
        
        $files = scandir($dir);
        foreach ($files as $file) {
            if ($file != '.' && $file != '..') {
                $filePath = $dir . '/' . $file;
                if (is_dir($filePath)) {
                    $this->deleteDirectory($filePath);
                } else {
                    unlink($filePath);
                }
            }
        }
        
        rmdir($dir);
    }
}
?>