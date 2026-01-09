<?php
/**
 * 插件初始化脚本
 * 将plugin.json中的插件导入到数据库中
 */

// 加载所有必要的类
require_once __DIR__ . '/src/Config/Config.php';
require_once __DIR__ . '/src/Utils/Logger.php';
require_once __DIR__ . '/src/Utils/Database.php';
require_once __DIR__ . '/src/Utils/PluginInitializer.php';

use InkClock\Utils\PluginInitializer;

echo "开始初始化插件...\n";

if (PluginInitializer::initialize()) {
    echo "插件初始化成功！\n";
} else {
    echo "插件初始化失败！\n";
}

echo "初始化完成。\n";
