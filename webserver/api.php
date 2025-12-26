<?php
/**
 * API接口入口
 */
require_once 'config.php';
require_once 'models/Device.php';
require_once 'models/Message.php';

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

// 路由处理
switch ($parts[0]) {
    case 'device':
        handleDeviceApi($method, $parts);
        break;
    case 'message':
        handleMessageApi($method, $parts);
        break;
    case 'status':
        handleStatusApi();
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
    global $deviceModel;
    
    switch ($method) {
        case 'POST':
            // 注册或更新设备
            $data = json_decode(file_get_contents('php://input'), true);
            if (!$data) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            $result = $deviceModel->registerDevice($data);
            sendJsonResponse($result);
            break;
        case 'GET':
            if (isset($parts[1])) {
                // 获取单个设备信息
                $deviceId = $parts[1];
                $device = $deviceModel->getDevice($deviceId);
                if ($device) {
                    sendJsonResponse(array('device' => $device));
                } else {
                    sendJsonResponse(array('error' => '设备不存在'), 404);
                }
            } else {
                // 获取设备列表
                $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 50;
                $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
                $devices = $deviceModel->getDevices($limit, $offset);
                sendJsonResponse(array('devices' => $devices));
            }
            break;
        case 'DELETE':
            if (isset($parts[1])) {
                // 删除设备
                $deviceId = $parts[1];
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
    global $messageModel;
    
    switch ($method) {
        case 'POST':
            // 发送消息
            $data = json_decode(file_get_contents('php://input'), true);
            if (!$data || !isset($data['device_id']) || !isset($data['content'])) {
                sendJsonResponse(array('error' => '无效的请求数据'), 400);
            }
            $result = $messageModel->sendMessage($data);
            sendJsonResponse($result);
            break;
        case 'GET':
            if (isset($parts[1])) {
                $deviceId = $parts[1];
                
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
?>