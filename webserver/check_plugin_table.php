<?php
/**
 * 检查 plugins 表结构
 */

// 数据库文件路径
$dbPath = __DIR__ . '/db/inkclock.db';

// 直接使用 SQLite3 连接
try {
    // 创建 SQLite3 连接
    $db = new SQLite3($dbPath);
    
    // 检查 plugins 表是否存在
    $result = $db->query('PRAGMA table_info(plugins)');
    
    echo "Plugins 表结构：\n";
    echo "=====================================\n";
    
    $columns = [];
    while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
        $columns[] = $row['name'];
        echo "列名: {$row['name']}, 类型: {$row['type']}, 非空: {$row['notnull']}, 默认值: {$row['dflt_value']}\n";
    }
    
    echo "=====================================\n";
    echo "所有列: " . implode(', ', $columns) . "\n";
    
    // 检查是否缺少必要的列
    $requiredColumns = ['id', 'name', 'description', 'url', 'type', 'status', 'author', 'version', 'refresh_interval', 'settings_url', 'created_by', 'created_at', 'approved_by', 'approved_at', 'approval_status'];
    $missingColumns = [];
    
    foreach ($requiredColumns as $column) {
        if (!in_array($column, $columns)) {
            $missingColumns[] = $column;
        }
    }
    
    if (!empty($missingColumns)) {
        echo "\n缺少的列: " . implode(', ', $missingColumns) . "\n";
    } else {
        echo "\n所有必要的列都存在！\n";
    }
    
    // 测试查询插件列表
    echo "\n测试查询插件列表：\n";
    $stmt = $db->prepare('SELECT * FROM plugins LIMIT 5');
    $result = $stmt->execute();
    
    $plugins = [];
    while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
        $plugins[] = $row;
    }
    
    echo "找到 " . count($plugins) . " 个插件\n";
    if (!empty($plugins)) {
        echo "第一个插件: " . json_encode($plugins[0], JSON_UNESCAPED_UNICODE) . "\n";
    }
    
    // 关闭连接
    $db->close();
    echo "\n检查完成！\n";
    
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
}
?>