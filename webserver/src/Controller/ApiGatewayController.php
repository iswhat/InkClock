<?php
/**
 * API网关控制器
 * 用于处理固件端和管理系统端之间的API请求转发和转换
 */

namespace InkClock\Controller;

class ApiGatewayController extends BaseController {
    /**
     * API网关入口
     * 处理来自固件端的API请求
     */
    public function gateway($params) {
        $this->logAction('api_gateway_request');
        
        // 获取请求信息
        $path = $_SERVER['REQUEST_URI'] ?? '';
        $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
        $headers = getallheaders();
        $body = file_get_contents('php://input');
        
        // 解析路径，提取实际的API路径
        $apiPath = $this->parseApiPath($path);
        
        // 记录请求信息
        $this->logger->info('API Gateway Request', [
            'path' => $path,
            'api_path' => $apiPath,
            'method' => $method,
            'headers' => $headers,
            'body' => $body
        ]);
        
        // 处理API请求转发
        $response = $this->forwardRequest($apiPath, $method, $headers, $body);
        
        // 返回响应
        $this->sendResponse($response);
    }
    
    /**
     * 解析API路径
     * @param string $path 原始请求路径
     * @return string 解析后的API路径
     */
    private function parseApiPath($path) {
        // 移除/api/gateway前缀
        $apiPath = preg_replace('#^/api/gateway#', '', $path);
        
        // 移除查询参数
        $apiPath = preg_replace('#\?.*$#', '', $apiPath);
        
        // 确保路径以/开头
        if (empty($apiPath)) {
            $apiPath = '/';
        } elseif (substr($apiPath, 0, 1) !== '/') {
            $apiPath = '/' . $apiPath;
        }
        
        return $apiPath;
    }
    
    /**
     * 转发API请求
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @param array $headers 请求头
     * @param string $body 请求体
     * @return array 响应数据
     */
    private function forwardRequest($apiPath, $method, $headers, $body) {
        // 根据API路径和方法确定转发目标
        $target = $this->getForwardTarget($apiPath, $method);
        
        if (!$target) {
            return [
                'status' => 404,
                'headers' => ['Content-Type' => 'application/json'],
                'body' => json_encode(['success' => false, 'message' => 'API路径不存在'])
            ];
        }
        
        // 转换请求数据
        $convertedBody = $this->convertRequestBody($body, $target);
        
        // 调用目标控制器方法
        return $this->callControllerMethod($target, $convertedBody);
    }
    
    /**
     * 获取转发目标
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @return array|null 转发目标信息
     */
    private function getForwardTarget($apiPath, $method) {
        // 定义API路径映射
        $pathMap = [
            // 设备相关
            '/device/status' => [
                'controller' => 'DeviceController',
                'method' => 'getDevice',
                'params' => ['id' => '{device_id}']
            ],
            '/device/refresh' => [
                'controller' => 'DeviceController',
                'method' => 'refreshDevice',
                'params' => []
            ],
            
            // 插件相关
            '/plugin/list' => [
                'controller' => 'PluginController',
                'method' => 'getPlugins',
                'params' => []
            ],
            '/plugin/status' => [
                'controller' => 'PluginController',
                'method' => 'getDevicePlugins',
                'params' => ['deviceId' => '{device_id}']
            ],
            '/plugin/toggle' => [
                'controller' => 'PluginController',
                'method' => 'togglePlugin',
                'params' => ['id' => '{plugin_id}']
            ],
            
            // 消息相关
            '/message/push' => [
                'controller' => 'MessageController',
                'method' => 'sendMessage',
                'params' => []
            ],
            '/message/list' => [
                'controller' => 'MessageController',
                'method' => 'getMessages',
                'params' => ['deviceId' => '{device_id}']
            ],
            
            // 系统相关
            '/system/status' => [
                'controller' => 'StatusController',
                'method' => 'getStatus',
                'params' => []
            ],
            '/system/refresh' => [
                'controller' => 'SystemController',
                'method' => 'refreshSystem',
                'params' => []
            ],
        ];
        
        // 查找匹配的路径
        foreach ($pathMap as $path => $target) {
            if ($apiPath === $path) {
                return $target;
            }
        }
        
        // 记录未找到的路径
        $this->logger->warning('API Gateway Path Not Found', [
            'path' => $apiPath,
            'method' => $method
        ]);
        
        return null;
    }
    
    /**
     * 转换请求体
     * @param string $body 原始请求体
     * @param array $target 转发目标
     * @return array 转换后的请求数据
     */
    private function convertRequestBody($body, $target) {
        // 解析JSON请求体
        $data = [];
        if (!empty($body)) {
            $data = json_decode($body, true);
            if (json_last_error() !== JSON_ERROR_NONE) {
                $data = [];
            }
        }
        
        // 根据目标转换数据格式
        switch ($target['controller']) {
            case 'MessageController':
                if ($target['method'] === 'sendMessage') {
                    return [
                        'device_id' => $data['device_id'] ?? '',
                        'content' => $data['content'] ?? '',
                        'type' => $data['type'] ?? 'text'
                    ];
                }
                break;
                
            case 'PluginController':
                if ($target['method'] === 'togglePlugin') {
                    return [
                        'id' => $data['plugin_id'] ?? '',
                        'status' => $data['status'] ?? 'disabled'
                    ];
                }
                break;
        }
        
        return $data;
    }
    
    /**
     * 调用控制器方法
     * @param array $target 转发目标
     * @param array $data 请求数据
     * @return array 响应数据
     */
    private function callControllerMethod($target, $data) {
        try {
            // 实例化控制器
            $controllerClass = 'InkClock\\Controller\\' . $target['controller'];
            
            // 尝试不同的构造函数签名
            try {
                // 尝试带容器参数的构造函数（推荐方式）
                $controller = new $controllerClass($this->container);
            } catch (\ArgumentCountError $e) {
                try {
                    // 尝试无参数的构造函数
                    $controller = new $controllerClass();
                } catch (\Exception $e2) {
                    // 记录错误
                    $this->logger->error('Controller instantiation error', [
                        'error' => $e2->getMessage(),
                        'controller' => $controllerClass
                    ]);
                    throw new \Exception('无法实例化控制器: ' . $controllerClass);
                }
            }
            
            // 调用方法
            // 对于需要请求体参数的方法，传递转换后的数据
            // 对于需要路径参数的方法，传递参数映射
            $result = call_user_func_array([$controller, $target['method']], [$data]);
            
            // 转换响应格式
            return [
                'status' => 200,
                'headers' => ['Content-Type' => 'application/json'],
                'body' => json_encode($result)
            ];
        } catch (\Exception $e) {
            // 记录错误
            $this->logger->error('API Gateway Error', [
                'error' => $e->getMessage(),
                'trace' => $e->getTraceAsString()
            ]);
            
            return [
                'status' => 500,
                'headers' => ['Content-Type' => 'application/json'],
                'body' => json_encode(['success' => false, 'message' => 'API网关错误: ' . $e->getMessage()])
            ];
        }
    }
    
    /**
     * 发送响应
     * @param array $response 响应数据
     */
    private function sendResponse($response) {
        // 设置响应状态码
        http_response_code($response['status']);
        
        // 设置响应头
        foreach ($response['headers'] as $name => $value) {
            header($name . ': ' . $value);
        }
        
        // 发送响应体
        echo $response['body'];
        exit;
    }
    
    /**
     * 刷新设备
     */
    public function refreshDevice($params) {
        $this->logAction('device_refresh');
        
        return [
            'success' => true,
            'message' => '设备刷新成功',
            'timestamp' => time()
        ];
    }
    
    /**
     * 刷新系统
     */
    public function refreshSystem($params) {
        $this->logAction('system_refresh');
        
        return [
            'success' => true,
            'message' => '系统刷新成功',
            'timestamp' => time()
        ];
    }
}
?>