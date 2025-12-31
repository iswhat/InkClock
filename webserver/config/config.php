<?php
/**
 * WebServer配置文件
 */

// 数据库配置
$config = array(
    'db' => array(
        'host' => 'localhost',
        'port' => 3306,
        'user' => 'root',
        'password' => '',
        'database' => 'inkclock',
        'charset' => 'utf8mb4'
    ),
    'api' => array(
        'secret_key' => 'your_secret_key_here',
        'version' => 'v1',
        'max_requests_per_minute' => 100
    ),
    'device' => array(
        'max_messages_per_device' => 20,
        'message_expire_days' => 30
    ),
    'log' => array(
        'level' => 'info',
        'file_path' => __DIR__ . '/../logs/app.log'
    )
);

// 生成设备唯一标识
function generateDeviceId() {
    return md5(uniqid(rand(), true));
}

// 生成消息唯一标识
function generateMessageId() {
    return md5(uniqid(rand(), true));
}
?>