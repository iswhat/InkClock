<?php
/**
 * 配置管理类
 */

namespace InkClock\Config;

class Config {
    private static $config = [];
    private static $loaded = false;
    private static $envLoaded = false;
    private static $instance = null;
    private static $environment = 'development';
    private static $configDir = __DIR__ . '/../../config';

    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        // 私有构造函数
    }

    /**
     * 获取单例实例
     * @return Config
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }

    /**
     * 设置当前环境
     * @param string $environment 环境名称（development, production, testing等）
     * @return void
     */
    public static function setEnvironment($environment) {
        self::$environment = $environment;
        // 环境改变时重新加载配置
        self::$loaded = false;
    }

    /**
     * 获取当前环境
     * @return string
     */
    public static function getEnvironment() {
        return self::$environment;
    }

    /**
     * 设置配置目录
     * @param string $configDir 配置目录路径
     * @return void
     */
    public static function setConfigDir($configDir) {
        self::$configDir = $configDir;
        // 配置目录改变时重新加载配置
        self::$loaded = false;
    }

    /**
     * 加载配置文件
     * @param string $filePath 配置文件路径（可选）
     * @return void
     */
    public static function load($filePath = null) {
        if (self::$loaded) {
            return;
        }

        // 加载环境变量
        self::loadEnv();

        // 获取当前环境
        $env = getenv('APP_ENV') ?: self::$environment;
        self::$environment = $env;

        // 加载基础配置
        $baseConfigFile = $filePath ?: self::$configDir . '/config.php';
        $baseConfig = require_once $baseConfigFile;
        
        // 加载环境特定配置
        $envConfigFile = self::$configDir . '/config.' . $env . '.php';
        $envConfig = file_exists($envConfigFile) ? require_once $envConfigFile : [];
        
        // 合并配置，环境配置覆盖基础配置
        self::$config = array_replace_recursive($baseConfig, $envConfig);
        
        // 从环境变量覆盖配置
        self::overrideConfigFromEnv();
        
        self::$loaded = true;
    }
    
    /**
     * 加载环境变量
     * @param string $envPath .env文件路径（可选）
     * @return void
     */
    private static function loadEnv($envPath = null) {
        if (self::$envLoaded) {
            return;
        }
        
        // 获取环境特定的.env文件
        $baseEnvPath = $envPath ?: __DIR__ . '/../../.env';
        $env = getenv('APP_ENV') ?: self::$environment;
        $envSpecificPath = __DIR__ . '/../../.env.' . $env;
        
        // 优先加载基础.env文件
        self::loadEnvFile($baseEnvPath);
        
        // 然后加载环境特定的.env文件（会覆盖基础配置）
        self::loadEnvFile($envSpecificPath);
        
        // 最后加载.env.local文件（用于本地开发，会覆盖所有配置）
        self::loadEnvFile(__DIR__ . '/../../.env.local');
        
        self::$envLoaded = true;
    }
    
    /**
     * 加载单个环境变量文件
     * @param string $filePath .env文件路径
     * @return void
     */
    private static function loadEnvFile($filePath) {
        if (file_exists($filePath)) {
            $lines = file($filePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            foreach ($lines as $line) {
                // 跳过注释
                if (strpos(trim($line), '#') === 0) {
                    continue;
                }
                
                // 跳过没有等号的行
                if (strpos($line, '=') === false) {
                    continue;
                }
                
                // 解析环境变量
                list($key, $value) = explode('=', $line, 2);
                $key = trim($key);
                $value = trim($value, "'\" ");
                
                // 设置环境变量
                putenv("$key=$value");
                $_ENV[$key] = $value;
                $_SERVER[$key] = $value;
            }
        }
    }
    
    /**
     * 从环境变量覆盖配置
     * @return void
     */
    private static function overrideConfigFromEnv() {
        // 遍历配置，检查是否有对应的环境变量
        self::overrideConfigRecursive(self::$config, '');
    }
    
    /**
     * 递归覆盖配置
     * @param array $config 配置数组
     * @param string $prefix 环境变量前缀
     * @return void
     */
    private static function overrideConfigRecursive(&$config, $prefix) {
        foreach ($config as $key => &$value) {
            $currentKey = $prefix ? $prefix . '.' . $key : $key;
            $envKey = strtoupper(str_replace('.', '_', "INKCLOCK_$currentKey"));
            
            // 检查环境变量
            $envValue = getenv($envKey);
            if ($envValue !== false) {
                // 转换环境变量类型
                $config[$key] = self::convertEnvValue($envValue);
            } elseif (is_array($value)) {
                // 递归处理数组
                self::overrideConfigRecursive($value, $currentKey);
            }
        }
    }
    
    /**
     * 转换环境变量值的类型
     * @param string $value 环境变量值
     * @return mixed 转换后的值
     */
    private static function convertEnvValue($value) {
        // 转换布尔值
        if (strtolower($value) === 'true') {
            return true;
        }
        if (strtolower($value) === 'false') {
            return false;
        }
        
        // 转换整数
        if (is_numeric($value) && strpos($value, '.') === false) {
            return (int)$value;
        }
        
        // 转换浮点数
        if (is_numeric($value)) {
            return (float)$value;
        }
        
        // 转换JSON
        $jsonValue = json_decode($value, true);
        if (json_last_error() === JSON_ERROR_NONE) {
            return $jsonValue;
        }
        
        // 转换数组（逗号分隔）
        if (strpos($value, ',') !== false) {
            return array_map('trim', explode(',', $value));
        }
        
        // 默认返回字符串
        return $value;
    }

    /**
     * 获取配置项
     * @param string $key 配置项键名，支持点分隔（如：db.host）
     * @param mixed $default 默认值
     * @return mixed
     */
    public static function get($key, $default = null) {
        if (!self::$loaded) {
            self::load();
        }

        $keys = explode('.', $key);
        $value = self::$config;

        foreach ($keys as $k) {
            if (!isset($value[$k])) {
                return $default;
            }
            $value = $value[$k];
        }

        return $value;
    }

    /**
     * 设置配置项
     * @param string $key 配置项键名，支持点分隔（如：db.host）
     * @param mixed $value 配置值
     * @return void
     */
    public static function set($key, $value) {
        if (!self::$loaded) {
            self::load();
        }

        $keys = explode('.', $key);
        $config = &self::$config;

        foreach ($keys as $k) {
            if (!isset($config[$k])) {
                $config[$k] = [];
            }
            $config = &$config[$k];
        }

        $config = $value;
    }

    /**
     * 检查配置项是否存在
     * @param string $key 配置项键名，支持点分隔（如：db.host）
     * @return bool
     */
    public static function has($key) {
        if (!self::$loaded) {
            self::load();
        }

        $keys = explode('.', $key);
        $value = self::$config;

        foreach ($keys as $k) {
            if (!isset($value[$k])) {
                return false;
            }
            $value = $value[$k];
        }

        return true;
    }
}