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
    )
);

// 错误报告设置
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 连接数据库
function getDbConnection() {
    global $config;
    $db = new mysqli(
        $config['db']['host'],
        $config['db']['user'],
        $config['db']['password'],
        $config['db']['database'],
        $config['db']['port']
    );
    
    if ($db->connect_error) {
        die("数据库连接失败: " . $db->connect_error);
    }
    
    $db->set_charset($config['db']['charset']);
    return $db;
}

// 生成设备唯一标识
function generateDeviceId() {
    return md5(uniqid(rand(), true));
}

// 生成消息唯一标识
function generateMessageId() {
    return md5(uniqid(rand(), true));
}

// 检查API密钥
function checkApiKey($apiKey) {
    global $config;
    return $apiKey === $config['api']['secret_key'];
}

// 发送JSON响应
function sendJsonResponse($data, $statusCode = 200) {
    header('Content-Type: application/json');
    http_response_code($statusCode);
    echo json_encode($data);
    exit;
}

// 处理CORS
function handleCORS() {
    header("Access-Control-Allow-Origin: *");
    header("Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS");
    header("Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With");
    
    if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
        http_response_code(200);
        exit;
    }
}
?>