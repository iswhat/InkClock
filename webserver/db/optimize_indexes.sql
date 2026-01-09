-- 数据库索引优化脚本
-- 执行此脚本以添加必要的索引，提高查询性能

-- users表索引
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_api_key ON users(api_key);
CREATE INDEX IF NOT EXISTS idx_users_is_admin ON users(is_admin);
CREATE INDEX IF NOT EXISTS idx_users_status ON users(status);

-- devices表索引
CREATE INDEX IF NOT EXISTS idx_devices_device_id ON devices(device_id);
CREATE INDEX IF NOT EXISTS idx_devices_model ON devices(model);
CREATE INDEX IF NOT EXISTS idx_devices_firmware_version ON devices(firmware_version);
CREATE INDEX IF NOT EXISTS idx_devices_connection_status ON devices(connection_status);
CREATE INDEX IF NOT EXISTS idx_devices_last_active ON devices(last_active);

-- messages表索引
CREATE INDEX IF NOT EXISTS idx_messages_device_id ON messages(device_id);
CREATE INDEX IF NOT EXISTS idx_messages_status ON messages(status);
CREATE INDEX IF NOT EXISTS idx_messages_sync_status ON messages(sync_status);
CREATE INDEX IF NOT EXISTS idx_messages_created_at ON messages(created_at);
CREATE INDEX IF NOT EXISTS idx_messages_device_status ON messages(device_id, status);
CREATE INDEX IF NOT EXISTS idx_messages_device_sync ON messages(device_id, sync_status);

-- user_devices表索引
CREATE INDEX IF NOT EXISTS idx_user_devices_user_id ON user_devices(user_id);
CREATE INDEX IF NOT EXISTS idx_user_devices_device_id ON user_devices(device_id);
CREATE INDEX IF NOT EXISTS idx_user_devices_user_device ON user_devices(user_id, device_id);

-- firmware_versions表索引（如果存在）
CREATE INDEX IF NOT EXISTS idx_firmware_model ON firmware_versions(model);
CREATE INDEX IF NOT EXISTS idx_firmware_version ON firmware_versions(version);
CREATE INDEX IF NOT EXISTS idx_firmware_active ON firmware_versions(is_active);

-- device_groups表索引（如果存在）
CREATE INDEX IF NOT EXISTS idx_device_groups_name ON device_groups(name);
CREATE INDEX IF NOT EXISTS idx_device_groups_parent_id ON device_groups(parent_id);

-- device_tags表索引（如果存在）
CREATE INDEX IF NOT EXISTS idx_device_tags_name ON device_tags(name);

-- notifications表索引（如果存在）
CREATE INDEX IF NOT EXISTS idx_notifications_user_id ON notifications(user_id);
CREATE INDEX IF NOT EXISTS idx_notifications_status ON notifications(status);
CREATE INDEX IF NOT EXISTS idx_notifications_created_at ON notifications(created_at);

-- 查看所有索引
SELECT name, tbl_name, sql FROM sqlite_master WHERE type='index';
