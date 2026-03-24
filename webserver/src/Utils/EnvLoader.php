<?php
/**
 * 环境变量加载器
 * 从 .env 文件读取配置并加载到环境变量
 */

namespace InkClock\Utils;

class EnvLoader {
    /**
     * 加载 .env 文件
     * @param string $path .env 文件路径
     * @return bool 是否成功加载
     */
    public static function load($path = null) {
        if ($path === null) {
            $path = dirname(__DIR__, 2) . '/.env';
        }
        
        if (!file_exists($path)) {
            // .env 文件不存在，创建示例文件
            self::createSampleEnv($path);
            return false;
        }
        
        $lines = file($path, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        
        foreach ($lines as $line) {
            // 跳过注释行
            if (strpos(trim($line), '#') === 0) {
                continue;
            }
            
            // 解析 KEY=VALUE 格式
            if (strpos($line, '=') !== false) {
                list($key, $value) = explode('=', $line, 2);
                $key = trim($key);
                $value = trim($value);
                
                // 移除引号
                $value = self::removeQuotes($value);
                
                // 设置环境变量
                if (!empty($key)) {
                    putenv("$key=$value");
                    $_ENV[$key] = $value;
                }
            }
        }
        
        return true;
    }
    
    /**
     * 从环境变量获取值
     * @param string $key 键名
     * @param mixed $default 默认值
     * @return mixed 返回环境变量值或默认值
     */
    public static function get($key, $default = null) {
        $value = getenv($key);
        return $value === false ? $default : $value;
    }
    
    /**
     * 获取布尔值
     * @param string $key 键名
     * @param bool $default 默认值
     * @return bool 返回布尔值
     */
    public static function getBool($key, $default = false) {
        $value = self::get($key, $default ? 'true' : 'false');
        return in_array(strtolower($value), ['true', '1', 'yes', 'on']);
    }
    
    /**
     * 获取整数值
     * @param string $key 键名
     * @param int $default 默认值
     * @return int 返回整数值
     */
    public static function getInt($key, $default = 0) {
        return (int) self::get($key, $default);
    }
    
    /**
     * 获取浮点数值
     * @param string $key 键名
     * @param float $default 默认值
     * @return float 返回浮点数值
     */
    public static function getFloat($key, $default = 0.0) {
        return (float) self::get($key, $default);
    }
    
    /**
     * 移除字符串周围的引号
     * @param string $value 要处理的字符串
     * @return string 移除引号后的字符串
     */
    private static function removeQuotes($value) {
        if ((strpos($value, '"') === 0 && strrpos($value, '"') === strlen($value) - 1) ||
            (strpos($value, "'") === 0 && strrpos($value, "'") === strlen($value) - 1)) {
            return substr($value, 1, -1);
        }
        return $value;
    }
    
    /**
     * 创建示例 .env 文件
     * @param string $path 文件路径
     */
    private static function createSampleEnv($path) {
        $sampleContent = <<<EOT
# ========================================
# InkClock 环境变量配置文件
# ========================================
# 说明：复制此文件并重命名为 .env，然后根据实际需求修改配置值
# 注意：.env 文件包含敏感信息，请勿提交到版本控制系统！

# ========================================
# 数据库配置
# ========================================
DB_PATH=./db/inkclock.db
DB_CACHE_ENABLED=true
DB_CACHE_DEFAULT_EXPIRE=300

# ========================================
# Web 服务器配置
# ========================================
WEB_SERVER_HOST=0.0.0.0
WEB_SERVER_PORT=8080
WEB_SERVER_DEBUG=false

# ========================================
# 安全配置（首次启动时必须修改！）
# ========================================
WEB_SERVER_DEFAULT_USERNAME=admin
WEB_SERVER_DEFAULT_PASSWORD=ChangeMe123!  # TODO: 首次启动后必须修改

# ========================================
# API 配置
# ========================================
API_KEY=
API_SECRET=

# ========================================
# 日志配置
# ========================================
LOG_LEVEL=info
LOG_FORMAT=text
LOG_MAX_SIZE=10485760

# ========================================
# 其他配置
# ========================================
APP_ENV=production
APP_DEBUG=false
APP_URL=http://localhost:8080
EOT;
        
        file_put_contents($path, $sampleContent);
    }
}
