<?php
/**
 * 配置管理控制器
 */

namespace InkClock\Controller;

use InkClock\Utils\EnvLoader;

class ConfigController extends BaseController {
    /**
     * 获取所有配置
     */
    public function getAll($params) {
        // 不需要认证，但需要检查管理员权限
        $user = $this->checkApiPermission(false);
        
        // 如果是开发环境，允许匿名访问
        if (!$user && $this->isDevelopmentEnvironment()) {
            $this->logAction('get_all_config', ['user_id' => 'anonymous', 'env' => 'development']);
            $config = $this->getConfigData();
            $this->response->success('获取配置成功', $config);
            return;
        }
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $this->logAction('get_all_config', ['user_id' => $user['id']]);
        
        // 获取配置
        $config = $this->getConfigData();
        
        $this->response->success('获取配置成功', $config);
    }
    
    /**
     * 更新配置
     */
    public function update($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $data = $this->parseRequestBody();
        $this->logAction('update_config', ['user_id' => $user['id'], 'config_keys' => array_keys($data)]);
        
        // 验证配置数据
        if (empty($data)) {
            $this->response->error('配置数据不能为空', 400);
        }
        
        // 更新配置
        $result = $this->updateConfigData($data);
        
        if ($result['success']) {
            $this->response->success('配置更新成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 获取特定配置项
     */
    public function get($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $key = $params['key'] ?? null;
        if (!$key) {
            $this->response->error('缺少配置键名', 400);
        }
        
        $this->logAction('get_config', ['user_id' => $user['id'], 'key' => $key]);
        
        $config = $this->getConfigData();
        
        if (isset($config[$key])) {
            $this->response->success('获取配置成功', [$key => $config[$key]]);
        } else {
            $this->response->error('配置项不存在', 404);
        }
    }
    
    /**
     * 获取配置数据
     */
    private function getConfigData() {
        // 从环境变量和配置文件中读取配置
        $config = [
            // 数据库配置
            'DB_PATH' => EnvLoader::get('DB_PATH', './db/inkclock.db'),
            'DB_CACHE_ENABLED' => EnvLoader::getBool('DB_CACHE_ENABLED', true),
            'DB_CACHE_DEFAULT_EXPIRE' => EnvLoader::getInt('DB_CACHE_DEFAULT_EXPIRE', 300),
            
            // Web 服务器配置
            'WEB_SERVER_HOST' => EnvLoader::get('WEB_SERVER_HOST', '0.0.0.0'),
            'WEB_SERVER_PORT' => EnvLoader::get('WEB_SERVER_PORT', '8080'),
            'WEB_SERVER_DEBUG' => EnvLoader::getBool('WEB_SERVER_DEBUG', false),
            'WEB_SERVER_FORCE_HTTPS' => EnvLoader::getBool('WEB_SERVER_FORCE_HTTPS', false),
            
            // 安全配置
            'WEB_SERVER_DEFAULT_USERNAME' => EnvLoader::get('WEB_SERVER_DEFAULT_USERNAME', 'admin'),
            
            // 日志配置
            'LOG_LEVEL' => EnvLoader::get('LOG_LEVEL', 'info'),
            'LOG_FORMAT' => EnvLoader::get('LOG_FORMAT', 'text'),
            'LOG_MAX_SIZE' => EnvLoader::getInt('LOG_MAX_SIZE', 10485760),
            
            // 应用配置
            'APP_ENV' => EnvLoader::get('APP_ENV', 'production'),
            'APP_DEBUG' => EnvLoader::getBool('APP_DEBUG', false),
            'APP_URL' => EnvLoader::get('APP_URL', 'http://localhost:8080'),
        ];
        
        return $config;
    }
    
    /**
     * 更新配置数据
     */
    private function updateConfigData($data) {
        $envFilePath = dirname(__DIR__, 2) . '/.env';
        
        // 读取现有的 .env 文件
        $envContent = [];
        if (file_exists($envFilePath)) {
            $lines = file($envFilePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            foreach ($lines as $line) {
                if (strpos(trim($line), '#') === 0) {
                    // 保留注释
                    $envContent[] = $line;
                    continue;
                }
                
                if (strpos($line, '=') !== false) {
                    list($key, $value) = explode('=', $line, 2);
                    $envContent[trim($key)] = $line;
                } else {
                    $envContent[] = $line;
                }
            }
        }
        
        // 更新配置
        foreach ($data as $key => $value) {
            $key = trim($key);
            
            // 验证配置键名
            if (!preg_match('/^[A-Z_]+$/', $key)) {
                return ['success' => false, 'error' => "无效的配置键名：$key"];
            }
            
            // 格式化值
            if (is_bool($value)) {
                $formattedValue = $value ? 'true' : 'false';
            } else if (is_numeric($value)) {
                $formattedValue = $value;
            } else {
                $formattedValue = '"' . str_replace('"', '\"', $value) . '"';
            }
            
            $envContent[$key] = "$key=$formattedValue";
        }
        
        // 生成新的 .env 文件内容
        $newContent = "";
        foreach ($envContent as $key => $line) {
            if (is_numeric($key)) {
                $newContent .= $line . "\n";
            } else {
                $newContent .= $line . "\n";
            }
        }
        
        // 写入文件
        try {
            file_put_contents($envFilePath, $newContent);
            
            // 重新加载环境变量
            EnvLoader::load($envFilePath);
            
            return ['success' => true];
        } catch (\Exception $e) {
            return ['success' => false, 'error' => '写入配置文件失败：' . $e->getMessage()];
        }
    }
}
