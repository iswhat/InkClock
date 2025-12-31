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
    
    // 消息模板相关路由
    'POST /api/template' => 'MessageTemplateController@createTemplate',
    'GET /api/template' => 'MessageTemplateController@getTemplates',
    'GET /api/template/{id}' => 'MessageTemplateController@getTemplate',
    'PUT /api/template/{id}' => 'MessageTemplateController@updateTemplate',
    'DELETE /api/template/{id}' => 'MessageTemplateController@deleteTemplate',
    
    // 系统日志相关路由
    'GET /api/log' => 'SystemLogController@getLogs',
    'DELETE /api/log/old' => 'SystemLogController@deleteOldLogs',
    'POST /api/log' => 'SystemLogController@log',
    
    // 通知相关路由
    'GET /api/notification' => 'NotificationController@getNotifications',
    'GET /api/notification/{id}' => 'NotificationController@getNotification',
    'GET /api/notification/unread-count' => 'NotificationController@getUnreadCount',
    'PUT /api/notification' => 'NotificationController@markAllAsRead',
    'PUT /api/notification/{id}' => 'NotificationController@markAsRead',
    'DELETE /api/notification/{id}' => 'NotificationController@deleteNotification',
    
    // 插件相关路由
    'GET /api/plugin' => 'PluginController@getPlugins',
    'POST /api/plugin' => 'PluginController@addPlugin',
    'PUT /api/plugin/{index}' => 'PluginController@updatePlugin',
    'DELETE /api/plugin/{index}' => 'PluginController@deletePlugin',
    
    // 状态检查路由
    'GET /api/status' => 'StatusController@getStatus'
];
?>