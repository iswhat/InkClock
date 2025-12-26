-- 初始化数据库表结构

-- 创建设备表
CREATE TABLE IF NOT EXISTS `devices` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `device_id` varchar(64) NOT NULL COMMENT '设备唯一标识',
  `mac_address` varchar(64) DEFAULT '' COMMENT 'MAC地址',
  `ip_address` varchar(45) DEFAULT '' COMMENT 'IP地址',
  `ipv6_address` varchar(128) DEFAULT '' COMMENT 'IPv6地址',
  `model` varchar(64) DEFAULT '' COMMENT '设备型号',
  `firmware_version` varchar(32) DEFAULT 'unknown' COMMENT '固件版本',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  `last_active` datetime NOT NULL COMMENT '最后活跃时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `device_id` (`device_id`),
  KEY `mac_address` (`mac_address`),
  KEY `last_active` (`last_active`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='设备表';

-- 创建消息表
CREATE TABLE IF NOT EXISTS `messages` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `message_id` varchar(64) NOT NULL COMMENT '消息唯一标识',
  `device_id` varchar(64) NOT NULL COMMENT '设备ID',
  `sender` varchar(64) NOT NULL DEFAULT 'Unknown' COMMENT '发送者',
  `content` text NOT NULL COMMENT '消息内容（文本或文件路径）',
  `type` varchar(16) NOT NULL DEFAULT 'text' COMMENT '消息类型：text, image, audio',
  `status` varchar(16) NOT NULL DEFAULT 'unread' COMMENT '消息状态：unread, read',
  `created_at` datetime NOT NULL COMMENT '创建时间',
  `read_at` datetime DEFAULT NULL COMMENT '阅读时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `message_id` (`message_id`),
  KEY `device_id` (`device_id`),
  KEY `status` (`status`),
  KEY `created_at` (`created_at`),
  CONSTRAINT `messages_ibfk_1` FOREIGN KEY (`device_id`) REFERENCES `devices` (`device_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息表';