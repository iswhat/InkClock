<?php
/**
 * API路由配置
 */

return [
    // 用户相关路由
    'POST /api/user/register' => 'UserController@register',
    'POST /api/user/login' => 'UserController@login',
    'GET /api/user/info' => 'UserController@getInfo',
    'GET /api/user/devices' => 'UserController@getDevices',
    'POST /api/user/bind' => 'UserController@bindDevice',
    'POST /api/user/unbind' => 'UserController@unbindDevice',
    'PUT /api/user/device' => 'UserController@updateDeviceNickname',
    'GET /api/user/exist' => 'UserController@checkUsers',
    'POST /api/user/first-admin' => 'UserController@createFirstAdmin',
    // 管理员用户管理路由
    'GET /api/user' => 'UserController@getUsers',
    'POST /api/user' => 'UserController@addUser',
    'GET /api/user/stats' => 'UserController@getUserStats',
    'GET /api/user/{id}' => 'UserController@getUser',
    'PUT /api/user/{id}' => 'UserController@updateUser',
    'DELETE /api/user/{id}' => 'UserController@deleteUser',
    
    // 设备相关路由
    'POST /api/device' => 'DeviceController@registerDevice',
    'GET /api/device' => 'DeviceController@getDevices',
    'GET /api/device/{id}' => 'DeviceController@getDevice',
    'DELETE /api/device/{id}' => 'DeviceController@deleteDevice',
    
    // 消息相关路由
    'POST /api/message' => 'MessageController@sendMessage',
    'GET /api/message/{deviceId}' => 'MessageController@getMessages',
    'GET /api/message/{deviceId}/unread' => 'MessageController@getUnreadMessages',
    'PUT /api/message/{deviceId}/read' => 'MessageController@markAsRead',
    'DELETE /api/message/{deviceId}/{messageId}' => 'MessageController@deleteMessage',
    
    // 固件相关路由
    'POST /api/firmware' => 'FirmwareController@addVersion',
    'GET /api/firmware' => 'FirmwareController@getAllVersions',
    'GET /api/firmware/active/{model}' => 'FirmwareController@getActiveVersion',
    'GET /api/firmware/{id}' => 'FirmwareController@getVersion',
    'PUT /api/firmware/{id}' => 'FirmwareController@updateVersion',
    'PUT /api/firmware/publish/{id}' => 'FirmwareController@publishVersion',
    'DELETE /api/firmware/{id}' => 'FirmwareController@deleteVersion',
    
    // 设备分组相关路由
    'POST /api/group' => 'DeviceGroupController@createGroup',
    'GET /api/group' => 'DeviceGroupController@getGroups',
    'GET /api/group/{id}' => 'DeviceGroupController@getGroup',
    'PUT /api/group/{id}' => 'DeviceGroupController@updateGroup',
    'DELETE /api/group/{id}' => 'DeviceGroupController@deleteGroup',
    'POST /api/group/add_device' => 'DeviceGroupController@addDeviceToGroup',
    'POST /api/group/remove_device' => 'DeviceGroupController@removeDeviceFromGroup',
    'GET /api/group/devices/{id}' => 'DeviceGroupController@getDevicesByGroup',
    
    // 设备标签相关路由
    'POST /api/tag' => 'DeviceTagController@createTag',
    'GET /api/tag' => 'DeviceTagController@getTags',
    'GET /api/tag/{id}' => 'DeviceTagController@getTag',
    'PUT /api/tag/{id}' => 'DeviceTagController@updateTag',
    'DELETE /api/tag/{id}' => 'DeviceTagController@deleteTag',
    'POST /api/tag/add_device' => 'DeviceTagController@addTagToDevice',
    'POST /api/tag/remove_device' => 'DeviceTagController@removeTagFromDevice',
    'POST /api/tag/batch_add' => 'DeviceTagController@batchAddTagsToDevices',
    'POST /api/tag/batch_remove' => 'DeviceTagController@batchRemoveTagsFromDevices',
    'GET /api/tag/devices/{id}' => 'DeviceTagController@getDevicesByTag',
    'GET /api/tag/device_tags/{deviceId}' => 'DeviceTagController@getTagsByDevice',
    
    // 固件推送任务相关路由
    'POST /api/push_task' => 'FirmwarePushTaskController@createPushTask',
    'GET /api/push_task' => 'FirmwarePushTaskController@getPushTasks',
    'GET /api/push_task/{id}' => 'FirmwarePushTaskController@getPushTask',
    'PUT /api/push_task/{id}' => 'FirmwarePushTaskController@updatePushTaskStatus',
    'DELETE /api/push_task/{id}' => 'FirmwarePushTaskController@deletePushTask',
    'POST /api/push_task/add_log' => 'FirmwarePushTaskController@addPushLog',
    'POST /api/push_task/batch_log' => 'FirmwarePushTaskController@batchAddPushLogs',
    

    

    
    // 通知相关路由
    'GET /api/notification' => 'NotificationController@getNotifications',
    'GET /api/notification/unread-count' => 'NotificationController@getUnreadCount',
    'GET /api/notification/stats' => 'NotificationController@getNotificationStats',
    'GET /api/notification/{id}' => 'NotificationController@getNotification',
    'POST /api/notification/publish' => 'NotificationController@publishNotification',
    'PUT /api/notification' => 'NotificationController@markAllAsRead',
    'PUT /api/notification/{id}' => 'NotificationController@markAsRead',
    'DELETE /api/notification/{id}' => 'NotificationController@deleteNotification',
    
    // 插件相关路由
    'GET /api/plugin' => 'PluginController@getPlugins',
    'GET /api/plugin/stats' => 'PluginController@getPluginStats',
    'GET /api/plugin/device/{deviceId}' => 'PluginController@getDevicePlugins',
    'GET /api/plugin/pending' => 'PluginController@getPendingPlugins',
    'POST /api/plugin/external' => 'PluginController@addExternalPlugin',
    'POST /api/plugin/upload' => 'PluginController@uploadPlugin',
    'POST /api/plugin/initialize' => 'PluginController@initializePlugins',
    'POST /api/plugin/init' => 'PluginController@initPluginsApi',
    'GET /api/plugin/{id}' => 'PluginController@getPlugin',
    'PUT /api/plugin/{id}/toggle' => 'PluginController@togglePlugin',
    'POST /api/plugin/approve/{id}' => 'PluginController@approvePlugin',
    'POST /api/plugin/device/{deviceId}/enable/{pluginId}' => 'PluginController@enablePluginForDevice',
    'POST /api/plugin/device/{deviceId}/disable/{pluginId}' => 'PluginController@disablePluginForDevice',
    'DELETE /api/plugin/{id}' => 'PluginController@deletePlugin',
    
    // 状态检查路由
    'GET /api/status' => 'StatusController@getStatus',
    
    // 系统设置相关路由
    'GET /api/system/settings' => 'SystemController@getSettings',
    'PUT /api/system/settings' => 'SystemController@updateSettings',
    'POST /api/system/settings/reset' => 'SystemController@resetSettings',
    'GET /api/system/info' => 'SystemController@getSystemInfo',
    'GET /api/system/database/backup' => 'SystemController@backupDatabase',
    'POST /api/system/restart' => 'SystemController@restartSystem',
    'POST /api/system/cache/clear' => 'SystemController@clearCache'
];
?>