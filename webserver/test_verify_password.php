<?php
// 测试验证密码和登录方法
require_once __DIR__ . '/src/Config/Config.php';
require_once __DIR__ . '/src/Utils/Logger.php';
require_once __DIR__ . '/src/Utils/Database.php';
require_once __DIR__ . '/src/Model/User.php';

use InkClock\Config\Config;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;
use InkClock\Model\User;

echo "测试验证密码和登录方法\n";

try {
    // 初始化配置
    Config::load();
    
    // 初始化日志
    $logger = Logger::getInstance();
    
    // 初始化数据库
    $db = Database::getInstance();
    echo "数据库实例已创建\n";
    
    // 直接查询用户密码哈希
    $result = $db->query("SELECT username, password_hash FROM users WHERE username = 'iswhat'");
    if (count($result) > 0) {
        $user = $result[0];
        echo "用户: " . $user['username'] . "\n";
        echo "密码哈希: " . $user['password_hash'] . "\n";
        
        // 验证密码
        $testPassword = 'admin123';
        $isValid = password_verify($testPassword, $user['password_hash']);
        echo "密码验证结果: " . ($isValid ? "正确" : "错误") . "\n";
    } else {
        echo "未找到用户'iswhat'\n";
    }
    
    // 测试User::login方法
    echo "\n测试User::login方法\n";
    $userModel = new User($db);
    $loginResult = $userModel->login('iswhat', 'admin123');
    echo "登录结果: " . json_encode($loginResult) . "\n";
    
exit(0);
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    echo "堆栈跟踪: " . $e->getTraceAsString() . "\n";
    exit(1);
}
?>