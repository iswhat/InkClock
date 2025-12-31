<?php
/**
 * 数据库连接工具类
 */

namespace InkClock\Utils;

class Database {
    private static $instance = null;
    private $db;
    private $config;

    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
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
        // 确保数据库目录存在
        $dbDir = __DIR__ . '/../../db';
        if (!file_exists($dbDir)) {
            mkdir($dbDir, 0755, true);
        }
        
        // 创建SQLite数据库连接
        $dbPath = $dbDir . '/inkclock.db';
        $this->db = new \SQLite3($dbPath);
        
        // 创建表如果不存在
        $this->createTables();
    }

    /**
     * 创建数据库表
     */
    private function createTables() {
        // 用户表
        $this->db->exec("
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                email TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                api_key TEXT NOT NULL UNIQUE,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                last_login DATETIME,
                is_admin INTEGER DEFAULT 0,
                status INTEGER DEFAULT 1
            )
        ");

        // 用户设备关联表
        $this->db->exec("
            CREATE TABLE IF NOT EXISTS user_devices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                device_id TEXT NOT NULL,
                nickname TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users(id)
            )
        ");

        // 设备表
        $this->db->exec("
            CREATE TABLE IF NOT EXISTS devices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id TEXT NOT NULL UNIQUE,
                nickname TEXT,
                model TEXT,
                firmware_version TEXT,
                status TEXT DEFAULT 'offline',
                last_active DATETIME,
                ip_address TEXT
            )
        ");

        // 消息表
        $this->db->exec("
            CREATE TABLE IF NOT EXISTS messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                message_id TEXT NOT NULL UNIQUE,
                device_id TEXT NOT NULL,
                sender_id TEXT NOT NULL,
                content TEXT NOT NULL,
                type TEXT DEFAULT 'text',
                is_read INTEGER DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ");
    }

    /**
     * 获取数据库连接实例
     */
    public static function getInstance() {
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
}