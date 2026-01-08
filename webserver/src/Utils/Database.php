<?php
/**
 * 数据库连接工具类
 */

namespace InkClock\Utils;

use InkClock\Config\Config;

class Database {
    private static $instance = null;
    private $db = null;
    private $config;
    private $logger;
    private $connected = false;
    private $dbPath;

    /**
     * 构造函数
     * @param array $config 配置数组（可选）
     */
    public function __construct($config = null) {
        $this->logger = Logger::getInstance();
        $this->loadConfig($config);
        // 延迟连接，首次使用时才连接
    }

    /**
     * 加载配置
     * @param array $config 配置数组（可选）
     */
    private function loadConfig($config = null) {
        if ($config) {
            $this->config = $config;
        } else {
            // 使用改进后的Config类加载配置
            Config::load();
            $this->config = Config::get('database');
        }
        
        // 获取数据库路径
        $this->dbPath = $this->config['path'];
    }

    /**
     * 连接SQLite数据库（延迟连接，首次使用时调用）
     */
    private function connect() {
        if ($this->connected && $this->db) {
            return;
        }
        
        $startTime = microtime(true);
        
        // 确保数据库目录存在
        $dbDir = dirname($this->dbPath);
        if (!file_exists($dbDir)) {
            mkdir($dbDir, 0755, true);
        }
        
        // 检查SQLite3扩展是否存在
        if (!class_exists('SQLite3')) {
            $this->connected = false;
            $this->logger->error('SQLite3扩展未启用，请在php.ini中启用sqlite3扩展');
            // 记录详细的错误信息，包括如何启用SQLite3扩展
            $this->logger->error('启用SQLite3扩展的方法：1. 打开php.ini文件 2. 找到;extension=sqlite3 3. 移除前面的分号 4. 重启服务器');
            return;
        }
        
        // 创建SQLite数据库连接
        $this->db = new \SQLite3($this->dbPath);
        
        // 临时设置为已连接，防止createTables()中的execute()再次调用connect()
        $this->connected = true;
        
        // 配置SQLite
        $this->db->enableExceptions(true);
        $this->db->exec('PRAGMA foreign_keys = ON');
        $this->db->exec('PRAGMA journal_mode = WAL');
        $this->db->exec('PRAGMA synchronous = NORMAL');
        $this->db->exec('PRAGMA temp_store = MEMORY');
        
        // 创建表如果不存在
        $this->createTables();
        
        $connectTime = microtime(true) - $startTime;
        $this->logger->info("数据库连接成功", ["path" => $this->dbPath, "time" => $connectTime]);
    }

    /**
     * 创建数据库表
     */
    private function createTables() {
        // 用户表
        $this->execute("CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                email TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                api_key TEXT NOT NULL UNIQUE,
                api_key_created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                api_key_expires_at DATETIME,
                api_key_ip_whitelist TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                last_login DATETIME,
                is_admin INTEGER DEFAULT 0,
                status INTEGER DEFAULT 1
            )");
        
        // 为用户表添加索引
        $this->execute("CREATE INDEX IF NOT EXISTS idx_users_api_key ON users (api_key)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_users_email ON users (email)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_users_username ON users (username)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_users_status ON users (status)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_users_api_key_expires ON users (api_key_expires_at)");

        // 设备表
        $this->execute("CREATE TABLE IF NOT EXISTS devices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id TEXT NOT NULL UNIQUE,
                nickname TEXT,
                model TEXT,
                firmware_version TEXT,
                battery_level INTEGER DEFAULT 100,
                connection_status TEXT DEFAULT 'offline',
                last_active DATETIME,
                ip_address TEXT,
                ipv6_address TEXT,
                mac_address TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )");
        
        // 为设备表添加索引
        $this->execute("CREATE INDEX IF NOT EXISTS idx_devices_device_id ON devices (device_id)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_devices_last_active ON devices (last_active)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_devices_connection_status ON devices (connection_status)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_devices_created_at ON devices (created_at)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_devices_model ON devices (model)");

        // 用户设备关联表
        $this->execute("CREATE TABLE IF NOT EXISTS user_devices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                device_id TEXT NOT NULL,
                nickname TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE,
                UNIQUE(user_id, device_id)
            )");
        
        // 为用户设备关联表添加索引
        $this->execute("CREATE INDEX IF NOT EXISTS idx_user_devices_user_id ON user_devices (user_id)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_user_devices_device_id ON user_devices (device_id)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_user_devices_user_device ON user_devices (user_id, device_id)");

        // 消息表
        $this->execute("CREATE TABLE IF NOT EXISTS messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                message_id TEXT NOT NULL UNIQUE,
                device_id TEXT NOT NULL,
                sender TEXT NOT NULL,
                content TEXT NOT NULL,
                type TEXT DEFAULT 'text',
                is_read INTEGER DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE
            )");
        
        // 为消息表添加索引
        $this->execute("CREATE INDEX IF NOT EXISTS idx_messages_device_id ON messages (device_id)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_messages_is_read ON messages (is_read)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_messages_created_at ON messages (created_at)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_messages_device_id_created_at ON messages (device_id, created_at DESC)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_messages_device_id_is_read ON messages (device_id, is_read)");
        
        // 固件版本表
        $this->execute("CREATE TABLE IF NOT EXISTS firmware_versions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                version TEXT NOT NULL,
                device_model TEXT,
                file_path TEXT,
                release_notes TEXT,
                is_active INTEGER DEFAULT 0,
                is_forced INTEGER DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                published_at DATETIME
            )");
        
        // 为固件版本表添加索引
        $this->execute("CREATE INDEX IF NOT EXISTS idx_firmware_versions_device_model ON firmware_versions (device_model)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_firmware_versions_is_active ON firmware_versions (is_active)");
        $this->execute("CREATE INDEX IF NOT EXISTS idx_firmware_versions_created_at ON firmware_versions (created_at)");
        
        // 设备分组表
        $this->execute("CREATE TABLE IF NOT EXISTS device_groups (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                user_id INTEGER NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )");
        
        // 设备分组与设备关联表
        $this->execute("CREATE TABLE IF NOT EXISTS device_group_devices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                group_id INTEGER NOT NULL,
                device_id TEXT NOT NULL,
                added_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (group_id) REFERENCES device_groups(id) ON DELETE CASCADE,
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE,
                UNIQUE(group_id, device_id)
            )");
        
        // 设备标签表
        $this->execute("CREATE TABLE IF NOT EXISTS device_tags (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                color TEXT DEFAULT '#3498db',
                user_id INTEGER NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )");
        
        // 设备与标签关联表
        $this->execute("CREATE TABLE IF NOT EXISTS device_tag_relations (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                tag_id INTEGER NOT NULL,
                device_id TEXT NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (tag_id) REFERENCES device_tags(id) ON DELETE CASCADE,
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE,
                UNIQUE(tag_id, device_id)
            )");
        
        // 消息模板表
        $this->execute("CREATE TABLE IF NOT EXISTS message_templates (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                content TEXT NOT NULL,
                type TEXT DEFAULT 'text',
                variables TEXT,
                user_id INTEGER NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )");
        
        // 固件推送任务表
        $this->execute("CREATE TABLE IF NOT EXISTS firmware_push_tasks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                firmware_id INTEGER NOT NULL,
                status TEXT DEFAULT 'pending',
                total_devices INTEGER DEFAULT 0,
                completed_devices INTEGER DEFAULT 0,
                failed_devices INTEGER DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                started_at DATETIME,
                completed_at DATETIME,
                FOREIGN KEY (firmware_id) REFERENCES firmware_versions(id) ON DELETE CASCADE
            )");
        
        // 固件推送日志表
        $this->execute("CREATE TABLE IF NOT EXISTS firmware_push_logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                task_id INTEGER NOT NULL,
                device_id TEXT NOT NULL,
                status TEXT DEFAULT 'pending',
                error_message TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                completed_at DATETIME,
                FOREIGN KEY (task_id) REFERENCES firmware_push_tasks(id) ON DELETE CASCADE,
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE
            )");
        
        // 通知表
        $this->execute("CREATE TABLE IF NOT EXISTS notifications (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                title TEXT,
                content TEXT NOT NULL,
                type TEXT DEFAULT 'info',
                is_read INTEGER DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            )");
        
        // 系统日志表
        $this->execute("CREATE TABLE IF NOT EXISTS system_logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                log_level TEXT NOT NULL,
                message TEXT NOT NULL,
                context TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )");
    }

    /**
     * 执行SQL语句（用于无返回结果的操作）
     * @param string $sql SQL语句
     * @param array $params 参数（可选）
     * @return bool 执行结果
     */
    public function execute($sql, $params = []) {
        // 延迟连接，首次使用时才连接
        $this->connect();
        
        $startTime = microtime(true);
        
        try {
            $stmt = $this->db->prepare($sql);
            if ($stmt) {
                $this->bindParams($stmt, $params);
                $result = $stmt->execute();
                $stmt->close();
            } else {
                $result = $this->db->exec($sql);
            }
            
            $duration = microtime(true) - $startTime;
            $this->logger->databaseLog($sql, $params, $duration);
            
            return $result;
        } catch (\Exception $e) {
            $duration = microtime(true) - $startTime;
            $this->logger->error("数据库执行错误", [
                "sql" => $sql,
                "params" => $params,
                "error" => $e->getMessage(),
                "time" => $duration
            ]);
            throw $e;
        }
    }
    
    /**
     * 执行查询语句（用于有返回结果的操作）
     * @param string $sql SQL语句
     * @param array $params 参数（可选）
     * @return array 查询结果
     */
    public function query($sql, $params = []) {
        // 延迟连接，首次使用时才连接
        $this->connect();
        
        $startTime = microtime(true);
        
        try {
            $stmt = $this->db->prepare($sql);
            $this->bindParams($stmt, $params);
            $result = $stmt->execute();
            
            $rows = [];
            while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
                $rows[] = $row;
            }
            
            $stmt->close();
            $result->finalize();
            
            $duration = microtime(true) - $startTime;
            $this->logger->databaseLog($sql, $params, $duration);
            
            return $rows;
        } catch (\Exception $e) {
            $duration = microtime(true) - $startTime;
            $this->logger->error("数据库查询错误", [
                "sql" => $sql,
                "params" => $params,
                "error" => $e->getMessage(),
                "time" => $duration
            ]);
            throw $e;
        }
    }
    
    /**
     * 执行单条查询（返回单条结果）
     * @param string $sql SQL语句
     * @param array $params 参数（可选）
     * @return array|null 查询结果
     */
    public function querySingle($sql, $params = []) {
        $rows = $this->query($sql, $params);
        return $rows[0] ?? null;
    }
    
    /**
     * 获取最后插入的ID
     * @return int 最后插入的ID
     */
    public function lastInsertId() {
        // 延迟连接，首次使用时才连接
        $this->connect();
        return $this->db->lastInsertRowID();
    }
    
    /**
     * 绑定参数
     * @param \SQLite3Stmt $stmt 预处理语句
     * @param array $params 参数数组
     */
    private function bindParams(&$stmt, $params) {
        foreach ($params as $key => $value) {
            if (is_int($value)) {
                $stmt->bindValue(":{$key}", $value, SQLITE3_INTEGER);
            } elseif (is_float($value)) {
                $stmt->bindValue(":{$key}", $value, SQLITE3_FLOAT);
            } elseif (is_bool($value)) {
                $stmt->bindValue(":{$key}", $value ? 1 : 0, SQLITE3_INTEGER);
            } else {
                $stmt->bindValue(":{$key}", $value, SQLITE3_TEXT);
            }
        }
    }
    
    /**
     * 开始事务
     */
    public function beginTransaction() {
        // 延迟连接，首次使用时才连接
        $this->connect();
        $this->db->exec('BEGIN TRANSACTION');
    }
    
    /**
     * 提交事务
     */
    public function commit() {
        // 确保连接存在
        if ($this->db) {
            $this->db->exec('COMMIT');
        }
    }
    
    /**
     * 回滚事务
     */
    public function rollback() {
        // 确保连接存在
        if ($this->db) {
            $this->db->exec('ROLLBACK');
        }
    }
    
    /**
     * 获取数据库连接实例
     * @param array $config 配置数组（可选）
     */
    public static function getInstance($config = null) {
        if (self::$instance === null) {
            self::$instance = new self($config);
        }
        return self::$instance;
    }

    /**
     * 获取数据库连接
     */
    public function getConnection() {
        // 延迟连接，首次使用时才连接
        $this->connect();
        
        // 如果连接失败（SQLite3扩展未启用），抛出异常
        if (!$this->db) {
            throw new \Exception('SQLite3扩展未启用，请在php.ini中启用sqlite3扩展');
        }
        
        return $this->db;
    }
    
    /**
     * 关闭数据库连接
     */
    public function close() {
        if ($this->db) {
            $this->db->close();
            $this->db = null;
            $this->connected = false;
        }
    }
    
    /**
     * 检查连接是否已建立
     * @return bool 连接状态
     */
    public function isConnected() {
        return $this->connected && $this->db !== null;
    }
}