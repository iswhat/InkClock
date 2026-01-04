<?php
/**
 * 缓存工具类
 */

namespace InkClock\Utils;

class Cache {
    private static $instance = null;
    private $cacheDir;
    private $defaultExpire = 3600; // 默认过期时间：1小时
    private $logger;

    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        $this->logger = Logger::getInstance();
        $this->cacheDir = __DIR__ . '/../../cache';
        $this->ensureCacheDirectory();
    }

    /**
     * 确保缓存目录存在
     */
    private function ensureCacheDirectory() {
        if (!file_exists($this->cacheDir)) {
            mkdir($this->cacheDir, 0755, true);
        }
    }

    /**
     * 获取缓存实例
     * @return Cache
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }

    /**
     * 生成缓存文件路径
     * @param string $key 缓存键
     * @return string 缓存文件路径
     */
    private function getCacheFilePath($key) {
        $hash = md5($key);
        $subDir = substr($hash, 0, 2);
        
        // 创建子目录，分散缓存文件
        $subDirPath = $this->cacheDir . '/' . $subDir;
        if (!file_exists($subDirPath)) {
            mkdir($subDirPath, 0755, true);
        }
        
        return $subDirPath . '/' . $hash . '.cache';
    }

    /**
     * 设置缓存
     * @param string $key 缓存键
     * @param mixed $value 缓存值
     * @param int $expire 过期时间（秒），默认1小时
     * @return bool 设置结果
     */
    public function set($key, $value, $expire = null) {
        $expire = $expire ?? $this->defaultExpire;
        $filePath = $this->getCacheFilePath($key);
        
        // 序列化缓存数据
        $cacheData = [
            'value' => $value,
            'expire' => time() + $expire,
            'created_at' => time()
        ];
        
        $serialized = serialize($cacheData);
        $result = file_put_contents($filePath, $serialized);
        
        $this->logger->debug("缓存设置", ["key" => $key, "expire" => $expire]);
        
        return $result !== false;
    }

    /**
     * 获取缓存
     * @param string $key 缓存键
     * @return mixed 缓存值，如果不存在或过期则返回null
     */
    public function get($key) {
        $filePath = $this->getCacheFilePath($key);
        
        if (!file_exists($filePath)) {
            return null;
        }
        
        // 读取缓存数据
        $serialized = file_get_contents($filePath);
        if ($serialized === false) {
            return null;
        }
        
        // 反序列化缓存数据
        $cacheData = unserialize($serialized);
        if ($cacheData === false) {
            return null;
        }
        
        // 检查缓存是否过期
        if (time() > $cacheData['expire']) {
            // 删除过期缓存
            $this->delete($key);
            return null;
        }
        
        $this->logger->debug("缓存命中", ["key" => $key]);
        
        return $cacheData['value'];
    }

    /**
     * 删除缓存
     * @param string $key 缓存键
     * @return bool 删除结果
     */
    public function delete($key) {
        $filePath = $this->getCacheFilePath($key);
        
        if (file_exists($filePath)) {
            $result = unlink($filePath);
            $this->logger->debug("缓存删除", ["key" => $key]);
            return $result;
        }
        
        return true;
    }

    /**
     * 检查缓存是否存在
     * @param string $key 缓存键
     * @return bool 是否存在且未过期
     */
    public function has($key) {
        return $this->get($key) !== null;
    }

    /**
     * 清除所有缓存
     * @return bool 清除结果
     */
    public function clear() {
        $result = $this->deleteDirectory($this->cacheDir);
        $this->ensureCacheDirectory(); // 重新创建缓存目录
        $this->logger->debug("缓存全部清除");
        return $result;
    }

    /**
     * 删除目录（递归）
     * @param string $dir 目录路径
     * @return bool 删除结果
     */
    private function deleteDirectory($dir) {
        if (!file_exists($dir)) {
            return true;
        }
        
        if (!is_dir($dir)) {
            return unlink($dir);
        }
        
        foreach (scandir($dir) as $item) {
            if ($item == '.' || $item == '..') {
                continue;
            }
            
            if (!$this->deleteDirectory($dir . DIRECTORY_SEPARATOR . $item)) {
                return false;
            }
        }
        
        return rmdir($dir);
    }

    /**
     * 设置默认过期时间
     * @param int $seconds 过期时间（秒）
     */
    public function setDefaultExpire($seconds) {
        $this->defaultExpire = $seconds;
    }

    /**
     * 获取缓存统计信息
     * @return array 统计信息
     */
    public function getStats() {
        $stats = [
            'cache_dir' => $this->cacheDir,
            'default_expire' => $this->defaultExpire,
            'cache_count' => $this->countCacheFiles(),
            'cache_size' => $this->getCacheSize()
        ];
        
        return $stats;
    }

    /**
     * 统计缓存文件数量
     * @return int 缓存文件数量
     */
    private function countCacheFiles() {
        $count = 0;
        $dirIterator = new \RecursiveDirectoryIterator($this->cacheDir);
        $iterator = new \RecursiveIteratorIterator($dirIterator);
        
        foreach ($iterator as $file) {
            if ($file->isFile() && $file->getExtension() === 'cache') {
                $count++;
            }
        }
        
        return $count;
    }

    /**
     * 获取缓存总大小
     * @return int 缓存大小（字节）
     */
    private function getCacheSize() {
        $size = 0;
        $dirIterator = new \RecursiveDirectoryIterator($this->cacheDir);
        $iterator = new \RecursiveIteratorIterator($dirIterator);
        
        foreach ($iterator as $file) {
            if ($file->isFile() && $file->getExtension() === 'cache') {
                $size += $file->getSize();
            }
        }
        
        return $size;
    }
}
