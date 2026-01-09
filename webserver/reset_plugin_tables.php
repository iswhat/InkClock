<?php
/**
 * 重置插件表结构
 */

// 数据库文件路径
$dbPath = __DIR__ . '/db/inkclock.db';

try {
    // 创建 SQLite3 连接
    $db = new SQLite3($dbPath);
    
    // 删除旧的插件表
    $db->exec('DROP TABLE IF EXISTS plugins');
    $db->exec('DROP TABLE IF EXISTS device_plugins');
    
    echo "已删除旧的插件表结构，现在会重新创建正确的表结构。\n";
    
    // 关闭连接
    $db->close();
    
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
}
?>