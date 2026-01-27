<?php
// 测试重置管理员用户密码
require_once __DIR__ . '/src/Config/Config.php';
require_once __DIR__ . '/src/Utils/Logger.php';
require_once __DIR__ . '/src/Utils/Database.php';

use InkClock\Config\Config;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

echo "测试重置管理员用户密码\n";

try {
    // 初始化配置
    Config::load();
    
    // 初始化日志
    $logger = Logger::getInstance();
    
    // 初始化数据库
    $db = Database::getInstance();
    echo "数据库实例已创建\n";
    
    // 重置管理员用户密码
    $newPassword = 'admin123';
    $passwordHash = password_hash($newPassword, PASSWORD_DEFAULT);
    
    $sql = "UPDATE users SET password_hash = :password_hash WHERE username = :username";
    $params = [
        'password_hash' => $passwordHash,
        'username' => 'iswhat'
    ];
    
    echo "准备重置用户'iswhat'的密码为'$newPassword'\n";
    
    $result = $db->execute($sql, $params);
    echo "重置结果: " . ($result ? "成功" : "失败") . "\n";
    
    if ($result) {
        echo "成功: 管理员用户密码已重置\n";
    } else {
        echo "失败: 管理员用户密码重置失败\n";
    }
    
exit(0);
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    echo "堆栈跟踪: " . $e->getTraceAsString() . "\n";
    exit(1);
}
?>