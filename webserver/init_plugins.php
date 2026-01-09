<?php
/**
 * 插件初始化脚本
 * 直接操作数据库，将插件数据插入到数据库中
 */

// 直接连接SQLite数据库
$dbPath = __DIR__ . '/data/inkclock.db';

if (!file_exists($dbPath)) {
    echo "数据库文件不存在: $dbPath\n";
    exit(1);
}

try {
    // 连接数据库
    $db = new SQLite3($dbPath);
    $db->exec('PRAGMA foreign_keys = ON');
    
    echo "连接数据库成功\n";
    
    // 准备插入语句
    $stmt = $db->prepare("INSERT OR IGNORE INTO plugins (name, description, url, type, status, author, version, refresh_interval, settings_url, created_by, approval_status, created_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)");
    
    if (!$stmt) {
        echo "准备语句失败: " . $db->lastErrorMsg() . "\n";
        exit(1);
    }
    
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
        $stmt->bindValue(1, $plugin['name'], SQLITE3_TEXT);
        $stmt->bindValue(2, $plugin['description'], SQLITE3_TEXT);
        $stmt->bindValue(3, $plugin['url'], SQLITE3_TEXT);
        $stmt->bindValue(4, $plugin['type'], SQLITE3_TEXT);
        $stmt->bindValue(5, $plugin['status'], SQLITE3_TEXT);
        $stmt->bindValue(6, $plugin['author'], SQLITE3_TEXT);
        $stmt->bindValue(7, $plugin['version'], SQLITE3_TEXT);
        $stmt->bindValue(8, $plugin['refresh_interval'], SQLITE3_TEXT);
        $stmt->bindValue(9, $plugin['settings_url'], SQLITE3_TEXT);
        $stmt->bindValue(10, $plugin['created_by'], SQLITE3_NULL);
        $stmt->bindValue(11, $plugin['approval_status'], SQLITE3_TEXT);
        
        if ($stmt->execute()) {
            $insertedCount++;
            echo "插入插件成功: {$plugin['name']}\n";
        } else {
            echo "插入插件失败: {$plugin['name']} - " . $db->lastErrorMsg() . "\n";
        }
    }
    
    // 查看插入结果
    echo "\n插入结果: 成功插入 $insertedCount 个插件\n";
    
    // 查询所有插件
    echo "\n所有插件:\n";
    $result = $db->query('SELECT id, name, status, approval_status FROM plugins');
    while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
        echo "ID: {$row['id']}, 名称: {$row['name']}, 状态: {$row['status']}, 审批状态: {$row['approval_status']}\n";
    }
    
    // 关闭数据库连接
    $db->close();
    
    echo "\n插件初始化完成\n";
    
} catch (Exception $e) {
    echo "错误: " . $e->getMessage() . "\n";
    exit(1);
}
