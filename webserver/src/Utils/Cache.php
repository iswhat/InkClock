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
    private $compress = false; // 是否启用缓存压缩
    private $stats = [
        'hits' => 0,
        'misses' => 0,
        'writes' => 0,
        'deletes' => 0
    ];
    private $tagMapping = []; // 标签到缓存键的映射
    private $tagMappingFile; // 标签映射文件路径

    /**
     * 构造函数
     * @param array $config 配置数组
     */
    public function __construct($config = null) {
        $this->logger = Logger::getInstance();
        
        // 加载配置
        if ($config) {
            $this->cacheDir = $config['dir'] ?? __DIR__ . '/../../cache';
            $this->defaultExpire = $config['expire'] ?? 3600;
            $this->compress = $config['compress'] ?? false;
        } else {
            $this->cacheDir = __DIR__ . '/../../cache';
        }
        
        $this->tagMappingFile = $this->cacheDir . '/tag_mapping.json';
        $this->ensureCacheDirectory();
        $this->loadTagMapping();
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
     * @param array $config 配置数组
     * @return Cache
     */
    public static function getInstance($config = null) {
        if (self::$instance === null) {
            self::$instance = new self($config);
        }
        return self::$instance;
    }

    /**
     * 加载标签映射
     */
    private function loadTagMapping() {
        if (file_exists($this->tagMappingFile)) {
            $content = file_get_contents($this->tagMappingFile);
            if ($content) {
                $this->tagMapping = json_decode($content, true) ?? [];
            }
        }
    }
    
    /**
     * 保存标签映射
     */
    private function saveTagMapping() {
        file_put_contents($this->tagMappingFile, json_encode($this->tagMapping, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT));
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
     * @param array $tags 标签数组（可选）
     * @return bool 设置结果
     */
    public function set($key, $value, $expire = null, $tags = []) {
        $expire = $expire ?? $this->defaultExpire;
        $filePath = $this->getCacheFilePath($key);
        
        // 序列化缓存数据
        $cacheData = [
            'value' => $value,
            'expire' => time() + $expire,
            'created_at' => time(),
            'tags' => $tags
        ];
        
        $serialized = serialize($cacheData);
        
        // 如果启用压缩，压缩数据
        if ($this->compress) {
            $serialized = gzcompress($serialized, 6);
            $cacheData['compressed'] = true;
        }
        
        $result = file_put_contents($filePath, $serialized);
        
        if ($result !== false) {
            // 更新统计
            $this->stats['writes']++;
            
            // 更新标签映射
            if (!empty($tags)) {
                foreach ($tags as $tag) {
                    if (!isset($this->tagMapping[$tag])) {
                        $this->tagMapping[$tag] = [];
                    }
                    $this->tagMapping[$tag][] = $key;
                }
                $this->saveTagMapping();
            }
        }
        
        $this->logger->debug("缓存设置", ["key" => $key, "expire" => $expire, "tags" => $tags]);
        
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
            $this->stats['misses']++;
            return null;
        }
        
        // 读取缓存数据
        $data = file_get_contents($filePath);
        if ($data === false) {
            $this->stats['misses']++;
            return null;
        }
        
        // 检查是否压缩
        if (substr($data, 0, 2) === "x") {
            $data = gzuncompress($data);
        }
        
        // 反序列化缓存数据
        $cacheData = unserialize($data);
        if ($cacheData === false) {
            $this->stats['misses']++;
            return null;
        }
        
        // 检查缓存是否过期
        if (time() > $cacheData['expire']) {
            // 删除过期缓存
            $this->delete($key);
            $this->stats['misses']++;
            return null;
        }
        
        // 更新统计
        $this->stats['hits']++;
        
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
        
        // 更新统计
        $this->stats['deletes']++;
        
        if (file_exists($filePath)) {
            // 读取缓存数据以获取标签信息
            $data = file_get_contents($filePath);
            if ($data) {
                if (substr($data, 0, 2) === "x") {
                    $data = gzuncompress($data);
                }
                $cacheData = unserialize($data);
                if ($cacheData && isset($cacheData['tags']) && !empty($cacheData['tags'])) {
                    // 从标签映射中移除
                    foreach ($cacheData['tags'] as $tag) {
                        if (isset($this->tagMapping[$tag])) {
                            $this->tagMapping[$tag] = array_filter($this->tagMapping[$tag], function($cachedKey) use ($key) {
                                return $cachedKey !== $key;
                            });
                            if (empty($this->tagMapping[$tag])) {
                                unset($this->tagMapping[$tag]);
                            }
                        }
                    }
                    $this->saveTagMapping();
                }
            }
            
            $result = unlink($filePath);
            $this->logger->debug("缓存删除", ["key" => $key]);
            return $result;
        }
        
        return true;
    }

    /**
     * 根据标签清除缓存
     * @param string|array $tags 标签或标签数组
     * @return bool 清除结果
     */
    public function flushByTags($tags) {
        if (!is_array($tags)) {
            $tags = [$tags];
        }
        
        $result = true;
        $deletedKeys = [];
        
        foreach ($tags as $tag) {
            if (isset($this->tagMapping[$tag])) {
                foreach ($this->tagMapping[$tag] as $key) {
                    if (!in_array($key, $deletedKeys)) {
                        $result = $result && $this->delete($key);
                        $deletedKeys[] = $key;
                    }
                }
                unset($this->tagMapping[$tag]);
            }
        }
        
        $this->saveTagMapping();
        $this->logger->debug("根据标签清除缓存", ["tags" => $tags, "deleted_keys" => $deletedKeys]);
        
        return $result;
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
        $this->tagMapping = [];
        $this->saveTagMapping();
        $this->stats = [
            'hits' => 0,
            'misses' => 0,
            'writes' => 0,
            'deletes' => 0
        ];
        $this->logger->debug("缓存全部清除");
        return $result;
    }
    
    /**
     * 获取缓存命中率
     * @return float 命中率（0-1之间的数值）
     */
    public function getHitRate() {
        $total = $this->stats['hits'] + $this->stats['misses'];
        return $total > 0 ? $this->stats['hits'] / $total : 0;
    }
    
    /**
     * 缓存预热
     * @param array $items 预加载的缓存项，格式：[key => ['value' => $value, 'expire' => $expire, 'tags' => $tags]]
     * @return int 成功预热的缓存项数量
     */
    public function warmup(array $items) {
        $successCount = 0;
        
        foreach ($items as $key => $item) {
            $value = $item['value'];
            $expire = $item['expire'] ?? $this->defaultExpire;
            $tags = $item['tags'] ?? [];
            
            if ($this->set($key, $value, $expire, $tags)) {
                $successCount++;
            }
        }
        
        $this->logger->debug("缓存预热完成", ["total" => count($items), "success" => $successCount]);
        return $successCount;
    }
    
    /**
     * 设置缓存压缩
     * @param bool $compress 是否启用压缩
     */
    public function setCompress($compress) {
        $this->compress = $compress;
    }
    
    /**
     * 清除过期缓存
     * @return int 清除的过期缓存数量
     */
    public function clearExpired() {
        $count = 0;
        $dirIterator = new \RecursiveDirectoryIterator($this->cacheDir);
        $iterator = new \RecursiveIteratorIterator($dirIterator);
        
        foreach ($iterator as $file) {
            if ($file->isFile() && $file->getExtension() === 'cache') {
                $filePath = $file->getPathname();
                $data = file_get_contents($filePath);
                if ($data) {
                    if (substr($data, 0, 2) === "x") {
                        $data = gzuncompress($data);
                    }
                    $cacheData = unserialize($data);
                    if ($cacheData && time() > $cacheData['expire']) {
                        unlink($filePath);
                        $count++;
                    }
                }
            }
        }
        
        $this->logger->debug("清除过期缓存", ["count" => $count]);
        return $count;
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
            'compress' => $this->compress,
            'cache_count' => $this->countCacheFiles(),
            'cache_size' => $this->getCacheSize(),
            'hits' => $this->stats['hits'],
            'misses' => $this->stats['misses'],
            'writes' => $this->stats['writes'],
            'deletes' => $this->stats['deletes'],
            'hit_rate' => $this->getHitRate(),
            'tag_count' => count($this->tagMapping)
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
