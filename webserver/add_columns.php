<?php
/**
 * 添加users表中缺失的双因素认证列
 */

$dbPath = __DIR__ . '/data/inkclock.db';

// 检查数据库文件是否存在
if (!file_exists($dbPath)) {
    echo "数据库文件不存在: $dbPath\n";
    exit(1);
}

// 连接到SQLite数据库
try {
    $db = new SQLite3($dbPath);
    echo "成功连接到数据库\n";
    
    // 添加two_factor_enabled列
    $sql1 = "ALTER TABLE users ADD COLUMN two_factor_enabled INTEGER DEFAULT 0";
    $result1 = $db->exec($sql1);
    if ($result1) {
        echo "成功添加two_factor_enabled列\n";
    } else {
        echo "添加two_factor_enabled列失败: " . $db->lastErrorMsg() . "\n";
    }
    
    // 添加two_factor_secret列
    $sql2 = "ALTER TABLE users ADD COLUMN two_factor_secret TEXT";
    $result2 = $db->exec($sql2);
    if ($result2) {
        echo "成功添加two_factor_secret列\n";
    } else {
        echo "添加two_factor_secret列失败: " . $db->lastErrorMsg() . "\n";
    }
    
    $db->close();
    echo "数据库操作完成\n";
} catch (Exception $e) {
    echo "数据库操作失败: " . $e->getMessage() . "\n";
    exit(1);
}
?>