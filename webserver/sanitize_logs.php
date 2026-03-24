<?php
/**
 * 日志脱敏工具
 * 用于清理日志文件中的敏感信息
 */

$logFile = __DIR__ . '/logs/app.log';
$backupFile = __DIR__ . '/logs/app.log.backup';

if (!file_exists($logFile)) {
    echo "日志文件不存在\n";
    exit(1);
}

// 备份原日志
copy($logFile, $backupFile);
echo "已创建备份：$backupFile\n";

// 读取日志内容
$logContent = file_get_contents($logFile);

// 定义敏感信息模式
$sensitivePatterns = [
    // 密码
    '/"password":"[^"]*"/i' => '"password":"***REDACTED***"',
    '/password=\w+/i' => 'password=***REDACTED***',
    
    // API 密钥
    '/"api_key":"[^"]*"/i' => '"api_key":"***REDACTED***"',
    '/api_key=[^&\s]+/i' => 'api_key=***REDACTED***',
    
    // 密钥
    '/"secret":"[^"]*"/i' => '"secret":"***REDACTED***"',
    '/secret_key=[^&\s]+/i' => 'secret_key=***REDACTED***',
    
    // 邮箱（可选，根据实际情况决定是否脱敏）
    // '/"email":"[^"]*"/i' => '"email":"***REDACTED***"',
];

// 执行脱敏
foreach ($sensitivePatterns as $pattern => $replacement) {
    $logContent = preg_replace($pattern, $replacement, $logContent);
}

// 写回日志文件
file_put_contents($logFile, $logContent);

echo "日志脱敏完成！\n";
echo "已移除的敏感信息类型：\n";
echo "  - 密码 (password)\n";
echo "  - API 密钥 (api_key)\n";
echo "  - 密钥 (secret/secret_key)\n";
echo "\n注意：请检查代码中的日志记录逻辑，避免再次记录敏感信息。\n";
