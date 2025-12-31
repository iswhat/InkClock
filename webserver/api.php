<?php
/**
 * API接口入口
 */
require_once 'config.php';
require_once 'models/Device.php';
require_once 'models/Message.php';
require_once 'models/FirmwareVersion.php';
require_once 'models/User.php';

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
    global $currentUser, $userModel;
    
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
?>