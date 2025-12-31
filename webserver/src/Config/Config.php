<?php
/**
 * 配置管理类
 */

namespace InkClock\Config;

class Config {
    private static $config = [];
    private static $loaded = false;

    /**
     * 加载配置文件
     * @param string $filePath 配置文件路径
     * @return void
     */
    public static function load($filePath = null) {
        if (self::$loaded) {
            return;
        }

        // 默认配置文件路径
        if ($filePath === null) {
            $filePath = __DIR__ . '/../../config/config.php';
        }

        // 加载配置文件
        $config = require $filePath;
        self::$config = $config;
        self::$loaded = true;
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