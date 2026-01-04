<?php
/**
 * WebServer配置文件
 */

// 加载环境变量
function loadEnvironmentVariables() {
    $envFile = __DIR__ . '/../.env';
    if (file_exists($envFile)) {
        $lines = file($envFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($lines as $line) {
            // 跳过注释
            if (strpos($line, '#') === 0) {
                continue;
            }
            list($name, $value) = explode('=', $line, 2);
            $name = trim($name);
            $value = trim($value, "'\"");
            putenv("$name=$value");
            $_ENV[$name] = $value;
        }
    }
}

// 加载环境变量
loadEnvironmentVariables();

// 获取环境变量，带默认值
function env($name, $default = null) {
    return getenv($name) !== false ? getenv($name) : $default;
}

// 数据库配置
$config = array(
    'db' => array(
        'host' => env('DB_HOST', 'localhost'),
        'port' => (int)env('DB_PORT', 3306),
        'user' => env('DB_USER', 'root'),
        'password' => env('DB_PASSWORD', ''),
        'database' => env('DB_NAME', 'inkclock'),
        'charset' => env('DB_CHARSET', 'utf8mb4')
    ),
    'database' => array(
        'path' => env('SQLITE_PATH', __DIR__ . '/../db/inkclock.db')
    ),
    'api' => array(
        'secret_key' => env('API_SECRET_KEY', 'your_secret_key_here'),
        'version' => env('API_VERSION', 'v1'),
        'max_requests_per_minute' => (int)env('API_MAX_REQUESTS', 100)
    ),
    'device' => array(
        'max_messages_per_device' => (int)env('DEVICE_MAX_MESSAGES', 20),
        'message_expire_days' => (int)env('DEVICE_MESSAGE_EXPIRE', 30)
    ),
    'log' => array(
        'level' => env('LOG_LEVEL', 'info'),
        'file_path' => env('LOG_FILE', __DIR__ . '/../logs/app.log')
    ),
    'cache' => array(
        'dir' => env('CACHE_DIR', __DIR__ . '/../cache'),
        'expire' => (int)env('CACHE_EXPIRE', 3600)
    ),
    'middleware' => array(
        'rate_limit' => array(
            'enabled' => env('RATE_LIMIT_ENABLED', true),
            'default_limit' => (int)env('RATE_LIMIT_DEFAULT', 60),
            'default_window' => (int)env('RATE_LIMIT_WINDOW', 60)
        )
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