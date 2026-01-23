<?php
// 测试PDO连接到SQLite
$dbPath = __DIR__ . '/db/inkclock.db';
echo "测试PDO连接到: $dbPath\n";

try {
    // 检查PDO扩展是否启用
    if (!class_exists('PDO')) {
        echo "错误: PDO扩展未启用\n";
        exit(1);
    }
    
    // 检查PDO是否支持SQLite
    $drivers = PDO::getAvailableDrivers();
    echo "可用的PDO驱动: " . implode(', ', $drivers) . "\n";
    
    if (!in_array('sqlite', $drivers)) {
        echo "错误: PDO不支持SQLite\n";
        exit(1);
    }
    
    // 创建数据库连接
    $dsn = "sqlite:$dbPath";
    $db = new PDO($dsn);
    echo "成功: PDO数据库连接已创建\n";
    
    // 测试基本查询
    $stmt = $db->query("SELECT 1 as test");
    if ($stmt) {
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        echo "成功: 基本查询执行结果: " . $row['test'] . "\n";
    }
    
    // 关闭连接
    $db = null;
    echo "成功: PDO数据库连接已关闭\n";
    
exit(0);
} catch (PDOException $e) {
    echo "错误: " . $e->getMessage() . "\n";
    exit(1);
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    exit(1);
}
?>