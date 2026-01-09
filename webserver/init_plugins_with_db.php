<?php
/**
 * 插件初始化脚本（使用项目的Database类）
 * 直接操作数据库，将插件数据插入到数据库中
 */

// 加载项目的自动加载器（如果有）
require_once __DIR__ . '/src/Utils/Database.php';

use InkClock\Utils\Database;

try {
    echo "初始化插件数据...\n";
    
    // 获取数据库实例
    $db = Database::getInstance();
    
    echo "连接数据库成功\n";
    
    // 准备插入语句
    $sql = "INSERT OR IGNORE INTO plugins (name, description, url, type, status, author, version, refresh_interval, settings_url, created_by, approval_status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    // 插件数据
    $plugins = [
        [
            'name' => '每日古诗',
            'description' => '每天获取一首经典古诗',
            'url' => '/plugin/daily_poem/index.php',
            'type' => 'system',
            'status' => 'enabled',
            'author' => 'system',
            'version' => '1.0.0',
            'refresh_interval' => '60分钟',
            'settings_url' => '/plugin/daily_poem/index.php',
            'created_by' => null,
            'approval_status' => 'approved'
        ],
        [
            'name' => '每日英语单词',
            'description' => '每天学习一个英语单词',
            'url' => '/plugin/daily_word/index.php',
            'type' => 'system',
            'status' => 'enabled',
            'author' => 'system',
            'version' => '1.0.0',
            'refresh_interval' => '24小时',
            'settings_url' => '/plugin/daily_word/index.php',
            'created_by' => null,
            'approval_status' => 'approved'
        ],
        [
            'name' => '新闻头条',
            'description' => '获取最新新闻头条',
            'url' => '/plugin/news_headlines/index.php',
            'type' => 'system',
            'status' => 'enabled',
            'author' => 'system',
            'version' => '1.0.0',
            'refresh_interval' => '30分钟',
            'settings_url' => '/plugin/news_headlines/index.php',
            'created_by' => null,
            'approval_status' => 'approved'
        ]
    ];
    
    // 插入插件数据
    $insertedCount = 0;
    foreach ($plugins as $plugin) {
        $params = [
            $plugin['name'],
            $plugin['description'],
            $plugin['url'],
            $plugin['type'],
            $plugin['status'],
            $plugin['author'],
            $plugin['version'],
            $plugin['refresh_interval'],
            $plugin['settings_url'],
            $plugin['created_by'],
            $plugin['approval_status']
        ];
        
        if ($db->execute($sql, $params)) {
            $insertedCount++;
            echo "插入插件成功: {$plugin['name']}\n";
        } else {
            echo "插入插件失败: {$plugin['name']}\n";
        }
    }
    
    // 查看插入结果
    echo "\n插入结果: 成功插入 $insertedCount 个插件\n";
    
    // 查询所有插件
    echo "\n所有插件:\n";
    $result = $db->query('SELECT id, name, status, approval_status FROM plugins');
    foreach ($result as $row) {
        echo "ID: {$row['id']}, 名称: {$row['name']}, 状态: {$row['status']}, 审批状态: {$row['approval_status']}\n";
    }
    
    echo "\n插件初始化完成\n";
    
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    exit(1);
}
