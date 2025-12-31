-- 数据库升级脚本：添加用户和设备绑定功能

-- 创建用户表
CREATE TABLE IF NOT EXISTS `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(64) NOT NULL COMMENT '用户名',
  `email` varchar(128) NOT NULL COMMENT '邮箱',
  `password_hash` varchar(255) NOT NULL COMMENT '密码哈希',
  `api_key` varchar(64) NOT NULL COMMENT 'API密钥',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  `last_login` datetime DEFAULT NULL COMMENT '最后登录时间',
  `status` tinyint(1) NOT NULL DEFAULT 1 COMMENT '状态：1-启用，0-禁用',
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`),
  UNIQUE KEY `email` (`email`),
  UNIQUE KEY `api_key` (`api_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- 创建用户设备绑定表
CREATE TABLE IF NOT EXISTS `user_devices` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL COMMENT '用户ID',
  `device_id` varchar(64) NOT NULL COMMENT '设备ID',
  `nickname` varchar(64) DEFAULT '' COMMENT '设备昵称',
  `created_at` datetime NOT NULL COMMENT '绑定时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_device` (`user_id`,`device_id`),
  KEY `user_id` (`user_id`),
  KEY `device_id` (`device_id`),
  CONSTRAINT `user_devices_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `user_devices_ibfk_2` FOREIGN KEY (`device_id`) REFERENCES `devices` (`device_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户设备绑定表';

-- 消息表添加用户ID字段
ALTER TABLE `messages` ADD COLUMN `user_id` int(11) DEFAULT NULL COMMENT '用户ID';
ALTER TABLE `messages` ADD CONSTRAINT `messages_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE SET NULL ON UPDATE CASCADE;

-- 固件版本表添加发布者ID字段
ALTER TABLE `firmware_versions` ADD COLUMN `publisher_id` int(11) DEFAULT NULL COMMENT '发布者ID';
ALTER TABLE `firmware_versions` ADD CONSTRAINT `firmware_versions_ibfk_1` FOREIGN KEY (`publisher_id`) REFERENCES `users` (`id`) ON DELETE SET NULL ON UPDATE CASCADE;

-- 插入默认管理员用户（密码：admin123）
INSERT INTO `users` (`username`, `email`, `password_hash`, `api_key`, `created_at`, `status`) VALUES 
('admin', 'admin@example.com', '$2y$10$7e6J6Z5K5f4D3C2B1A0Z9Y8X7W6V5U4T3S2R1Q0P9O8N7M6L5K4J3I2H1G', 'admin_api_key_1234567890', NOW(), 1);
