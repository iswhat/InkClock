<?php
/**
 * API网关控制器
 * 用于处理固件端和管理系统端之间的API请求转发和转换
 */

namespace InkClock\Controller;

class ApiGatewayController extends BaseController {
    /**
     * 请求缓存
     * @var array
     */
    private $requestCache = [];
    
    /**
     * 缓存有效期（秒）
     * @var int
     */
    private $cacheExpiry = 30;
    
    /**
     * API网关入口
     * 处理来自固件端的API请求
     */
    public function gateway($params) {
        $this->logAction('api_gateway_request');
        
        try {
            // 记录请求开始时间
            $startTime = microtime(true);
            
            // 获取请求信息
            $path = $_SERVER['REQUEST_URI'] ?? '';
            $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
            $headers = getallheaders();
            $body = file_get_contents('php://input');
            
            // 解析路径，提取实际的API路径
            $apiPath = $this->parseApiPath($path);
            
            // 生成请求缓存键
            $cacheKey = $this->generateCacheKey($apiPath, $method, $body);
            
            // 检查缓存（只缓存GET请求）
            if ($method === 'GET') {
                $cachedResponse = $this->getCachedResponse($cacheKey);
                if ($cachedResponse) {
                    $this->logger->info('API Gateway Cache Hit', [
                        'api_path' => $apiPath,
                        'method' => $method,
                        'cache_key' => $cacheKey
                    ]);
                    
                    // 记录响应信息
                    $processingTime = microtime(true) - $startTime;
                    $this->logger->info('API Gateway Response', [
                        'api_path' => $apiPath,
                        'method' => $method,
                        'status' => $cachedResponse['status'],
                        'processing_time' => round($processingTime, 4),
                        'response_size' => strlen($cachedResponse['body']),
                        'from_cache' => true
                    ]);
                    
                    // 返回缓存的响应
                    $this->sendResponse($cachedResponse);
                }
            }
            
            // 记录请求信息（敏感信息过滤）
            $logHeaders = $this->filterSensitiveHeaders($headers);
            $logBody = $this->filterSensitiveData($body);
            
            $this->logger->info('API Gateway Request', [
                'path' => $path,
                'api_path' => $apiPath,
                'method' => $method,
                'headers' => $logHeaders,
                'body' => $logBody,
                'cache_key' => $cacheKey
            ]);
            
            // 安全检查
            if (!$this->validateRequest($apiPath, $method, $headers)) {
                return $this->sendErrorResponse(403, '请求验证失败');
            }
            
            // 速率限制检查
            if (!$this->checkRateLimit($apiPath)) {
                return $this->sendErrorResponse(429, '请求过于频繁，请稍后再试');
            }
            
            // 处理API请求转发
            $response = $this->forwardRequest($apiPath, $method, $headers, $body);
            
            // 缓存响应（只缓存GET请求且状态码为200的响应）
            if ($method === 'GET' && $response['status'] === 200) {
                $this->cacheResponse($cacheKey, $response);
            }
            
            // 记录响应信息和处理时间
            $processingTime = microtime(true) - $startTime;
            $this->logger->info('API Gateway Response', [
                'api_path' => $apiPath,
                'method' => $method,
                'status' => $response['status'],
                'processing_time' => round($processingTime, 4),
                'response_size' => strlen($response['body']),
                'from_cache' => false
            ]);
            
            // 更新监控指标
            $this->updateMetrics($apiPath, $method, $response['status'], $processingTime);
            
            // 返回响应
            $this->sendResponse($response);
        } catch (\Exception $e) {
            // 记录未捕获的错误
            $this->logger->error('API Gateway Unhandled Error', [
                'error' => $e->getMessage(),
                'trace' => $e->getTraceAsString()
            ]);
            
            // 返回500错误
            $this->sendErrorResponse(500, 'API网关内部错误');
        }
    }
    
    /**
     * 生成请求缓存键
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @param string $body 请求体
     * @return string 缓存键
     */
    private function generateCacheKey($apiPath, $method, $body) {
        // 对于GET请求，使用路径和查询参数生成缓存键
        if ($method === 'GET') {
            $queryString = $_SERVER['QUERY_STRING'] ?? '';
            return md5($method . ':' . $apiPath . '?' . $queryString);
        }
        // 对于其他请求，使用路径、方法和请求体生成缓存键
        return md5($method . ':' . $apiPath . ':' . $body);
    }
    
    /**
     * 获取缓存的响应
     * @param string $cacheKey 缓存键
     * @return array|null 缓存的响应
     */
    private function getCachedResponse($cacheKey) {
        if (isset($this->requestCache[$cacheKey])) {
            $cachedItem = $this->requestCache[$cacheKey];
            // 检查缓存是否过期
            if (time() - $cachedItem['timestamp'] < $this->cacheExpiry) {
                return $cachedItem['response'];
            } else {
                // 移除过期的缓存
                unset($this->requestCache[$cacheKey]);
            }
        }
        return null;
    }
    
    /**
     * 缓存响应
     * @param string $cacheKey 缓存键
     * @param array $response 响应数据
     */
    private function cacheResponse($cacheKey, $response) {
        $this->requestCache[$cacheKey] = [
            'timestamp' => time(),
            'response' => $response
        ];
        
        // 限制缓存大小
        if (count($this->requestCache) > 100) {
            // 移除最早的缓存项
            reset($this->requestCache);
            unset($this->requestCache[key($this->requestCache)]);
        }
    }
    
    /**
     * 检查速率限制
     * @param string $apiPath API路径
     * @return bool 是否通过速率限制检查
     */
    private function checkRateLimit($apiPath) {
        // 简单的速率限制实现
        // 实际应用中应该使用Redis或其他存储来存储速率限制数据
        static $rateLimits = [];
        
        $clientIp = $_SERVER['REMOTE_ADDR'] ?? '';
        $key = $clientIp . ':' . $apiPath;
        $now = time();
        $window = 60; // 60秒窗口
        $limit = 100; // 每个窗口最多100个请求
        
        if (!isset($rateLimits[$key])) {
            $rateLimits[$key] = [];
        }
        
        // 清理过期的请求记录
        $rateLimits[$key] = array_filter($rateLimits[$key], function($timestamp) use ($now, $window) {
            return $now - $timestamp < $window;
        });
        
        // 检查是否超过限制
        if (count($rateLimits[$key]) >= $limit) {
            $this->logger->warning('API Gateway Rate Limit Exceeded', [
                'api_path' => $apiPath,
                'client_ip' => $clientIp,
                'current_requests' => count($rateLimits[$key]),
                'limit' => $limit
            ]);
            return false;
        }
        
        // 添加当前请求
        $rateLimits[$key][] = $now;
        return true;
    }
    
    /**
     * 更新监控指标
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @param int $statusCode 状态码
     * @param float $processingTime 处理时间
     */
    private function updateMetrics($apiPath, $method, $statusCode, $processingTime) {
        // 这里可以实现监控指标的更新
        // 例如，使用Prometheus或其他监控系统
        $this->logger->debug('API Gateway Metrics', [
            'api_path' => $apiPath,
            'method' => $method,
            'status_code' => $statusCode,
            'processing_time' => round($processingTime, 4)
        ]);
    }
    
    /**
     * 过滤敏感请求头
     * @param array $headers 请求头
     * @return array 过滤后的请求头
     */
    private function filterSensitiveHeaders($headers) {
        $sensitiveHeaders = ['Authorization', 'X-API-Key', 'Cookie'];
        $filteredHeaders = [];
        
        foreach ($headers as $name => $value) {
            if (in_array($name, $sensitiveHeaders)) {
                $filteredHeaders[$name] = '***';
            } else {
                $filteredHeaders[$name] = $value;
            }
        }
        
        return $filteredHeaders;
    }
    
    /**
     * 过滤敏感请求数据
     * @param string $body 请求体
     * @return string 过滤后的请求体
     */
    private function filterSensitiveData($body) {
        if (empty($body)) {
            return $body;
        }
        
        // 尝试解析JSON
        $data = json_decode($body, true);
        if (json_last_error() === JSON_ERROR_NONE) {
            $sensitiveFields = ['password', 'token', 'api_key', 'auth'];
            foreach ($sensitiveFields as $field) {
                if (isset($data[$field])) {
                    $data[$field] = '***';
                }
            }
            return json_encode($data);
        }
        
        return $body;
    }
    
    /**
     * 验证请求的安全性
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @param array $headers 请求头
     * @return bool 是否验证通过
     */
    private function validateRequest($apiPath, $method, $headers) {
        // 1. 检查请求来源
        $clientIp = $_SERVER['REMOTE_ADDR'] ?? '';
        
        // 2. 检查API密钥（如果需要）
        if (strpos($apiPath, '/auth') === 0 || strpos($apiPath, '/admin') === 0 || strpos($apiPath, '/system') === 0) {
            $apiKey = $headers['X-API-Key'] ?? $headers['x-api-key'] ?? '';
            if (empty($apiKey)) {
                $this->logger->warning('API Gateway Missing API Key', [
                    'api_path' => $apiPath,
                    'client_ip' => $clientIp
                ]);
                return false;
            }
            
            // 验证API密钥
            try {
                $validApiKey = $this->container->get('config')['api_key'] ?? '';
                if (empty($validApiKey)) {
                    $this->logger->warning('API Gateway Missing Valid API Key in Config');
                    return false;
                }
                
                if ($apiKey !== $validApiKey) {
                    $this->logger->warning('API Gateway Invalid API Key', [
                        'api_path' => $apiPath,
                        'client_ip' => $clientIp
                    ]);
                    return false;
                }
            } catch (\Exception $e) {
                $this->logger->error('API Gateway API Key Validation Error', [
                    'error' => $e->getMessage()
                ]);
                return false;
            }
        }
        
        // 3. 检查请求方法
        $allowedMethods = ['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS'];
        if (!in_array($method, $allowedMethods)) {
            $this->logger->warning('API Gateway Invalid HTTP Method', [
                'api_path' => $apiPath,
                'method' => $method,
                'client_ip' => $clientIp
            ]);
            return false;
        }
        
        // 4. 检查请求大小
        $contentLength = $_SERVER['CONTENT_LENGTH'] ?? 0;
        $maxRequestSize = 10 * 1024 * 1024; // 10MB
        if ($contentLength > $maxRequestSize) {
            $this->logger->warning('API Gateway Request Too Large', [
                'api_path' => $apiPath,
                'content_length' => $contentLength,
                'max_size' => $maxRequestSize,
                'client_ip' => $clientIp
            ]);
            return false;
        }
        
        // 5. 检查请求头
        if (isset($headers['Content-Type'])) {
            $contentType = $headers['Content-Type'];
            $allowedContentTypes = [
                'application/json',
                'application/x-www-form-urlencoded',
                'multipart/form-data'
            ];
            
            $found = false;
            foreach ($allowedContentTypes as $allowedType) {
                if (strpos($contentType, $allowedType) === 0) {
                    $found = true;
                    break;
                }
            }
            
            if (!$found) {
                $this->logger->warning('API Gateway Invalid Content-Type', [
                    'api_path' => $apiPath,
                    'content_type' => $contentType,
                    'client_ip' => $clientIp
                ]);
                return false;
            }
        }
        
        // 6. 检查用户代理（可选）
        $userAgent = $_SERVER['HTTP_USER_AGENT'] ?? '';
        if (empty($userAgent) && strpos($apiPath, '/device') !== 0) {
            // 对于非设备请求，要求用户代理
            $this->logger->warning('API Gateway Missing User Agent', [
                'api_path' => $apiPath,
                'client_ip' => $clientIp
            ]);
            return false;
        }
        
        return true;
    }
    
    /**
     * 发送错误响应
     * @param int $statusCode 状态码
     * @param string $message 错误信息
     */
    private function sendErrorResponse($statusCode, $message) {
        $response = [
            'status' => $statusCode,
            'headers' => ['Content-Type' => 'application/json'],
            'body' => json_encode(['success' => false, 'message' => $message])
        ];
        
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
        try {
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
        } catch (\Exception $e) {
            // 记录转发错误
            $this->logger->error('API Gateway Forward Error', [
                'error' => $e->getMessage(),
                'api_path' => $apiPath,
                'method' => $method
            ]);
            
            return [
                'status' => 502,
                'headers' => ['Content-Type' => 'application/json'],
                'body' => json_encode(['success' => false, 'message' => '请求转发失败: ' . $e->getMessage()])
            ];
        }
    }
    
    /**
     * 获取转发目标
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @return array|null 转发目标信息
     */
    private function getForwardTarget($apiPath, $method) {
        // 定义API路径映射（按优先级排序）
        static $pathMap = null;
        
        // 延迟初始化路径映射
        if ($pathMap === null) {
            $pathMap = [
                // 健康检查（最高优先级）
                '/health' => [
                    'controller' => 'HealthController',
                    'method' => 'check',
                    'params' => [],
                    'methods' => ['GET']
                ],
                
                // 设备相关
                '/device/status/{device_id}' => [
                    'controller' => 'DeviceController',
                    'method' => 'getDevice',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/device/refresh' => [
                    'controller' => 'DeviceController',
                    'method' => 'refreshDevice',
                    'params' => [],
                    'methods' => ['GET', 'POST']
                ],
                '/device/register' => [
                    'controller' => 'DeviceController',
                    'method' => 'registerDevice',
                    'params' => [],
                    'methods' => ['POST']
                ],
                '/device/list' => [
                    'controller' => 'DeviceController',
                    'method' => 'getDevices',
                    'params' => [],
                    'methods' => ['GET']
                ],
                
                // 插件相关
                '/plugin/list' => [
                    'controller' => 'PluginController',
                    'method' => 'getPlugins',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/plugin/status/{device_id}' => [
                    'controller' => 'PluginController',
                    'method' => 'getDevicePlugins',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/plugin/toggle/{plugin_id}' => [
                    'controller' => 'PluginController',
                    'method' => 'togglePlugin',
                    'params' => [],
                    'methods' => ['POST']
                ],
                '/plugin/device/list/{device_id}' => [
                    'controller' => 'PluginController',
                    'method' => 'getDevicePlugins',
                    'params' => [],
                    'methods' => ['GET']
                ],
                
                // 消息相关
                '/message/push' => [
                    'controller' => 'MessageController',
                    'method' => 'sendMessage',
                    'params' => [],
                    'methods' => ['POST']
                ],
                '/message/list/{device_id}' => [
                    'controller' => 'MessageController',
                    'method' => 'getMessages',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/message/unread/{device_id}' => [
                    'controller' => 'MessageController',
                    'method' => 'getUnreadMessages',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/message/sync/{device_id}' => [
                    'controller' => 'MessageController',
                    'method' => 'syncMessages',
                    'params' => [],
                    'methods' => ['POST']
                ],
                '/message/pending/{device_id}' => [
                    'controller' => 'MessageController',
                    'method' => 'getPendingMessages',
                    'params' => [],
                    'methods' => ['GET']
                ],
                
                // 固件相关
                '/firmware/check/{model}' => [
                    'controller' => 'FirmwareController',
                    'method' => 'getActiveVersion',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/firmware/list' => [
                    'controller' => 'FirmwareController',
                    'method' => 'getAllVersions',
                    'params' => [],
                    'methods' => ['GET']
                ],
                
                // 系统相关
                '/system/status' => [
                    'controller' => 'StatusController',
                    'method' => 'getStatus',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/system/refresh' => [
                    'controller' => 'SystemController',
                    'method' => 'refreshSystem',
                    'params' => [],
                    'methods' => ['GET', 'POST']
                ],
                '/system/info' => [
                    'controller' => 'SystemController',
                    'method' => 'getSystemInfo',
                    'params' => [],
                    'methods' => ['GET']
                ],
                
                // 根路径
                '/' => [
                    'controller' => 'ApiGatewayController',
                    'method' => 'getGatewayInfo',
                    'params' => [],
                    'methods' => ['GET']
                ]
            ];
        }
        
        // 1. 首先尝试精确匹配（不包含参数的路径）
        foreach ($pathMap as $path => $target) {
            // 跳过带参数的路径
            if (strpos($path, '{') !== false) {
                continue;
            }
            
            // 检查路径是否匹配
            if ($apiPath === $path) {
                // 检查HTTP方法是否允许
                if (empty($target['methods']) || in_array($method, $target['methods'])) {
                    return $target;
                } else {
                    $this->logger->warning('API Gateway Method Not Allowed', [
                        'api_path' => $apiPath,
                        'method' => $method,
                        'allowed_methods' => $target['methods']
                    ]);
                    return [
                        'controller' => 'ApiGatewayController',
                        'method' => 'methodNotAllowed',
                        'params' => ['allowedMethods' => $target['methods']]
                    ];
                }
            }
        }
        
        // 2. 尝试带参数的路径匹配
        foreach ($pathMap as $path => $target) {
            // 跳过不带参数的路径
            if (strpos($path, '{') === false) {
                continue;
            }
            
            // 检查HTTP方法是否允许
            if (!empty($target['methods']) && !in_array($method, $target['methods'])) {
                continue;
            }
            
            // 生成正则表达式模式
            $pattern = preg_replace('/\{([^}]+)\}/', '([^/]+)', $path);
            $pattern = '/^' . str_replace('/', '\\/', $pattern) . '$/';
            
            // 尝试匹配路径
            if (preg_match($pattern, $apiPath, $matches)) {
                // 提取参数名
                preg_match_all('/\{([^}]+)\}/', $path, $paramNames);
                $params = [];
                
                // 绑定参数值
                for ($i = 1; $i < count($matches); $i++) {
                    if (isset($paramNames[1][$i-1])) {
                        $paramName = $paramNames[1][$i-1];
                        $params[$paramName] = $matches[$i];
                    }
                }
                
                // 添加参数到目标配置
                $target['params'] = $params;
                return $target;
            }
        }
        
        // 3. 尝试旧格式的路径匹配（向后兼容）
        $legacyTarget = $this->getLegacyForwardTarget($apiPath, $method);
        if ($legacyTarget) {
            return $legacyTarget;
        }
        
        // 4. 尝试查询参数方式的路径匹配（向后兼容）
        $queryParamTarget = $this->getQueryParamTarget($apiPath, $method);
        if ($queryParamTarget) {
            return $queryParamTarget;
        }
        
        // 记录未找到的路径
        $this->logger->warning('API Gateway Path Not Found', [
            'path' => $apiPath,
            'method' => $method,
            'client_ip' => $_SERVER['REMOTE_ADDR'] ?? ''
        ]);
        
        return null;
    }
    
    /**
     * 获取查询参数方式的转发目标（向后兼容）
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @return array|null 转发目标信息
     */
    private function getQueryParamTarget($apiPath, $method) {
        // 解析查询参数
        $queryParams = [];
        if (isset($_GET)) {
            $queryParams = $_GET;
        }
        
        // 检查常见的查询参数路径
        switch ($apiPath) {
            case '/device/status':
                if (isset($queryParams['device_id']) && $method === 'GET') {
                    return [
                        'controller' => 'DeviceController',
                        'method' => 'getDevice',
                        'params' => ['device_id' => $queryParams['device_id']],
                        'methods' => ['GET']
                    ];
                }
                break;
            case '/plugin/status':
                if (isset($queryParams['device_id']) && $method === 'GET') {
                    return [
                        'controller' => 'PluginController',
                        'method' => 'getDevicePlugins',
                        'params' => ['device_id' => $queryParams['device_id']],
                        'methods' => ['GET']
                    ];
                }
                break;
            case '/plugin/device/list':
                if (isset($queryParams['device_id']) && $method === 'GET') {
                    return [
                        'controller' => 'PluginController',
                        'method' => 'getDevicePlugins',
                        'params' => ['device_id' => $queryParams['device_id']],
                        'methods' => ['GET']
                    ];
                }
                break;
            case '/message/list':
                if (isset($queryParams['device_id']) && $method === 'GET') {
                    return [
                        'controller' => 'MessageController',
                        'method' => 'getMessages',
                        'params' => ['device_id' => $queryParams['device_id']],
                        'methods' => ['GET']
                    ];
                }
                break;
            case '/message/unread':
                if (isset($queryParams['device_id']) && $method === 'GET') {
                    return [
                        'controller' => 'MessageController',
                        'method' => 'getUnreadMessages',
                        'params' => ['device_id' => $queryParams['device_id']],
                        'methods' => ['GET']
                    ];
                }
                break;
            case '/message/sync':
                if (isset($queryParams['device_id']) && $method === 'POST') {
                    return [
                        'controller' => 'MessageController',
                        'method' => 'syncMessages',
                        'params' => ['device_id' => $queryParams['device_id']],
                        'methods' => ['POST']
                    ];
                }
                break;
            case '/message/pending':
                if (isset($queryParams['device_id']) && $method === 'GET') {
                    return [
                        'controller' => 'MessageController',
                        'method' => 'getPendingMessages',
                        'params' => ['device_id' => $queryParams['device_id']],
                        'methods' => ['GET']
                    ];
                }
                break;
            case '/firmware/check':
                if (isset($queryParams['model']) && $method === 'GET') {
                    return [
                        'controller' => 'FirmwareController',
                        'method' => 'getActiveVersion',
                        'params' => ['model' => $queryParams['model']],
                        'methods' => ['GET']
                    ];
                }
                break;
        }
        
        return null;
    }
    
    /**
     * 获取API网关信息
     * @param array $params 参数
     * @return array 响应数据
     */
    public function getGatewayInfo($params) {
        return [
            'success' => true,
            'message' => 'InkClock API Gateway',
            'version' => '1.0.0',
            'endpoints' => [
                'health' => '/health',
                'device' => [
                    'status' => '/device/status/{device_id}',
                    'refresh' => '/device/refresh',
                    'register' => '/device/register',
                    'list' => '/device/list'
                ],
                'plugin' => [
                    'list' => '/plugin/list',
                    'status' => '/plugin/status/{device_id}',
                    'toggle' => '/plugin/toggle/{plugin_id}',
                    'device_list' => '/plugin/device/list/{device_id}'
                ],
                'message' => [
                    'push' => '/message/push',
                    'list' => '/message/list/{device_id}',
                    'unread' => '/message/unread/{device_id}',
                    'sync' => '/message/sync/{device_id}',
                    'pending' => '/message/pending/{device_id}'
                ],
                'firmware' => [
                    'check' => '/firmware/check/{model}',
                    'list' => '/firmware/list'
                ],
                'system' => [
                    'status' => '/system/status',
                    'refresh' => '/system/refresh',
                    'info' => '/system/info'
                ]
            ],
            'timestamp' => time()
        ];
    }
    
    /**
     * 获取旧格式的转发目标（向后兼容）
     * @param string $apiPath API路径
     * @param string $method 请求方法
     * @return array|null 转发目标信息
     */
    private function getLegacyForwardTarget($apiPath, $method) {
        // 旧格式路径映射
        $legacyMap = [
            '/device/status' => [
                'controller' => 'DeviceController',
                'method' => 'getDevice',
                'params' => ['id' => '{device_id}']
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
            '/plugin/device/list' => [
                'controller' => 'PluginController',
                'method' => 'getDevicePlugins',
                'params' => ['deviceId' => '{device_id}']
            ],
            '/message/list' => [
                'controller' => 'MessageController',
                'method' => 'getMessages',
                'params' => ['deviceId' => '{device_id}']
            ],
            '/message/unread' => [
                'controller' => 'MessageController',
                'method' => 'getUnreadMessages',
                'params' => ['deviceId' => '{device_id}']
            ],
            '/message/sync' => [
                'controller' => 'MessageController',
                'method' => 'syncMessages',
                'params' => ['deviceId' => '{device_id}']
            ],
            '/message/pending' => [
                'controller' => 'MessageController',
                'method' => 'getPendingMessages',
                'params' => ['deviceId' => '{device_id}']
            ],
            '/firmware/check' => [
                'controller' => 'FirmwareController',
                'method' => 'getActiveVersion',
                'params' => ['model' => '{model}']
            ]
        ];
        
        // 查找匹配的路径
        foreach ($legacyMap as $path => $target) {
            if ($apiPath === $path) {
                return $target;
            }
        }
        
        return null;
    }
    
    /**
     * 方法不允许响应
     * @param array $params 参数
     * @return array 响应数据
     */
    public function methodNotAllowed($params) {
        $allowedMethods = $params['allowedMethods'] ?? [];
        return [
            'status' => 405,
            'headers' => [
                'Content-Type' => 'application/json',
                'Allow' => implode(', ', $allowedMethods)
            ],
            'body' => json_encode([
                'success' => false,
                'message' => '方法不允许',
                'allowed_methods' => $allowedMethods
            ])
        ];
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
                // 尝试解析表单数据
                parse_str($body, $data);
            }
        }
        
        // 合并路径参数
        if (isset($target['params'])) {
            foreach ($target['params'] as $key => $value) {
                // 替换参数占位符
                if (strpos($value, '{') === 0 && strpos($value, '}') === strlen($value) - 1) {
                    $paramName = substr($value, 1, -1);
                    if (isset($data[$paramName])) {
                        $target['params'][$key] = $data[$paramName];
                    }
                }
            }
        }
        
        // 根据目标转换数据格式
        switch ($target['controller']) {
            case 'DeviceController':
                if ($target['method'] === 'registerDevice') {
                    return [
                        'device_id' => $data['device_id'] ?? '',
                        'model' => $data['model'] ?? '',
                        'firmware_version' => $data['version'] ?? $data['firmware_version'] ?? '',
                        'mac_address' => $data['mac_address'] ?? '',
                        'extra_info' => $data['extra'] ?? $data['extra_info'] ?? []
                    ];
                }
                break;
                
            case 'MessageController':
                switch ($target['method']) {
                    case 'sendMessage':
                        return [
                            'device_id' => $data['device_id'] ?? '',
                            'content' => $data['content'] ?? '',
                            'type' => $data['type'] ?? 'text'
                        ];
                    case 'syncMessages':
                        return [
                            'deviceId' => $target['params']['deviceId'] ?? $data['device_id'] ?? '',
                            'messages' => $data['messages'] ?? [],
                            'sync_time' => $data['sync_time'] ?? time()
                        ];
                    default:
                        return $data;
                }
                break;
                
            case 'PluginController':
                if ($target['method'] === 'togglePlugin') {
                    return [
                        'id' => $target['params']['id'] ?? $data['plugin_id'] ?? '',
                        'status' => $data['status'] ?? 'disabled'
                    ];
                }
                break;
                
            case 'FirmwareController':
                if ($target['method'] === 'getActiveVersion') {
                    return [
                        'model' => $target['params']['model'] ?? $data['model'] ?? ''
                    ];
                }
                break;
        }
        
        // 合并路径参数和请求体数据
        return array_merge($target['params'] ?? [], $data);
    }
    
    /**
     * 调用控制器方法
     * @param array $target 转发目标
     * @param array $data 请求数据
     * @return array 响应数据
     */
    private function callControllerMethod($target, $data) {
        try {
            // 记录开始时间
            $startTime = microtime(true);
            
            // 实例化控制器
            $controllerClass = 'InkClock\\Controller\\' . $target['controller'];
            
            // 检查控制器类是否存在
            if (!class_exists($controllerClass)) {
                throw new \Exception('控制器类不存在: ' . $controllerClass);
            }
            
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
            
            // 检查方法是否存在
            $method = $target['method'];
            if (!method_exists($controller, $method)) {
                throw new \Exception('方法不存在: ' . $method);
            }
            
            // 调用方法
            $result = null;
            
            // 检查方法参数
            $reflection = new \ReflectionMethod($controllerClass, $method);
            $params = $reflection->getParameters();
            
            if (empty($params)) {
                // 无参数方法
                $result = $controller->$method();
            } else if (count($params) === 1) {
                // 单参数方法
                $result = $controller->$method($data);
            } else {
                // 多参数方法，尝试根据参数名匹配
                $args = [];
                foreach ($params as $param) {
                    $paramName = $param->getName();
                    if (isset($data[$paramName])) {
                        $args[] = $data[$paramName];
                    } else if (isset($target['params'][$paramName])) {
                        $args[] = $target['params'][$paramName];
                    } else if ($param->isOptional()) {
                        $args[] = $param->getDefaultValue();
                    } else {
                        $args[] = null;
                    }
                }
                $result = call_user_func_array([$controller, $method], $args);
            }
            
            // 确保返回值是数组
            if (!is_array($result)) {
                $result = [
                    'success' => true,
                    'data' => $result
                ];
            }
            
            // 添加响应元数据
            $result['meta'] = [
                'timestamp' => time(),
                'processing_time' => round(microtime(true) - $startTime, 4),
                'controller' => $target['controller'],
                'method' => $target['method']
            ];
            
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
                'trace' => $e->getTraceAsString(),
                'controller' => $target['controller'],
                'method' => $target['method']
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