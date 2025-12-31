<?php
/**
 * API接口入口
 */
require_once 'config.php';
require_once 'models/Device.php';
require_once 'models/Message.php';
require_once 'models/FirmwareVersion.php';
require_once 'models/User.php';
require_once 'models/DeviceGroup.php';
require_once 'models/DeviceTag.php';
require_once 'models/FirmwarePushTask.php';
require_once 'models/MessageTemplate.php';
require_once 'models/SystemLog.php';
require_once 'models/Notification.php';

// 处理CORS
handleCORS();

// 获取请求方法和路径
$method = $_SERVER['REQUEST_METHOD'];
$path = isset($_GET['path']) ? $_GET['path'] : '';
$path = trim($path, '/');
$parts = explode('/', $path);

// 初始化模型
$deviceModel = new Device();
$messageModel = new Message();
$firmwareModel = new FirmwareVersion();
$userModel = new User();
$deviceGroupModel = new DeviceGroup();
$deviceTagModel = new DeviceTag();
$firmwarePushTaskModel = new FirmwarePushTask();
$messageTemplateModel = new MessageTemplate();
$systemLogModel = new SystemLog();
$notificationModel = new Notification();

// 解析API密钥
$apiKey = isset($_GET['api_key']) ? $_GET['api_key'] : (isset($_SERVER['HTTP_X_API_KEY']) ? $_SERVER['HTTP_X_API_KEY'] : '');
$currentUser = $apiKey ? $userModel->getUserByApiKey($apiKey) : null;

// 验证API访问权限
function checkApiPermission($required = false) {
    global $currentUser, $apiKey;
    
    if ($required && !$currentUser) {
        sendJsonResponse(array('error' => '未授权访问', 'code' => 401), 401);
        exit;
    }
    
    return $currentUser;
}

// 路由处理
switch ($parts[0]) {
    case 'user':
        handleUserApi($method, $parts);
        break;
    case 'device':
        handleDeviceApi($method, $parts);
        break;
    case 'message':
        handleMessageApi($method, $parts);
        break;
    case 'status':
        handleStatusApi();
        break;
    case 'firmware':
        handleFirmwareApi($method, $parts);
        break;
    case 'group':
        handleDeviceGroupApi($method, $parts);
        break;
    case 'tag':
        handleDeviceTagApi($method, $parts);
        break;
    case 'push_task':
        handleFirmwarePushTaskApi($method, $parts);
        break;
    case 'template':
        handleMessageTemplateApi($method, $parts);
        break;
    case 'log':
        handleSystemLogApi($method, $parts);
        break;
    case 'notification':
        handleNotificationApi($method, $parts);
        break;
    case 'plugin':
        handlePluginApi($method, $parts);
        break;
    default:
        sendJsonResponse(array(
            'error' => 'API路径不存在',
            'path' => $path
        ), 404);
        break;
}

/**
 * 处理设备相关API
 */
function handleDeviceApi($method, $parts) {
    global $deviceModel, $currentUser, $userModel;
    
    switch ($method) {
        case 'POST':
            // 注册或更新设备
            $data = json_decode(file_get_contents('php://input'), true);
            if (!$data) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            $result = $deviceModel->registerDevice($data);
            
            // 如果用户已登录，自动绑定设备
            if ($currentUser && $result['success']) {
                $userModel->bindDevice($currentUser['id'], $result['device_id']);
            }
            
            sendJsonResponse($result);
            break;
        case 'GET':
            if (isset($parts[1])) {
                // 获取单个设备信息
                $deviceId = $parts[1];
                
                // 检查设备访问权限
                checkDevicePermission($deviceId);
                
                $device = $deviceModel->getDevice($deviceId);
                if ($device) {
                    sendJsonResponse(array('device' => $device));
                } else {
                    sendJsonResponse(array('error' => '设备不存在'), 404);
                }
            } else {
                // 获取设备列表（仅返回当前用户绑定的设备）
                $user = checkApiPermission(true);
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $devices = $userModel->getUserDevices($user['id'], $limit, $offset);
                sendJsonResponse(array('devices' => $devices));
            }
            break;
        case 'DELETE':
            if (isset($parts[1])) {
                // 删除设备
                $deviceId = $parts[1];
                
                // 检查设备访问权限
                checkDevicePermission($deviceId);
                
                $result = $deviceModel->deleteDevice($deviceId);
                sendJsonResponse(array('success' => $result));
            } else {
                sendJsonResponse(array('error' => '缺少设备ID'), 400);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
        }
    }
}

/**
 * 处理插件管理相关API
 */
function handlePluginApi($method, $parts) {
    global $currentUser;
    
    // 插件JSON文件路径
    $pluginJsonPath = __DIR__ . '/plugin/plugin.json';
    
    // 读取现有插件数据
    $plugins = array();
    if (file_exists($pluginJsonPath)) {
        $plugins = json_decode(file_get_contents($pluginJsonPath), true) ?? array();
    }
    
    switch ($method) {
        case 'GET':
            // 获取插件列表
            sendJsonResponse(array('plugins' => $plugins));
            break;
        case 'POST':
            // 添加新插件
            $user = checkApiPermission(true);
            $data = json_decode(file_get_contents('php://input'), true);
            
            if (!$data || !isset($data['name']) || !isset($data['url'])) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            
            $newPlugin = array(
                'name' => $data['name'],
                'url' => $data['url'],
                'description' => $data['description'] ?? '',
                'refresh_interval' => $data['refresh_interval'] ?? '',
                'settings_url' => $data['settings_url'] ?? ''
            );
            
            $plugins[] = $newPlugin;
            file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
            sendJsonResponse(array('success' => true, 'plugin' => $newPlugin));
            break;
        case 'DELETE':
            // 删除插件
            $user = checkApiPermission(true);
            
            if (!isset($parts[1])) {
                sendJsonResponse(array('error' => '缺少插件索引'), 400);
            }
            
            $index = intval($parts[1]);
            if (isset($plugins[$index])) {
                $deletedPlugin = $plugins[$index];
                array_splice($plugins, $index, 1);
                file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
                sendJsonResponse(array('success' => true, 'deleted_plugin' => $deletedPlugin));
            } else {
                sendJsonResponse(array('error' => '插件不存在'), 404);
            }
            break;
        case 'PUT':
            // 更新插件
            $user = checkApiPermission(true);
            
            if (!isset($parts[1])) {
                sendJsonResponse(array('error' => '缺少插件索引'), 400);
            }
            
            $index = intval($parts[1]);
            $data = json_decode(file_get_contents('php://input'), true);
            
            if (!isset($plugins[$index])) {
                sendJsonResponse(array('error' => '插件不存在'), 404);
            }
            
            if (!$data) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            
            // 更新插件数据
            $plugins[$index] = array_merge($plugins[$index], $data);
            file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
            sendJsonResponse(array('success' => true, 'plugin' => $plugins[$index]));
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 处理通知相关API
 */
function handleNotificationApi($method, $parts) {
    global $notificationModel, $currentUser;
    
    $user = checkApiPermission(true);
    
    switch ($method) {
        case 'GET':
            if (!isset($parts[1])) {
                // 获取通知列表
                $status = isset($_GET['status']) ? $_GET['status'] : null;
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                
                $notifications = $notificationModel->getNotifications($user['id'], $status, $limit, $offset);
                $unreadCount = $notificationModel->getUnreadCount($user['id']);
                
                sendJsonResponse(array(
                    'notifications' => $notifications,
                    'unread_count' => $unreadCount
                ));
            } elseif (is_numeric($parts[1])) {
                // 获取单个通知详情
                $notificationId = intval($parts[1]);
                $notification = $notificationModel->getNotificationById($notificationId, $user['id']);
                if ($notification) {
                    // 自动标记为已读
                    $notificationModel->markAsRead($notificationId, $user['id']);
                    sendJsonResponse(array('notification' => $notification));
                } else {
                    sendJsonResponse(array('error' => '通知不存在'), 404);
                }
            } elseif ($parts[1] == 'unread-count') {
                // 获取未读通知数量
                $unreadCount = $notificationModel->getUnreadCount($user['id']);
                sendJsonResponse(array('unread_count' => $unreadCount));
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'PUT':
            if (!isset($parts[1])) {
                // 标记所有通知为已读
                $result = $notificationModel->markAllAsRead($user['id']);
                sendJsonResponse($result);
            } elseif (is_numeric($parts[1])) {
                // 标记单个通知为已读
                $notificationId = intval($parts[1]);
                $result = $notificationModel->markAsRead($notificationId, $user['id']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'DELETE':
            if (is_numeric($parts[1])) {
                // 删除单个通知
                $notificationId = intval($parts[1]);
                $result = $notificationModel->deleteNotification($notificationId, $user['id']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 处理系统日志相关API
 */
function handleSystemLogApi($method, $parts) {
    global $systemLogModel, $currentUser;
    
    $user = checkApiPermission(true);
    
    switch ($method) {
        case 'GET':
            // 获取日志列表
            $filters = array();
            
            // 解析过滤参数
            if (isset($_GET['level'])) {
                $filters['level'] = $_GET['level'];
            }
            if (isset($_GET['category'])) {
                $filters['category'] = $_GET['category'];
            }
            if (isset($_GET['user_id'])) {
                $filters['user_id'] = $_GET['user_id'];
            }
            if (isset($_GET['device_id'])) {
                $filters['device_id'] = $_GET['device_id'];
            }
            if (isset($_GET['start_time'])) {
                $filters['start_time'] = $_GET['start_time'];
            }
            if (isset($_GET['end_time'])) {
                $filters['end_time'] = $_GET['end_time'];
            }
            
            $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
            $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
            
            $logs = $systemLogModel->getLogs($filters, $limit, $offset);
            $count = $systemLogModel->getLogCount($filters);
            
            sendJsonResponse(array(
                'logs' => $logs,
                'total' => $count,
                'limit' => $limit,
                'offset' => $offset
            ));
            break;
        case 'DELETE':
            if (isset($parts[1]) && $parts[1] == 'old') {
                // 删除旧日志
                $days = isset($_GET['days']) ? intval($_GET['days']) : 30;
                $result = $systemLogModel->deleteOldLogs($days);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'POST':
            // 手动记录日志（可选功能）
            $data = json_decode(file_get_contents('php://input'), true);
            if (!$data || !isset($data['message'])) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            
            $level = isset($data['level']) ? $data['level'] : 'info';
            $category = isset($data['category']) ? $data['category'] : 'manual';
            $message = $data['message'];
            $deviceId = isset($data['device_id']) ? $data['device_id'] : null;
            
            $result = $systemLogModel->log($level, $category, $message, $user['id'], $deviceId);
            sendJsonResponse($result);
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 处理消息模板相关API
 */
function handleMessageTemplateApi($method, $parts) {
    global $messageTemplateModel, $currentUser;
    
    $user = checkApiPermission(true);
    
    switch ($method) {
        case 'POST':
            // 创建消息模板
            $data = json_decode(file_get_contents('php://input'), true);
            if (!$data || !isset($data['name']) || !isset($data['content'])) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            $name = $data['name'];
            $content = $data['content'];
            $type = isset($data['type']) ? $data['type'] : 'text';
            $result = $messageTemplateModel->createTemplate($user['id'], $name, $content, $type);
            sendJsonResponse($result);
            break;
        case 'GET':
            if (!isset($parts[1])) {
                // 获取模板列表
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $templates = $messageTemplateModel->getTemplatesByUserId($user['id'], $limit, $offset);
                sendJsonResponse(array('templates' => $templates));
            } elseif (is_numeric($parts[1])) {
                // 获取模板详情
                $templateId = intval($parts[1]);
                $template = $messageTemplateModel->getTemplateById($templateId, $user['id']);
                if ($template) {
                    sendJsonResponse(array('template' => $template));
                } else {
                    sendJsonResponse(array('error' => '模板不存在'), 404);
                }
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'PUT':
            if (is_numeric($parts[1])) {
                // 更新模板
                $templateId = intval($parts[1]);
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['name']) || !isset($data['content'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $name = $data['name'];
                $content = $data['content'];
                $type = isset($data['type']) ? $data['type'] : 'text';
                $result = $messageTemplateModel->updateTemplate($templateId, $user['id'], $name, $content, $type);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'DELETE':
            if (is_numeric($parts[1])) {
                // 删除模板
                $templateId = intval($parts[1]);
                $result = $messageTemplateModel->deleteTemplate($templateId, $user['id']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 处理消息相关API
 */
function handleMessageApi($method, $parts) {
    global $messageModel, $currentUser;
    
    switch ($method) {
        case 'POST':
            // 发送消息
            $data = json_decode(file_get_contents('php://input'), true);
            if (!$data || !isset($data['device_id']) || !isset($data['content'])) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            
            $deviceId = $data['device_id'];
            
            // 检查设备访问权限
            checkDevicePermission($deviceId);
            
            // 添加用户ID
            if ($currentUser) {
                $data['user_id'] = $currentUser['id'];
            }
            
            $result = $messageModel->sendMessage($data);
            sendJsonResponse($result);
            break;
        case 'GET':
            if (isset($parts[1])) {
                $deviceId = $parts[1];
                
                // 检查设备访问权限
                checkDevicePermission($deviceId);
                
                if (isset($parts[2]) && $parts[2] == 'unread') {
                    // 获取未读消息
                    $messages = $messageModel->getUnreadMessages($deviceId);
                    sendJsonResponse(array('messages' => $messages));
                } else {
                    // 获取所有消息
                    $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 20;
                    $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                    $messages = $messageModel->getAllMessages($deviceId, $limit, $offset);
                    sendJsonResponse(array('messages' => $messages));
                }
            } else {
                sendJsonResponse(array('error' => '缺少设备ID'), 400);
            }
            break;
        case 'PUT':
            if (isset($parts[1]) && isset($parts[2]) && $parts[2] == 'read') {
                // 标记消息为已读
                $deviceId = $parts[1];
                
                // 检查设备访问权限
                checkDevicePermission($deviceId);
                
                $data = json_decode(file_get_contents('php://input'), true);
                
                if (isset($data['message_id'])) {
                    // 标记单条消息为已读
                    $messageId = $data['message_id'];
                    $result = $messageModel->markAsRead($messageId, $deviceId);
                    sendJsonResponse(array('success' => $result));
                } else {
                    // 标记所有消息为已读
                    $result = $messageModel->markAllAsRead($deviceId);
                    sendJsonResponse(array('success' => $result));
                }
            } else {
                sendJsonResponse(array('error' => '无效的请求路径'), 400);
            }
            break;
        case 'DELETE':
            if (isset($parts[1]) && isset($parts[2])) {
                // 删除消息
                $deviceId = $parts[1];
                
                // 检查设备访问权限
                checkDevicePermission($deviceId);
                
                $messageId = $parts[2];
                $result = $messageModel->deleteMessage($messageId, $deviceId);
                sendJsonResponse(array('success' => $result));
            } else {
                sendJsonResponse(array('error' => '缺少设备ID或消息ID'), 400);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 处理状态检查API
 */
function handleStatusApi() {
    global $deviceModel, $messageModel;
    
    // 删除过期消息
    $expiredCount = $messageModel->deleteExpiredMessages();
    
    sendJsonResponse(array(
        'status' => 'ok',
        'time' => date('Y-m-d H:i:s'),
        'expired_messages_deleted' => $expiredCount
    ));
}

/**
 * 处理固件版本相关API
 */
function handleFirmwareApi($method, $parts) {
    global $firmwareModel, $currentUser;
    
    switch ($method) {
        case 'POST':
            // 添加新的固件版本（需要授权）
            checkApiPermission(true);
            
            $data = json_decode(file_get_contents('php://input'), true);
            if (!$data) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            
            // 添加发布者ID
            $data['publisher_id'] = $currentUser['id'];
            
            $result = $firmwareModel->addVersion($data);
            sendJsonResponse($result);
            break;
        case 'GET':
            if (isset($parts[1])) {
                if ($parts[1] == 'active') {
                    // 获取特定设备型号的活跃版本（不需要授权，设备可以直接访问）
                    if (!isset($parts[2])) {
                        sendJsonResponse(array('error' => '缺少设备型号'), 400);
                    }
                    $model = $parts[2];
                    $version = $firmwareModel->getActiveVersion($model);
                    if ($version) {
                        sendJsonResponse(array('firmware' => $version));
                    } else {
                        sendJsonResponse(array('error' => '未找到活跃的固件版本'), 404);
                    }
                } elseif (is_numeric($parts[1])) {
                    // 获取特定版本（需要授权）
                    checkApiPermission(true);
                    
                    $id = intval($parts[1]);
                    $version = $firmwareModel->getVersion($id);
                    if ($version) {
                        sendJsonResponse(array('firmware' => $version));
                    } else {
                        sendJsonResponse(array('error' => '固件版本不存在'), 404);
                    }
                } else {
                    // 获取特定设备型号的所有版本（需要授权）
                    checkApiPermission(true);
                    
                    $model = $parts[1];
                    $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                    $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                    $versions = $firmwareModel->getVersionsByModel($model, $limit, $offset);
                    sendJsonResponse(array('firmware_versions' => $versions));
                }
            } else {
                // 获取所有固件版本（需要授权）
                checkApiPermission(true);
                
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $versions = $firmwareModel->getAllVersions($limit, $offset);
                sendJsonResponse(array('firmware_versions' => $versions));
            }
            break;
        case 'PUT':
            // 更新固件版本（需要授权）
            checkApiPermission(true);
            
            if (isset($parts[1])) {
                if (is_numeric($parts[1])) {
                    // 更新固件版本
                    $id = intval($parts[1]);
                    $data = json_decode(file_get_contents('php://input'), true);
                    if (!$data) {
                        sendJsonResponse(array('error' => '无效的请求数据'), 400);
                    }
                    $result = $firmwareModel->updateVersion($id, $data);
                    sendJsonResponse($result);
                } elseif ($parts[1] == 'publish' && isset($parts[2])) {
                    // 发布固件版本
                    $id = intval($parts[2]);
                    $result = $firmwareModel->publishVersion($id);
                    sendJsonResponse($result);
                } else {
                    sendJsonResponse(array('error' => '无效的请求路径'), 400);
                }
            } else {
                sendJsonResponse(array('error' => '缺少版本ID'), 400);
            }
            break;
        case 'DELETE':
            // 删除固件版本（需要授权）
            checkApiPermission(true);
            
            if (isset($parts[1]) && is_numeric($parts[1])) {
                // 删除固件版本
                $id = intval($parts[1]);
                $result = $firmwareModel->deleteVersion($id);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '缺少有效的版本ID'), 400);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}
/**
 * 处理用户相关API
 */
function handleUserApi($method, $parts) {
    global $userModel, $currentUser;
    
    switch ($method) {
        case 'POST':
            if (isset($parts[1])) {
                if ($parts[1] == 'register') {
                    // 用户注册
                    $data = json_decode(file_get_contents('php://input'), true);
                    if (!$data || !isset($data['username']) || !isset($data['email']) || !isset($data['password'])) {
                        sendJsonResponse(array('error' => '无效的请求数据'), 400);
                    }
                    $result = $userModel->register($data);
                    sendJsonResponse($result);
                } elseif ($parts[1] == 'login') {
                    // 用户登录
                    $data = json_decode(file_get_contents('php://input'), true);
                    if (!$data || !isset($data['username']) || !isset($data['password'])) {
                        sendJsonResponse(array('error' => '无效的请求数据'), 400);
                    }
                    $result = $userModel->login($data['username'], $data['password']);
                    sendJsonResponse($result);
                } elseif ($parts[1] == 'bind') {
                    // 绑定设备
                    $user = checkApiPermission(true);
                    $data = json_decode(file_get_contents('php://input'), true);
                    if (!$data || !isset($data['device_id'])) {
                        sendJsonResponse(array('error' => '无效的请求数据'), 400);
                    }
                    $nickname = isset($data['nickname']) ? $data['nickname'] : '';
                    $result = $userModel->bindDevice($user['id'], $data['device_id'], $nickname);
                    sendJsonResponse($result);
                } elseif ($parts[1] == 'unbind') {
                    // 解绑设备
                    $user = checkApiPermission(true);
                    $data = json_decode(file_get_contents('php://input'), true);
                    if (!$data || !isset($data['device_id'])) {
                        sendJsonResponse(array('error' => '无效的请求数据'), 400);
                    }
                    $result = $userModel->unbindDevice($user['id'], $data['device_id']);
                    sendJsonResponse($result);
                } else {
                    sendJsonResponse(array('error' => '无效的API路径'), 404);
                }
            } else {
                sendJsonResponse(array('error' => '缺少API路径'), 404);
            }
            break;
        case 'GET':
            if (isset($parts[1])) {
                if ($parts[1] == 'devices') {
                    // 获取用户设备列表
                    $user = checkApiPermission(true);
                    $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                    $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                    $devices = $userModel->getUserDevices($user['id'], $limit, $offset);
                    sendJsonResponse(array('devices' => $devices));
                } elseif ($parts[1] == 'info') {
                    // 获取当前用户信息
                    $user = checkApiPermission(true);
                    sendJsonResponse(array('user' => $user));
                } else {
                    sendJsonResponse(array('error' => '无效的API路径'), 404);
                }
            } else {
                sendJsonResponse(array('error' => '缺少API路径'), 404);
            }
            break;
        case 'PUT':
            if (isset($parts[1]) && $parts[1] == 'device') {
                // 更新设备昵称
                $user = checkApiPermission(true);
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['device_id']) || !isset($data['nickname'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $userModel->updateDeviceNickname($user['id'], $data['device_id'], $data['nickname']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 验证设备所有权
 */
function validateDeviceOwnership($deviceId) {
    global $currentUser, $userModel;
    
    if (!$currentUser) {
        return false;
    }
    
    return $userModel->isDeviceOwnedByUser($currentUser['id'], $deviceId);
}

/**
 * 检查设备访问权限
 */
function checkDevicePermission($deviceId) {
    global $currentUser, $userModel, $deviceModel;
    
    // 设备可以自己访问自己的资源，不需要验证用户
    $ip = $_SERVER['REMOTE_ADDR'];
    $device = $deviceModel->getDevice($deviceId);
    if ($device && $device['ip_address'] == $ip) {
        return true;
    }
    
    // 其他情况需要验证用户权限
    if (!$currentUser) {
        sendJsonResponse(array('error' => '未授权访问', 'code' => 401), 401);
        exit;
    }
    
    if (!$userModel->isDeviceOwnedByUser($currentUser['id'], $deviceId)) {
        sendJsonResponse(array('error' => '无权访问该设备', 'code' => 403), 403);
        exit;
    }
    
    return true;
}

/**
 * 处理设备分组相关API
 */
function handleDeviceGroupApi($method, $parts) {
    global $deviceGroupModel, $currentUser;
    
    $user = checkApiPermission(true);
    
    switch ($method) {
        case 'POST':
            if (!isset($parts[1])) {
                // 创建分组
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['name'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $name = $data['name'];
                $description = isset($data['description']) ? $data['description'] : '';
                $result = $deviceGroupModel->createGroup($user['id'], $name, $description);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'add_device') {
                // 添加设备到分组
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['group_id']) || !isset($data['device_id'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $deviceGroupModel->addDeviceToGroup($data['group_id'], $data['device_id']);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'remove_device') {
                // 从分组中移除设备
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['group_id']) || !isset($data['device_id'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $deviceGroupModel->removeDeviceFromGroup($data['group_id'], $data['device_id']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'GET':
            if (!isset($parts[1])) {
                // 获取分组列表
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $groups = $deviceGroupModel->getGroupsByUserId($user['id'], $limit, $offset);
                sendJsonResponse(array('groups' => $groups));
            } elseif (is_numeric($parts[1])) {
                // 获取分组详情
                $groupId = intval($parts[1]);
                $group = $deviceGroupModel->getGroupById($groupId, $user['id']);
                if ($group) {
                    sendJsonResponse(array('group' => $group));
                } else {
                    sendJsonResponse(array('error' => '分组不存在'), 404);
                }
            } elseif ($parts[1] == 'devices') {
                // 获取分组的设备列表
                if (!isset($parts[2]) || !is_numeric($parts[2])) {
                    sendJsonResponse(array('error' => '缺少分组ID'), 400);
                }
                $groupId = intval($parts[2]);
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $devices = $deviceGroupModel->getDevicesByGroupId($groupId, $limit, $offset);
                sendJsonResponse(array('devices' => $devices));
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'PUT':
            if (is_numeric($parts[1])) {
                // 更新分组信息
                $groupId = intval($parts[1]);
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['name'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $name = $data['name'];
                $description = isset($data['description']) ? $data['description'] : '';
                $result = $deviceGroupModel->updateGroup($groupId, $user['id'], $name, $description);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'DELETE':
            if (is_numeric($parts[1])) {
                // 删除分组
                $groupId = intval($parts[1]);
                $result = $deviceGroupModel->deleteGroup($groupId, $user['id']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 处理设备标签相关API
 */
function handleDeviceTagApi($method, $parts) {
    global $deviceTagModel, $currentUser;
    
    $user = checkApiPermission(true);
    
    switch ($method) {
        case 'POST':
            if (!isset($parts[1])) {
                // 创建标签
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['name'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $name = $data['name'];
                $color = isset($data['color']) ? $data['color'] : '#3498db';
                $result = $deviceTagModel->createTag($user['id'], $name, $color);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'add_device') {
                // 给设备添加标签
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['tag_id']) || !isset($data['device_id'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $deviceTagModel->addTagToDevice($data['tag_id'], $data['device_id']);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'remove_device') {
                // 从设备移除标签
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['tag_id']) || !isset($data['device_id'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $deviceTagModel->removeTagFromDevice($data['tag_id'], $data['device_id']);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'batch_add') {
                // 批量给设备添加标签
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['tag_id']) || !isset($data['device_ids'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $deviceTagModel->batchAddTagsToDevices($data['tag_id'], $data['device_ids']);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'batch_remove') {
                // 批量从设备移除标签
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['tag_id']) || !isset($data['device_ids'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $deviceTagModel->batchRemoveTagsFromDevices($data['tag_id'], $data['device_ids']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'GET':
            if (!isset($parts[1])) {
                // 获取标签列表
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $tags = $deviceTagModel->getTagsByUserId($user['id'], $limit, $offset);
                sendJsonResponse(array('tags' => $tags));
            } elseif (is_numeric($parts[1])) {
                // 获取标签详情
                $tagId = intval($parts[1]);
                $tag = $deviceTagModel->getTagById($tagId, $user['id']);
                if ($tag) {
                    sendJsonResponse(array('tag' => $tag));
                } else {
                    sendJsonResponse(array('error' => '标签不存在'), 404);
                }
            } elseif ($parts[1] == 'devices') {
                // 获取标签的设备列表
                if (!isset($parts[2]) || !is_numeric($parts[2])) {
                    sendJsonResponse(array('error' => '缺少标签ID'), 400);
                }
                $tagId = intval($parts[2]);
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $devices = $deviceTagModel->getDevicesByTagId($tagId, $limit, $offset);
                sendJsonResponse(array('devices' => $devices));
            } elseif ($parts[1] == 'device_tags') {
                // 获取设备的标签列表
                if (!isset($parts[2])) {
                    sendJsonResponse(array('error' => '缺少设备ID'), 400);
                }
                $deviceId = $parts[2];
                $tags = $deviceTagModel->getTagsByDeviceId($deviceId);
                sendJsonResponse(array('tags' => $tags));
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'PUT':
            if (is_numeric($parts[1])) {
                // 更新标签信息
                $tagId = intval($parts[1]);
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['name'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $name = $data['name'];
                $color = isset($data['color']) ? $data['color'] : '#3498db';
                $result = $deviceTagModel->updateTag($tagId, $user['id'], $name, $color);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'DELETE':
            if (is_numeric($parts[1])) {
                // 删除标签
                $tagId = intval($parts[1]);
                $result = $deviceTagModel->deleteTag($tagId, $user['id']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}

/**
 * 处理固件推送任务相关API
 */
function handleFirmwarePushTaskApi($method, $parts) {
    global $firmwarePushTaskModel, $currentUser;
    
    $user = checkApiPermission(true);
    
    switch ($method) {
        case 'POST':
            if (!isset($parts[1])) {
                // 创建推送任务
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['firmware_id']) || !isset($data['target_type'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $firmwareId = $data['firmware_id'];
                $targetType = $data['target_type'];
                $targetIds = isset($data['target_ids']) ? json_encode($data['target_ids']) : null;
                $result = $firmwarePushTaskModel->createPushTask($user['id'], $firmwareId, $targetType, $targetIds);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'add_log') {
                // 添加推送日志
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['task_id']) || !isset($data['device_id']) || !isset($data['status'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $errorMessage = isset($data['error_message']) ? $data['error_message'] : null;
                $result = $firmwarePushTaskModel->addPushLog($data['task_id'], $data['device_id'], $data['status'], $errorMessage);
                sendJsonResponse($result);
            } elseif ($parts[1] == 'batch_log') {
                // 批量添加推送日志
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['task_id']) || !isset($data['logs'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $result = $firmwarePushTaskModel->batchAddPushLogs($data['task_id'], $data['logs']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'GET':
            if (!isset($parts[1])) {
                // 获取推送任务列表
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $tasks = $firmwarePushTaskModel->getPushTasksByUserId($user['id'], $limit, $offset);
                sendJsonResponse(array('tasks' => $tasks));
            } elseif (is_numeric($parts[1])) {
                // 获取推送任务详情
                $taskId = intval($parts[1]);
                $task = $firmwarePushTaskModel->getPushTaskById($taskId, $user['id']);
                if ($task) {
                    sendJsonResponse(array('task' => $task));
                } else {
                    sendJsonResponse(array('error' => '推送任务不存在'), 404);
                }
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'PUT':
            if (is_numeric($parts[1])) {
                // 更新推送任务状态
                $taskId = intval($parts[1]);
                $data = json_decode(file_get_contents('php://input'), true);
                if (!$data || !isset($data['status'])) {
                    sendJsonResponse(array('error' => '无效的请求数据'), 400);
                }
                $status = $data['status'];
                $progress = isset($data['progress']) ? $data['progress'] : null;
                $successCount = isset($data['success_count']) ? $data['success_count'] : null;
                $failedCount = isset($data['failed_count']) ? $data['failed_count'] : null;
                $result = $firmwarePushTaskModel->updatePushTaskStatus($taskId, $status, $progress, $successCount, $failedCount);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        case 'DELETE':
            if (is_numeric($parts[1])) {
                // 删除推送任务
                $taskId = intval($parts[1]);
                $result = $firmwarePushTaskModel->deletePushTask($taskId, $user['id']);
                sendJsonResponse($result);
            } else {
                sendJsonResponse(array('error' => '无效的API路径'), 404);
            }
            break;
        default:
            sendJsonResponse(array('error' => '不支持的请求方法'), 405);
            break;
    }
}
?>