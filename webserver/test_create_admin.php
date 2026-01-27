<?php
// 测试创建第一个管理员用户
require_once __DIR__ . '/src/Config/Config.php';
require_once __DIR__ . '/src/Utils/Logger.php';
require_once __DIR__ . '/src/Utils/Database.php';
require_once __DIR__ . '/src/Model/User.php';

use InkClock\Config\Config;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;
use InkClock\Model\User;

echo "测试创建第一个管理员用户\n";

try {
    // 初始化配置
    Config::load();
    
    // 初始化日志
    $logger = Logger::getInstance();
    
    // 初始化数据库
    $db = Database::getInstance();
    echo "数据库实例已创建\n";
    
    // 检查是否已有用户
    $userModel = new User($db);
    $hasUsers = $userModel->hasUsers();
    echo "是否已有用户: " . ($hasUsers ? "是" : "否") . "\n";
    
    // 创建管理员用户
    $adminInfo = [
        'username' => 'admin',
        'email' => 'admin@example.com',
        'password' => 'admin123'
    ];
    
    echo "准备创建管理员用户: " . json_encode($adminInfo) . "\n";
    
    $result = $userModel->createFirstAdmin($adminInfo);
    echo "创建结果: " . json_encode($result) . "\n";
    
    if ($result['success']) {
        echo "成功: 管理员用户已创建\n";
    } else {
        echo "失败: " . $result['error'] . "\n";
    }
    
exit(0);
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    echo "堆栈跟踪: " . $e->getTraceAsString() . "\n";
    exit(1);
}
?>