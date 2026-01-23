<?php
// 测试SQLite3连接
$dbPath = __DIR__ . '/db/inkclock.db';
echo "测试数据库连接到: $dbPath\n";

try {
    // 检查SQLite3扩展是否启用
    if (!class_exists('SQLite3')) {
        echo "错误: SQLite3扩展未启用\n";
        exit(1);
    }
    
    // 创建数据库连接
    $db = new SQLite3($dbPath);
    echo "成功: 数据库连接已创建\n";
    
    // 测试基本查询
    $result = $db->query("SELECT 1 as test");
    if ($result) {
        $row = $result->fetchArray(SQLITE3_ASSOC);
        echo "成功: 基本查询执行结果: " . $row['test'] . "\n";
    }
    
    // 关闭连接
    $db->close();
    echo "成功: 数据库连接已关闭\n";
    
exit(0);
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    exit(1);
}
?>