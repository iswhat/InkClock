<?php
// 测试列出所有用户
require_once __DIR__ . '/src/Config/Config.php';
require_once __DIR__ . '/src/Utils/Logger.php';
require_once __DIR__ . '/src/Utils/Database.php';

use InkClock\Config\Config;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

echo "测试列出所有用户\n";

try {
    // 初始化配置
    Config::load();
    
    // 初始化日志
    $logger = Logger::getInstance();
    
    // 初始化数据库
    $db = Database::getInstance();
    echo "数据库实例已创建\n";
    
    // 查询所有用户
    $result = $db->query("SELECT * FROM users");
    echo "查询结果: " . json_encode($result) . "\n";
    echo "用户数量: " . count($result) . "\n";
    
    // 输出每个用户的信息
    foreach ($result as $user) {
        echo "用户ID: " . $user['id'] . "\n";
        echo "用户名: " . $user['username'] . "\n";
        echo "邮箱: " . $user['email'] . "\n";
        echo "密码哈希: " . $user['password_hash'] . "\n";
        echo "API密钥: " . $user['api_key'] . "\n";
        echo "是否管理员: " . ($user['is_admin'] ? "是" : "否") . "\n";
        echo "--------------------\n";
    }
    
exit(0);
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    echo "堆栈跟踪: " . $e->getTraceAsString() . "\n";
    exit(1);
}
?>