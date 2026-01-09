-- 插件初始化SQL脚本
-- 将plugin.json中的插件插入到数据库中

-- 插入系统插件
INSERT OR IGNORE INTO plugins (name, description, url, type, status, author, version, refresh_interval, settings_url, created_by, approval_status, created_at)
VALUES 
('每日古诗', '每天获取一首经典古诗', '/plugin/daily_poem/index.php', 'system', 'enabled', 'system', '1.0.0', '60分钟', '/plugin/daily_poem/index.php', NULL, 'approved', CURRENT_TIMESTAMP),
('每日英语单词', '每天学习一个英语单词', '/plugin/daily_word/index.php', 'system', 'enabled', 'system', '1.0.0', '24小时', '/plugin/daily_word/index.php', NULL, 'approved', CURRENT_TIMESTAMP),
('新闻头条', '获取最新新闻头条', '/plugin/news_headlines/index.php', 'system', 'enabled', 'system', '1.0.0', '30分钟', '/plugin/news_headlines/index.php', NULL, 'approved', CURRENT_TIMESTAMP);

-- 查看插入结果
SELECT * FROM plugins;
