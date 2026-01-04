<?php
/**
 * 数据库连接工具类
 */

namespace App\Utils;

class Database {
    private static $instance = null;
    private $db;
    private $config;
    private $logger;

    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        $this->logger = Logger::getInstance();
        $this->loadConfig();
        $this->connect();
    }

    /**
     * 加载配置
     */
    private function loadConfig() {
        $config = require __DIR__ . '/../../config/config.php';
        $this->config = $config['db'];
    }

    /**
     * 连接SQLite数据库
     */
    private function connect() {
        $startTime = microtime(true);
        
        // 确保数据库目录存在
        $dbDir = __DIR__ . '/../../db';
        if (!file_exists($dbDir)) {
            mkdir($dbDir, 0755, true);
        }
        
        // 创建SQLite数据库连接
        $dbPath = $dbDir . '/inkclock.db';
        $this->db = new \SQLite3($dbPath);
        
        // 配置SQLite
        $this->db->enableExceptions(true);
        $this->db->exec('PRAGMA foreign_keys = ON');
        $this->db->exec('PRAGMA journal_mode = WAL');
        $this->db->exec('PRAGMA synchronous = NORMAL');
        $this->db->exec('PRAGMA temp_store = MEMORY');
        
        // 创建表如果不存在
        $this->createTables();
        
        $connectTime = microtime(true) - $startTime;
        $this->logger->info("数据库连接成功", ["path" => $dbPath, "time" => $connectTime]);
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
        $this->db->exec('BEGIN TRANSACTION');
    }
    
    /**
     * 提交事务
     */
    public function commit() {
        $this->db->exec('COMMIT');
    }
    
    /**
     * 回滚事务
     */
    public function rollback() {
        $this->db->exec('ROLLBACK');
    }
    
    /**
     * 获取数据库连接实例
     * @param string $dbPath 数据库路径（可选）
     */
    public static function getInstance($dbPath = null) {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }

    /**
     * 获取数据库连接
     */
    public function getConnection() {
        return $this->db;
    }
    
    /**
     * 关闭数据库连接
     */
    public function close() {
        if ($this->db) {
            $this->db->close();
            $this->db = null;
        }
    }
}