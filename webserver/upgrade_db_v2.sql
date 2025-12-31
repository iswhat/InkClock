-- 数据库升级脚本：添加设备分组和标签功能

-- 创建设备分组表
CREATE TABLE IF NOT EXISTS `device_groups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL COMMENT '分组名称',
  `description` text COMMENT '分组描述',
  `user_id` int(11) NOT NULL COMMENT '创建者ID',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  CONSTRAINT `device_groups_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='设备分组表';

-- 创建设备分组关系表
CREATE TABLE IF NOT EXISTS `device_group_relations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `group_id` int(11) NOT NULL COMMENT '分组ID',
  `device_id` varchar(64) NOT NULL COMMENT '设备ID',
  `created_at` datetime NOT NULL COMMENT '添加时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `group_device` (`group_id`,`device_id`),
  KEY `group_id` (`group_id`),
  KEY `device_id` (`device_id`),
  CONSTRAINT `device_group_relations_ibfk_1` FOREIGN KEY (`group_id`) REFERENCES `device_groups` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `device_group_relations_ibfk_2` FOREIGN KEY (`device_id`) REFERENCES `devices` (`device_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='设备分组关系表';

-- 创建设备标签表
CREATE TABLE IF NOT EXISTS `device_tags` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL COMMENT '标签名称',
  `color` varchar(16) NOT NULL DEFAULT '#3498db' COMMENT '标签颜色',
  `user_id` int(11) NOT NULL COMMENT '创建者ID',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_tag` (`user_id`,`name`),
  KEY `user_id` (`user_id`),
  CONSTRAINT `device_tags_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='设备标签表';

-- 创建设备标签关系表
CREATE TABLE IF NOT EXISTS `device_tag_relations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tag_id` int(11) NOT NULL COMMENT '标签ID',
  `device_id` varchar(64) NOT NULL COMMENT '设备ID',
  `created_at` datetime NOT NULL COMMENT '添加时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `tag_device` (`tag_id`,`device_id`),
  KEY `tag_id` (`tag_id`),
  KEY `device_id` (`device_id`),
  CONSTRAINT `device_tag_relations_ibfk_1` FOREIGN KEY (`tag_id`) REFERENCES `device_tags` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `device_tag_relations_ibfk_2` FOREIGN KEY (`device_id`) REFERENCES `devices` (`device_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='设备标签关系表';

-- 创建消息模板表
CREATE TABLE IF NOT EXISTS `message_templates` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL COMMENT '模板名称',
  `content` text NOT NULL COMMENT '模板内容',
  `type` varchar(16) NOT NULL DEFAULT 'text' COMMENT '消息类型',
  `user_id` int(11) NOT NULL COMMENT '创建者ID',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  CONSTRAINT `message_templates_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息模板表';

-- 创建固件推送任务表
CREATE TABLE IF NOT EXISTS `firmware_push_tasks` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `firmware_id` int(11) NOT NULL COMMENT '固件ID',
  `user_id` int(11) NOT NULL COMMENT '创建者ID',
  `target_type` varchar(16) NOT NULL DEFAULT 'all' COMMENT '推送目标类型：all, group, device_list',
  `target_ids` text COMMENT '推送目标ID列表（JSON格式）',
  `status` varchar(16) NOT NULL DEFAULT 'pending' COMMENT '推送状态：pending, running, completed, failed',
  `progress` int(11) NOT NULL DEFAULT 0 COMMENT '推送进度（0-100）',
  `total_devices` int(11) NOT NULL DEFAULT 0 COMMENT '总设备数',
  `success_count` int(11) NOT NULL DEFAULT 0 COMMENT '成功设备数',
  `failed_count` int(11) NOT NULL DEFAULT 0 COMMENT '失败设备数',
  `scheduled_at` datetime DEFAULT NULL COMMENT '计划推送时间',
  `started_at` datetime DEFAULT NULL COMMENT '开始推送时间',
  `completed_at` datetime DEFAULT NULL COMMENT '完成推送时间',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `firmware_id` (`firmware_id`),
  KEY `user_id` (`user_id`),
  KEY `status` (`status`),
  CONSTRAINT `firmware_push_tasks_ibfk_1` FOREIGN KEY (`firmware_id`) REFERENCES `firmware_versions` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `firmware_push_tasks_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='固件推送任务表';

-- 创建固件推送日志表
CREATE TABLE IF NOT EXISTS `firmware_push_logs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `push_task_id` int(11) NOT NULL COMMENT '推送任务ID',
  `device_id` varchar(64) NOT NULL COMMENT '设备ID',
  `status` varchar(16) NOT NULL DEFAULT 'pending' COMMENT '推送状态：pending, success, failed',
  `error_message` text COMMENT '错误信息',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `push_task_id` (`push_task_id`),
  KEY `device_id` (`device_id`),
  KEY `status` (`status`),
  CONSTRAINT `firmware_push_logs_ibfk_1` FOREIGN KEY (`push_task_id`) REFERENCES `firmware_push_tasks` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `firmware_push_logs_ibfk_2` FOREIGN KEY (`device_id`) REFERENCES `devices` (`device_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='固件推送日志表';

-- 创建系统日志表
CREATE TABLE IF NOT EXISTS `system_logs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `level` varchar(16) NOT NULL DEFAULT 'info' COMMENT '日志级别：debug, info, warning, error, critical',
  `category` varchar(64) NOT NULL DEFAULT 'system' COMMENT '日志分类',
  `message` text NOT NULL COMMENT '日志消息',
  `user_id` int(11) DEFAULT NULL COMMENT '操作用户ID',
  `device_id` varchar(64) DEFAULT NULL COMMENT '相关设备ID',
  `ip_address` varchar(45) DEFAULT NULL COMMENT 'IP地址',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `level` (`level`),
  KEY `category` (`category`),
  KEY `user_id` (`user_id`),
  KEY `device_id` (`device_id`),
  KEY `created_at` (`created_at`),
  CONSTRAINT `system_logs_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `system_logs_ibfk_2` FOREIGN KEY (`device_id`) REFERENCES `devices` (`device_id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='系统日志表';

-- 创建通知表
CREATE TABLE IF NOT EXISTS `notifications` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL COMMENT '接收用户ID',
  `title` varchar(128) NOT NULL COMMENT '通知标题',
  `content` text NOT NULL COMMENT '通知内容',
  `type` varchar(16) NOT NULL DEFAULT 'system' COMMENT '通知类型：system, device, firmware, plugin',
  `status` varchar(16) NOT NULL DEFAULT 'unread' COMMENT '通知状态：unread, read',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `status` (`status`),
  KEY `created_at` (`created_at`),
  CONSTRAINT `notifications_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='通知表';
