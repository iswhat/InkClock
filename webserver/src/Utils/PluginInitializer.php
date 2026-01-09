<?php
/**
 * 插件初始化工具类
 */

namespace InkClock\Utils;

class PluginInitializer {
    /**
     * 初始化插件数据
     * 将plugin.json中的插件导入到数据库中
     */
    public static function initialize() {
        $db = Database::getInstance();
        $pluginJsonPath = __DIR__ . '/../../plugin/plugin.json';
        
        if (!file_exists($pluginJsonPath)) {
            Logger::getInstance()->error('插件配置文件不存在', ['path' => $pluginJsonPath]);
            return false;
        }
        
        $pluginJsonContent = file_get_contents($pluginJsonPath);
        $plugins = json_decode($pluginJsonContent, true);
        
        if (!is_array($plugins)) {
            Logger::getInstance()->error('插件配置文件格式错误', ['path' => $pluginJsonPath]);
            return false;
        }
        
        foreach ($plugins as $plugin) {
            // 检查插件是否已存在
            $existingPlugin = $db->querySingle(
                "SELECT id FROM plugins WHERE url = ?",
                ['url' => $plugin['url']]
            );
            
            if (!$existingPlugin) {
                // 插入新插件
                $db->execute(
                    "INSERT INTO plugins (name, description, url, type, status, author, version, refresh_interval, settings_url, created_by, approval_status) 
                     VALUES (?, ?, ?, 'system', 'enabled', 'system', '1.0.0', ?, ?, NULL, 'approved')",
                    [
                        'name' => $plugin['name'],
                        'description' => $plugin['description'],
                        'url' => $plugin['url'],
                        'refresh_interval' => $plugin['refresh_interval'],
                        'settings_url' => $plugin['settings_url']
                    ]
                );
                
                Logger::getInstance()->info('插件初始化成功', ['name' => $plugin['name']]);
            }
        }
        
        return true;
    }
}
