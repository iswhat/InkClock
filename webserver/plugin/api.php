<?php
/**
 * 插件API管理
 * 提供插件相关的API函数
 */

// 插件JSON文件路径
$pluginJsonPath = __DIR__ . '/plugin.json';

/**
 * 获取插件列表
 * @return array 插件列表
 */
function get_plugins() {
    global $pluginJsonPath;
    
    if (file_exists($pluginJsonPath)) {
        $content = file_get_contents($pluginJsonPath);
        $plugins = json_decode($content, true);
        if (json_last_error() !== JSON_ERROR_NONE) {
            // JSON解析错误，返回空数组
            return [];
        }
        return $plugins ?? [];
    }
    return [];
}

/**
 * 添加插件
 * @param array $data 插件数据
 * @return array 操作结果
 */
function add_plugin($data) {
    global $pluginJsonPath;
    
    $plugins = [];
    if (file_exists($pluginJsonPath)) {
        $plugins = json_decode(file_get_contents($pluginJsonPath), true) ?? [];
    }
    
    $newPlugin = [
        'name' => $data['name'] ?? '',
        'url' => $data['url'] ?? '',
        'description' => $data['description'] ?? '',
        'refresh_interval' => $data['refresh_interval'] ?? '',
        'settings_url' => $data['settings_url'] ?? ''
    ];
    
    // 验证必填字段
    if (empty($newPlugin['name']) || empty($newPlugin['url'])) {
        return ['success' => false, 'error' => '插件名称和URL不能为空'];
    }
    
    $plugins[] = $newPlugin;
    $result = file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
    
    if ($result) {
        return ['success' => true, 'plugin_id' => count($plugins) - 1];
    } else {
        return ['success' => false, 'error' => '添加插件失败'];
    }
}

/**
 * 更新插件
 * @param int $index 插件索引
 * @param array $data 更新数据
 * @return array 操作结果
 */
function update_plugin($index, $data) {
    global $pluginJsonPath;
    
    if (!file_exists($pluginJsonPath)) {
        return ['success' => false, 'error' => '插件列表不存在'];
    }
    
    $plugins = json_decode(file_get_contents($pluginJsonPath), true) ?? [];
    
    if (!isset($plugins[$index])) {
        return ['success' => false, 'error' => '插件不存在'];
    }
    
    // 更新插件数据
    $plugins[$index]['name'] = $data['name'] ?? $plugins[$index]['name'];
    $plugins[$index]['url'] = $data['url'] ?? $plugins[$index]['url'];
    $plugins[$index]['description'] = $data['description'] ?? $plugins[$index]['description'];
    $plugins[$index]['refresh_interval'] = $data['refresh_interval'] ?? $plugins[$index]['refresh_interval'];
    $plugins[$index]['settings_url'] = $data['settings_url'] ?? $plugins[$index]['settings_url'];
    
    $result = file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
    
    if ($result) {
        return ['success' => true];
    } else {
        return ['success' => false, 'error' => '更新插件失败'];
    }
}

/**
 * 删除插件
 * @param int $index 插件索引
 * @return array 操作结果
 */
function delete_plugin($index) {
    global $pluginJsonPath;
    
    if (!file_exists($pluginJsonPath)) {
        return ['success' => false, 'error' => '插件列表不存在'];
    }
    
    $plugins = json_decode(file_get_contents($pluginJsonPath), true) ?? [];
    
    if (!isset($plugins[$index])) {
        return ['success' => false, 'error' => '插件不存在'];
    }
    
    array_splice($plugins, $index, 1);
    $result = file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
    
    if ($result) {
        return ['success' => true];
    } else {
        return ['success' => false, 'error' => '删除插件失败'];
    }
}
?>