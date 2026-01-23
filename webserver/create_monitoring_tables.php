<?php
/**
 * 创建监控相关的数据库表
 */

// 引入数据库连接
require_once __DIR__ . '/src/Utils/Database.php';

try {
    // 获取数据库实例
    $dbInstance = InkClock\Utils\Database::getInstance();
    $db = $dbInstance->getConnection();
    
    if (!$db) {
        echo "数据库连接失败\n";
        exit(1);
    }
    
    echo "数据库连接成功\n";
    
    // 检查并创建API请求日志表
    echo "检查api_requests表...\n";
    $createApiRequestsTable = "
        CREATE TABLE IF NOT EXISTS api_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            request_id TEXT NOT NULL,
            method TEXT NOT NULL,
            path TEXT NOT NULL,
            query TEXT,
            ip TEXT NOT NULL,
            user_agent TEXT,
            api_version TEXT,
            content_type TEXT,
            status_code INTEGER NOT NULL,
            response_time REAL NOT NULL,
            request_size INTEGER,
            response_size INTEGER,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    ";
    $db->exec($createApiRequestsTable);
    echo "api_requests表检查/创建完成\n";
    
    // 检查并创建性能指标表
    echo "检查performance_metrics表...\n";
    $createPerformanceMetricsTable = "
        CREATE TABLE IF NOT EXISTS performance_metrics (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            metric_name TEXT NOT NULL,
            metric_value REAL NOT NULL,
            metric_labels TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    ";
    $db->exec($createPerformanceMetricsTable);
    echo "performance_metrics表检查/创建完成\n";
    
    // 创建索引以提高查询性能
    echo "创建索引...\n";
    $db->exec("CREATE INDEX IF NOT EXISTS idx_api_requests_created_at ON api_requests(created_at)");
    $db->exec("CREATE INDEX IF NOT EXISTS idx_api_requests_path ON api_requests(path)");
    $db->exec("CREATE INDEX IF NOT EXISTS idx_api_requests_status_code ON api_requests(status_code)");
    $db->exec("CREATE INDEX IF NOT EXISTS idx_performance_metrics_created_at ON performance_metrics(created_at)");
    $db->exec("CREATE INDEX IF NOT EXISTS idx_performance_metrics_name ON performance_metrics(metric_name)");
    echo "索引创建完成\n";
    
    // 检查并创建系统状态表
    echo "检查system_status表...\n";
    $createSystemStatusTable = "
        CREATE TABLE IF NOT EXISTS system_status (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            cpu_usage REAL,
            memory_usage REAL,
            disk_usage REAL,
            network_in INTEGER,
            network_out INTEGER,
            uptime INTEGER,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    ";
    $db->exec($createSystemStatusTable);
    echo "system_status表检查/创建完成\n";
    
    echo "\n所有监控表创建完成！\n";
    
} catch (\Exception $e) {
    echo "错误：" . $e->getMessage() . "\n";
    exit(1);
}
