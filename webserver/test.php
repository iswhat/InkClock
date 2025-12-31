<?php
/**
 * 测试脚本，用于检查API功能是否正常
 */

// 检查PHP版本
if (version_compare(PHP_VERSION, '7.4.0', '<')) {
    die("PHP版本需要7.4.0或更高版本");
}

echo "PHP版本检查通过: " . PHP_VERSION . "\n";

// 检查数据库连接
require_once 'config.php';

echo "正在检查数据库连接...\n";

$db = getDbConnection();
if ($db) {
    echo "数据库连接成功\n";
} else {
    die("数据库连接失败");
}

// 测试数据库表是否存在
$tables = [
    'users',
    'devices',
    'user_devices',
    'messages',
    'firmware_versions',
    'device_groups',
    'device_tags',
    'device_group_relations',
    'device_tag_relations',
    'message_templates',
    'firmware_push_tasks',
    'firmware_push_logs',
    'system_logs',
    'notifications'
];

echo "正在检查数据库表...\n";
foreach ($tables as $table) {
    $stmt = $db->prepare("SHOW TABLES LIKE ?");
    $stmt->bind_param("s", $table);
    $stmt->execute();
    $result = $stmt->get_result();
    if ($result->num_rows > 0) {
        echo "✓ 表 $table 存在\n";
    } else {
        echo "✗ 表 $table 不存在\n";
    }
    $stmt->close();
}

// 测试模型类是否能正常加载和实例化
echo "\n正在测试模型类...\n";

$models = [
    'Device',
    'Message',
    'FirmwareVersion',
    'User',
    'DeviceGroup',
    'DeviceTag',
    'FirmwarePushTask',
    'MessageTemplate',
    'SystemLog',
    'Notification'
];

foreach ($models as $model) {
    $file = "models/{$model}.php";
    if (file_exists($file)) {
        require_once $file;
        try {
            $instance = new $model();
            echo "✓ 模型 $model 加载成功\n";
        } catch (Exception $e) {
            echo "✗ 模型 $model 加载失败: " . $e->getMessage() . "\n";
        }
    } else {
        echo "✗ 模型文件 $file 不存在\n";
    }
}

// 测试插件JSON文件是否可写
echo "\n正在测试插件管理功能...\n";
$pluginJsonPath = __DIR__ . '/plugin/plugin.json';
if (file_exists($pluginJsonPath)) {
    echo "✓ 插件JSON文件存在\n";
    if (is_writable($pluginJsonPath)) {
        echo "✓ 插件JSON文件可写\n";
    } else {
        echo "✗ 插件JSON文件不可写\n";
    }
} else {
    echo "插件JSON文件不存在，正在创建...\n";
    $result = file_put_contents($pluginJsonPath, json_encode(array(), JSON_PRETTY_PRINT));
    if ($result !== false) {
        echo "✓ 插件JSON文件创建成功\n";
    } else {
        echo "✗ 插件JSON文件创建失败\n";
    }
}

echo "\n所有测试完成！\n";
?>