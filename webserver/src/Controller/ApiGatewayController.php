<?php
// API网关控制器 | API gateway controller
// 处理固件和管理系统之间的API请求转发 | Handles API request forwarding between firmware and management system

namespace InkClock\Controller;

class ApiGatewayController extends BaseController {
    // 请求缓存 | Request cache
    // @var array
    private $requestCache = [];
    
    // 缓存过期时间（秒） | Cache expiry time in seconds
    // @var int
    private $cacheExpiry = 30;
    
    // 构造函数 | Constructor
    public function __construct($container = null) {
        parent::__construct($container);
        
        // 初始化各系统 | Initialize various systems
        $this->initConfig();
        $this->initJwtConfig();
        $this->initLoadBalancingConfig();
        $this->initCircuitBreakerConfig();
        $this->initSessionConfig();
        $this->initTracingConfig();
        $this->initPluginSystem();
    }
    
    // API版本 | API version
    // @var string
    private $apiVersion = 'v1';
    
    // 分布式追踪上下文 | Distributed tracing context
    // @var array
    private $tracingContext = [];
    
    // 服务实例列表 | Service instances list
    // @var array
    private $serviceInstances = [];
    
    // 熔断状态 | Circuit breaker states
    // @var array
    private $circuitBreakerStates = [];
    
    // 会话数据 | Session data
    // @var array
    private $sessionData = [];
    
    // 插件列表 | Plugins list
    // @var array
    private $plugins = [];
    
    // 配置数据 | Configuration data
    // @var array
    private $config = [];
    
    // 性能监控数据 | Performance monitoring data
    // @var array
    private $performanceMetrics = [];
    
    // API网关入口点 | API gateway entry point
    // 处理来自固件的请求 | Handles requests from firmware
    public function gateway($params) {
        $this->logAction('api_gateway_request');
        
        try {
            // 定期清理过期会话 | Clean up expired sessions periodically
            if (rand(1, 100) <= 5) { // 5% 概率执行清理 | 5% chance to execute cleanup
                $this->cleanupExpiredSessions();
            }
            
            // 记录请求开始时间 | Record request start time
            $startTime = microtime(true);
            
            // 获取请求信息 | Get request info
            $path = $_SERVER['REQUEST_URI'] ?? '';
            $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
            $headers = getallheaders();

            // 限制请求体大小为1MB，防止DoS攻击
            $contentLength = (int)($_SERVER['CONTENT_LENGTH'] ?? 0);
            if ($contentLength > 1024 * 1024) {
                http_response_code(413);
                echo json_encode(['error' => 'Request too large', 'message' => '请求体大小超过1MB限制']);
                return;
            }

            $body = file_get_contents('php://input');

            // 验证请求体格式
            if ($body === false) {
                http_response_code(400);
                echo json_encode(['error' => 'Invalid request', 'message' => '无法读取请求体']);
                return;
            }
            
            // 初始化分布式追踪上下文 | Initialize distributed tracing context
            $this->initTracingContext($headers);
            
            // 解析路径获取实际API路径 | Parse path to get actual API path
            $apiPath = $this->parseApiPath($path);
            
            // 添加追踪上下文到日志 | Add tracing context to logs
            $logContext = $this->getTracingContext();
            
            // 生成缓存键 | Generate cache key
            $cacheKey = $this->generateCacheKey($apiPath, $method, $body);
            
            // 检查缓存（仅适用于GET请求） | Check cache (only for GET requests)
            if ($method === 'GET') {
                $cachedResponse = $this->getCachedResponse($cacheKey);
                if ($cachedResponse) {
                    $this->logger->info('API Gateway Cache Hit', [
                        'api_path' => $apiPath,
                        'method' => $method,
                        'cache_key' => $cacheKey
                    ]);
                    
                    // 记录响应信息 | Record response info
                    $processingTime = microtime(true) - $startTime;
                    $this->logger->info('API Gateway Response', [
                        'api_path' => $apiPath,
                        'api_version' => $this->apiVersion,
                        'method' => $method,
                        'status' => $cachedResponse['status'],
                        'processing_time' => round($processingTime, 4),
                        'response_size' => strlen($cachedResponse['body']),
                        'from_cache' => true
                    ]);
                    
                    // 返回缓存响应 | Return cached response
                    $this->sendResponse($cachedResponse);
                }
            }
            
            // 记录请求信息（过滤敏感数据） | Record request info (filter sensitive data)
            $logHeaders = $this->filterSensitiveHeaders($headers);
            $logBody = $this->filterSensitiveData($body);
            
            $this->logger->info('API Gateway Request', array_merge([
                'path' => $path,
                'api_path' => $apiPath,
                'api_version' => $this->apiVersion,
                'method' => $method,
                'headers' => $logHeaders,
                'body' => $logBody,
                'cache_key' => $cacheKey
            ], $logContext));
            
            // 安全检查 | Security check
            if (!$this->validateRequest($apiPath, $method, $headers)) {
                return $this->sendErrorResponse(403, 'Request validation failed');
            }
            
            // 速率限制检查 | Rate limit check
            if (!$this->checkRateLimit($apiPath)) {
                return $this->sendErrorResponse(429, 'Too many requests, please try again later');
            }
            
            // 转发请求 | Forward request
            $response = $this->forwardRequest($apiPath, $method, $headers, $body);
            
            // 缓存响应（仅适用于200状态的GET请求） | Cache response (only for GET requests with 200 status)
            if ($method === 'GET' && $response['status'] === 200) {
                $this->cacheResponse($cacheKey, $response);
            }
            
            // 记录响应信息和处理时间 | Record response info and processing time
            $processingTime = microtime(true) - $startTime;
            $this->logger->info('API Gateway Response', [
                'api_path' => $apiPath,
                'api_version' => $this->apiVersion,
                'method' => $method,
                'status' => $response['status'],
                'processing_time' => round($processingTime, 4),
                'response_size' => strlen($response['body']),
                'from_cache' => false
            ]);
            
            // 更新指标 | Update metrics
            $this->updateMetrics($apiPath, $method, $response['status'], $processingTime);
            
            // 返回响应 | Return response
            $this->sendResponse($response);
        } catch (\Exception $e) {
            // 记录未处理的错误 | Record unhandled errors
            $this->logger->error('API Gateway Unhandled Error', [
                'error' => $e->getMessage(),
                'trace' => $e->getTraceAsString()
            ]);
            
            // 返回500错误 | Return 500 error
            $this->sendErrorResponse(500, 'API gateway internal error');
        }
    }
    
    // 生成请求缓存键 | Generate request cache key
    // @param string $apiPath API路径 | API path
    // @param string $method 请求方法 | Request method
    // @param string $body 请求体 | Request body
    // @return string 缓存键 | Cache key
    private function generateCacheKey($apiPath, $method, $body) {
        if ($method === 'GET') {
            $queryString = $_SERVER['QUERY_STRING'] ?? '';
            return md5($method . ':' . $apiPath . '?' . $queryString);
        }
        return md5($method . ':' . $apiPath . ':' . $body);
    }
    
    // 获取缓存响应 | Get cached response
    // @param string $cacheKey 缓存键 | Cache key
    // @return array|null 缓存响应或未找到时为null | Cached response or null if not found
    private function getCachedResponse($cacheKey) {
        if (isset($this->requestCache[$cacheKey])) {
            $cachedItem = $this->requestCache[$cacheKey];
            if (time() - $cachedItem['timestamp'] < $this->cacheExpiry) {
                return $cachedItem['response'];
            } else {
                // 缓存已过期，移除 | Cache expired, remove
                unset($this->requestCache[$cacheKey]);
            }
        }
        return null;
    }
    
    // 缓存响应 | Cache response
    // @param string $cacheKey 缓存键 | Cache key
    // @param array $response 响应数据 | Response data
    private function cacheResponse($cacheKey, $response) {
        $this->requestCache[$cacheKey] = [
            'timestamp' => time(),
            'response' => $response
        ];
        
        // 限制缓存大小 | Limit cache size
        if (count($this->requestCache) > 100) {
            // 移除最旧的缓存项 | Remove oldest cache item
            reset($this->requestCache);
            unset($this->requestCache[key($this->requestCache)]);
        }
    }
    
    // 检查速率限制 | Check rate limit
    // @param string $apiPath API路径 | API path
    // @return bool 请求是否被允许 | Whether request is allowed
    private function checkRateLimit($apiPath) {
        // 静态变量存储速率限制 | Static variable to store rate limits
        static $rateLimits = [];
        
        $clientIp = $_SERVER['REMOTE_ADDR'] ?? '';
        $key = $clientIp . ':' . $apiPath;
        $now = time();
        $window = 60; // 时间窗口（秒） | Time window in seconds
        $limit = 100; // 窗口内最大请求数 | Maximum requests per window
        
        if (!isset($rateLimits[$key])) {
            $rateLimits[$key] = [];
        }
        
        // 清理旧请求 | Clean up old requests
        $rateLimits[$key] = array_filter($rateLimits[$key], function($timestamp) use ($now, $window) {
            return $now - $timestamp < $window;
        });
        
        if (count($rateLimits[$key]) >= $limit) {
            // 超出速率限制 | Rate limit exceeded
            $this->logger->warning('API Gateway Rate Limit Exceeded', [
                'api_path' => $apiPath,
                'client_ip' => $clientIp,
                'current_requests' => count($rateLimits[$key]),
                'limit' => $limit
            ]);
            return false;
        }
        
        // 添加当前请求时间戳 | Add current request timestamp
        $rateLimits[$key][] = $now;
        return true;
    }
    
    // 更新指标 | Update metrics
    // @param string $apiPath API路径 | API path
    // @param string $method 请求方法 | Request method
    // @param int $statusCode 状态码 | Status code
    // @param float $processingTime 处理时间 | Processing time
    private function updateMetrics($apiPath, $method, $statusCode, $processingTime) {
        $this->logger->debug('API Gateway Metrics', [
            'api_path' => $apiPath,
            'method' => $method,
            'status_code' => $statusCode,
            'processing_time' => round($processingTime, 4)
        ]);
    }
    
    // 过滤敏感头信息 | Filter sensitive headers
    // @param array $headers 请求头 | Request headers
    // @return array 过滤后的头信息 | Filtered headers
    private function filterSensitiveHeaders($headers) {
        // 敏感头列表 | Sensitive headers list
        $sensitiveHeaders = ['Authorization', 'X-API-Key', 'Cookie'];
        $filteredHeaders = [];
        
        foreach ($headers as $name => $value) {
            if (in_array($name, $sensitiveHeaders)) {
                // 替换敏感头值 | Replace sensitive header value
                $filteredHeaders[$name] = '***';
            } else {
                $filteredHeaders[$name] = $value;
            }
        }
        
        return $filteredHeaders;
    }
    
    // 过滤请求体中的敏感数据 | Filter sensitive data in request body
    // @param string $body 请求体 | Request body
    // @return string 过滤后的请求体 | Filtered body
    private function filterSensitiveData($body) {
        if (empty($body)) {
            return $body;
        }
        
        $data = json_decode($body, true);
        if (json_last_error() === JSON_ERROR_NONE) {
            // 敏感字段列表 | Sensitive fields list
            $sensitiveFields = ['password', 'token', 'api_key', 'auth'];
            foreach ($sensitiveFields as $field) {
                if (isset($data[$field])) {
                    // 替换敏感字段值 | Replace sensitive field value
                    $data[$field] = '***';
                }
            }
            return json_encode($data);
        }
        
        return $body;
    }
    
    // 验证请求安全性 | Validate request security
    // @param string $apiPath API路径 | API path
    // @param string $method 请求方法 | Request method
    // @param array $headers 请求头 | Request headers
    // @return bool 请求是否有效 | Whether request is valid
    private function validateRequest($apiPath, $method, $headers) {
        $clientIp = $_SERVER['REMOTE_ADDR'] ?? '';
        
        // 允许的HTTP方法列表 | Allowed HTTP methods list
        $allowedMethods = ['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS'];
        if (!in_array($method, $allowedMethods)) {
            // 无效HTTP方法 | Invalid HTTP method
            $this->logger->warning('API Gateway Invalid HTTP Method', [
                'api_path' => $apiPath,
                'method' => $method,
                'allowed_methods' => $allowedMethods
            ]);
            return false;
        }
        
        // 处理 OPTIONS 请求 | Handle OPTIONS requests
        if ($method === 'OPTIONS') {
            return true;
        }
        
        // 保护路径需要认证 | Protected paths require authentication
        if (strpos($apiPath, '/auth') === 0 || strpos($apiPath, '/admin') === 0 || strpos($apiPath, '/system') === 0) {
            // 尝试 JWT 认证 | Try JWT authentication
            $token = $this->getJwtToken($headers);
            if ($token) {
                $payload = $this->validateJwtToken($token);
                if ($payload) {
                    // 检查访问权限 | Check access permission
                    if (!$this->checkPathAccess($apiPath, $payload)) {
                        $this->logger->warning('JWT authorization failed', [
                            'api_path' => $apiPath,
                            'client_ip' => $clientIp,
                            'user_id' => $payload['sub'] ?? 'unknown'
                        ]);
                        return false;
                    }
                    
                    // JWT 认证和授权成功 | JWT authentication and authorization successful
                    $this->logger->debug('JWT authentication and authorization successful', [
                        'api_path' => $apiPath,
                        'client_ip' => $clientIp,
                        'user_id' => $payload['sub'] ?? 'unknown'
                    ]);
                    return true;
                }
            }
            
            // JWT 认证失败，尝试 API 密钥认证 | JWT authentication failed, try API key authentication
            $apiKey = $headers['X-API-Key'] ?? $headers['x-api-key'] ?? '';
            if (empty($apiKey)) {
                // 缺少认证信息 | Missing authentication
                $this->logger->warning('API Gateway Missing Authentication', [
                    'api_path' => $apiPath,
                    'client_ip' => $clientIp
                ]);
                return false;
            }
            
            try {
                $validApiKey = $this->container->get('config')['api_key'] ?? '';
                if (empty($validApiKey)) {
                    // 配置中缺少有效API密钥 | Missing valid API key in config
                    $this->logger->warning('API Gateway Missing Valid API Key in Config');
                    return false;
                }

                // 修复：使用hash_equals进行时序安全的比较，防止时序攻击
                if (!hash_equals($validApiKey, $apiKey)) {
                    // 无效API密钥 | Invalid API key
                    $this->logger->warning('API Gateway Invalid API Key', [
                        'api_path' => $apiPath,
                        'client_ip' => $clientIp
                    ]);
                    return false;
                }
                
                // API 密钥认证成功 | API key authentication successful
                return true;
            } catch (\Exception $e) {
                // API密钥验证错误 | API key validation error
                $this->logger->error('API Gateway API Key Validation Error', [
                    'error' => $e->getMessage()
                ]);
                return false;
            }
        }
        
        // 公开路径不需要认证 | Public paths don't require authentication
        return true;
    }
    
    // 发送错误响应 | Send error response
    // @param int $statusCode 状态码 | Status code
    // @param string $message 错误信息 | Error message
    private function sendErrorResponse($statusCode, $message) {
        $response = [
            'status' => $statusCode,
            'headers' => ['Content-Type' => 'application/json'],
            'body' => json_encode(['success' => false, 'message' => $message])
        ];
        
        $this->sendResponse($response);
    }
    
    // JWT密钥 | JWT secret key
    // @var string
    private $jwtSecret = '';
    
    // JWT过期时间（秒） | JWT expiration time in seconds
    // @var int
    private $jwtExpiration = 3600;
    
    // 负载均衡配置 | Load balancing configuration
    // @var array
    private $loadBalancingConfig = [
        'algorithm' => 'round_robin', // round_robin, random
        'services' => [],
        'health_check_interval' => 30 // 健康检查间隔（秒） | Health check interval in seconds
    ];
    
    // 服务健康状态 | Service health status
    // @var array
    private $serviceHealth = [];
    
    // 轮询计数器 | Round robin counter
    // @var int
    private $roundRobinCounter = 0;
    
    // 熔断机制配置 | Circuit breaker configuration
    // @var array
    private $circuitBreakerConfig = [
        'failure_threshold' => 5, // 失败阈值 | Failure threshold
        'recovery_timeout' => 30, // 恢复超时时间（秒） | Recovery timeout in seconds
        'half_open_max_requests' => 3 // 半开状态下允许的最大请求数 | Max requests in half-open state
    ];
    
    // 断路器状态 | Circuit breaker states
    const CIRCUIT_STATE_CLOSED = 'closed'; // 关闭状态 | Closed state
    const CIRCUIT_STATE_OPEN = 'open'; // 打开状态 | Open state
    const CIRCUIT_STATE_HALF_OPEN = 'half_open'; // 半开状态 | Half-open state
    
    // 会话管理配置 | Session management configuration
    // @var array
    private $sessionConfig = [
        'storage' => 'memory', // 会话存储方式 | Session storage type
        'expiration' => 3600, // 会话过期时间（秒） | Session expiration time in seconds
        'refresh_expiration' => 7200 // 会话刷新过期时间（秒） | Session refresh expiration time in seconds
    ];
    
    // 活跃会话存储 | Active sessions storage
    // @var array
    private $activeSessions = [];
    
    // 分布式追踪配置 | Distributed tracing configuration
    // @var array
    private $tracingConfig = [
        'enabled' => true, // 是否启用追踪 | Whether tracing is enabled
        'provider' => 'opentelemetry', // 追踪提供商 | Tracing provider
        'sampling_rate' => 1.0 // 采样率 | Sampling rate (0.0 to 1.0)
    ];
    
    // 性能监控配置 | Performance monitoring configuration
    // @var array
    private $performanceMonitoringConfig = [
        'enabled' => true, // 是否启用性能监控 | Whether performance monitoring is enabled
        'batch_size' => 100, // 批量发送大小 | Batch size for sending metrics
        'collection_interval' => 30 // 收集间隔（秒）| Collection interval in seconds
    ];
    
    // 告警配置 | Alerting configuration
    // @var array
    private $alertingConfig = [
        'enabled' => true, // 是否启用告警 | Whether alerting is enabled
        'thresholds' => [], // 告警阈值 | Alert thresholds
        'notification_channels' => ['email', 'webhook'] // 通知渠道 | Notification channels
    ];
    
    // 解析API路径 | Parse API path
    // @param string $path 原始请求路径 | Original request path
    // @return string 解析后的API路径 | Parsed API path
    private function parseApiPath($path) {
        // 移除/api/gateway前缀 | Remove /api/gateway prefix
        $apiPath = preg_replace('#^/api/gateway#', '', $path);
        
        // 提取API版本 | Extract API version from path
        if (preg_match('#^/v([0-9]+)#', $apiPath, $matches)) {
            $this->apiVersion = 'v' . $matches[1];
            // 移除版本前缀 | Remove version prefix from path
            $apiPath = preg_replace('#^/v[0-9]+#', '', $apiPath);
        } else {
            // 使用默认版本 | Use default version
            $this->apiVersion = 'v1';
        }
        
        // 移除查询字符串 | Remove query string
        $apiPath = preg_replace('#\?.*$#', '', $apiPath);
        
        if (empty($apiPath)) {
            $apiPath = '/';
        } elseif (substr($apiPath, 0, 1) !== '/') {
            // 确保路径以/开头 | Ensure path starts with /
            $apiPath = '/' . $apiPath;
        }
        
        return $apiPath;
    }
    
    // 转发API请求 | Forward API request
    // @param string $apiPath API路径 | API path
    // @param string $method 请求方法 | Request method
    // @param array $headers 请求头 | Request headers
    // @param string $body 请求体 | Request body
    // @return array 响应数据 | Response data
    private function forwardRequest($apiPath, $method, $headers, $body) {
        try {
            // 本地控制器调用模式 | Local controller call mode
            // 获取转发目标 | Get forward target
            $target = $this->getForwardTarget($apiPath, $method);
            
            if (!$target) {
                // API路径未找到 | API path not found
                return [
                    'status' => 404,
                    'headers' => ['Content-Type' => 'application/json'],
                    'body' => json_encode(['success' => false, 'message' => 'API path not found'])
                ];
            }
            
            // 转换请求体 | Convert request body
            $convertedBody = $this->convertRequestBody($body, $target);
            
            // 调用控制器方法 | Call controller method
            return $this->callControllerMethod($target, $convertedBody);
        } catch (\Exception $e) {
            // 请求转发失败 | Request forwarding failed
            $this->logger->error('API Gateway Forward Error', [
                'error' => $e->getMessage(),
                'api_path' => $apiPath,
                'method' => $method
            ]);
            
            return [
                'status' => 502,
                'headers' => ['Content-Type' => 'application/json'],
                'body' => json_encode(['success' => false, 'message' => 'Request forwarding failed: ' . $e->getMessage()])
            ];
        }
    }
    
    // 调用远程服务 | Call remote service
    // @param array $service 服务配置 | Service configuration
    // @param string $apiPath API路径 | API path
    // @param string $method 请求方法 | Request method
    // @param array $headers 请求头 | Request headers
    // @param string $body 请求体 | Request body
    // @return array 响应数据 | Response data
    private function callRemoteService($service, $apiPath, $method, $headers, $body) {
        $url = $service['url'] ?? '';
        if (empty($url)) {
            throw new \Exception('Invalid service URL');
        }
        
        $serviceName = $service['name'] ?? $url;
        
        // 检查断路器状态 | Check circuit breaker state
        if (!$this->isServiceAllowed($serviceName)) {
            throw new \Exception('Circuit breaker is open, service temporarily unavailable');
        }
        
        $fullUrl = $url . $apiPath;
        if (!empty($_SERVER['QUERY_STRING'])) {
            $fullUrl .= '?' . $_SERVER['QUERY_STRING'];
        }
        
        try {
            $ch = curl_init($fullUrl);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $method);
            curl_setopt($ch, CURLOPT_TIMEOUT, 10);
            
            // 设置请求头 | Set request headers
            $curlHeaders = [];
            foreach ($headers as $name => $value) {
                $curlHeaders[] = $name . ': ' . $value;
            }
            curl_setopt($ch, CURLOPT_HTTPHEADER, $curlHeaders);
            
            // 设置请求体 | Set request body
            if (!empty($body)) {
                curl_setopt($ch, CURLOPT_POSTFIELDS, $body);
            }
            
            // 执行请求 | Execute request
            $responseBody = curl_exec($ch);
            $statusCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            $responseHeaders = curl_getinfo($ch, CURLINFO_HEADER_OUT);
            curl_close($ch);
            
            // 解析响应头 | Parse response headers
            $parsedHeaders = [];
            foreach (explode("\r\n", $responseHeaders) as $headerLine) {
                if (strpos($headerLine, ':') !== false) {
                    list($name, $value) = explode(':', $headerLine, 2);
                    $parsedHeaders[trim($name)] = trim($value);
                }
            }
            
            $response = [
                'status' => $statusCode,
                'headers' => $parsedHeaders,
                'body' => $responseBody
            ];
            
            // 检查响应是否成功 | Check if response is successful
            $isSuccess = $statusCode >= 200 && $statusCode < 500;
            
            // 更新断路器状态 | Update circuit breaker state
            $this->updateCircuitBreakerState($serviceName, $isSuccess);
            
            return $response;
        } catch (\Exception $e) {
            // 更新断路器状态 | Update circuit breaker state on failure
            $this->updateCircuitBreakerState($serviceName, false);
            
            $this->logger->error('Remote service call failed', [
                'error' => $e->getMessage(),
                'service' => $serviceName,
                'api_path' => $apiPath,
                'method' => $method
            ]);
            throw $e;
        }
    }
    
    // 获取转发目标 | Get forward target
    // @param string $apiPath API路径 | API path
    // @param string $method 请求方法 | Request method
    // @return array|null 转发目标信息或未找到时为null | Forward target info or null if not found
    private function getForwardTarget($apiPath, $method) {
        // 静态变量存储路径映射 | Static variable to store path mappings
        static $pathMap = null;
        
        if ($pathMap === null) {
            // 初始化路径映射 | Initialize path mappings
            $pathMap = [
                // 版本无关的路径 | Version-independent paths
                '/health' => [
                    'controller' => 'HealthController',
                    'method' => 'check',
                    'params' => [],
                    'methods' => ['GET']
                ],
                '/' => [
                    'controller' => 'ApiGatewayController',
                    'method' => 'getGatewayInfo',
                    'params' => [],
                    'methods' => ['GET']
                ]
            ];
        }
        
        // 构建版本特定的控制器类名前缀 | Build version-specific controller class name prefix
        $versionedControllers = [
            'v1' => '',  // v1 使用默认控制器 | v1 uses default controllers
            'v2' => 'V2\\',  // v2 使用 V2 命名空间下的控制器 | v2 uses controllers in V2 namespace
            'v3' => 'V3\\'   // v3 使用 V3 命名空间下的控制器 | v3 uses controllers in V3 namespace
        ];
        
        $controllerPrefix = $versionedControllers[$this->apiVersion] ?? '';
        
        foreach ($pathMap as $path => $target) {
            // 跳过包含参数的路径（暂时不支持） | Skip paths with parameters (not supported yet)
            if (strpos($path, '{') !== false) {
                continue;
            }
            
            if ($apiPath === $path) {
                if (empty($target['methods']) || in_array($method, $target['methods'])) {
                    // 添加版本前缀到控制器类名 | Add version prefix to controller class name
                    $versionedTarget = $target;
                    $versionedTarget['controller'] = $controllerPrefix . $target['controller'];
                    return $versionedTarget;
                } else {
                    // 方法不允许 | Method not allowed
                    $this->logger->warning('API Gateway Method Not Allowed', [
                        'api_path' => $apiPath,
                        'api_version' => $this->apiVersion,
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
        
        // 路径未找到 | Path not found
        $this->logger->warning('API Gateway Path Not Found', [
            'path' => $apiPath,
            'api_version' => $this->apiVersion,
            'method' => $method,
            'client_ip' => $_SERVER['REMOTE_ADDR'] ?? ''
        ]);
        
        return null;
    }
    
    // 获取网关信息 | Get gateway info
    // @param array $params 参数 | Parameters
    // @return array 响应数据 | Response data
    public function getGatewayInfo($params) {
        return [
            'success' => true,
            'message' => 'InkClock API Gateway',
            'version' => '1.0.0',
            'timestamp' => time()
        ];
    }
    
    // 方法不允许响应 | Method not allowed response
    // @param array $params 参数 | Parameters
    // @return array 响应数据 | Response data
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
                'message' => 'Method not allowed',
                'allowed_methods' => $allowedMethods
            ])
        ];
    }
    
    // 转换请求体 | Convert request body
    // @param string $body 原始请求体 | Original request body
    // @param array $target 转发目标 | Forward target
    // @return array 转换后的请求数据 | Converted request data
    private function convertRequestBody($body, $target) {
        $data = [];
        
        // 检查是否为文件上传请求 | Check if it's a file upload request
        $contentType = $_SERVER['CONTENT_TYPE'] ?? '';
        if (strpos($contentType, 'multipart/form-data') !== false) {
            // 处理文件上传 | Handle file uploads
            $data = $_POST ?? [];
            
            // 添加文件信息 | Add file information
            if (!empty($_FILES)) {
                $data['files'] = $_FILES;
            }
        } elseif (!empty($body)) {
            // 尝试JSON解析 | Try JSON parsing
            $data = json_decode($body, true);
            if (json_last_error() !== JSON_ERROR_NONE) {
                // JSON解析失败，尝试解析为表单数据 | JSON parsing failed, try parsing as form data
                parse_str($body, $data);
            }
        }
        
        // 处理路径参数 | Handle path parameters
        if (isset($target['params'])) {
            foreach ($target['params'] as $key => $value) {
                if (strpos($value, '{') === 0 && strpos($value, '}') === strlen($value) - 1) {
                    // 提取参数名 | Extract parameter name
                    $paramName = substr($value, 1, -1);
                    if (isset($data[$paramName])) {
                        // 替换参数值 | Replace parameter value
                        $target['params'][$key] = $data[$paramName];
                    }
                }
            }
        }
        
        // 合并参数 | Merge parameters
        return array_merge($target['params'] ?? [], $data);
    }
    
    // 调用控制器方法 | Call controller method
    // @param array $target 转发目标 | Forward target
    // @param array $data 请求数据 | Request data
    // @return array 响应数据 | Response data
    private function callControllerMethod($target, $data) {
        try {
            $startTime = microtime(true);
            
            // 构建控制器类名 | Build controller class name
            $controllerClass = 'InkClock\\Controller\\' . $target['controller'];
            
            if (!class_exists($controllerClass)) {
                throw new \Exception('Controller class not found: ' . $controllerClass);
            }
            
            $controller = null;
            try {
                // 尝试带容器参数实例化 | Try instantiation with container parameter
                $controller = new $controllerClass($this->container);
            } catch (\ArgumentCountError $e) {
                try {
                    // 尝试无参数实例化 | Try instantiation without parameters
                    $controller = new $controllerClass();
                } catch (\Exception $e2) {
                    // 控制器实例化错误 | Controller instantiation error
                    $this->logger->error('Controller instantiation error', [
                        'error' => $e2->getMessage(),
                        'controller' => $controllerClass
                    ]);
                    throw new \Exception('Failed to instantiate controller: ' . $controllerClass);
                }
            }
            
            $method = $target['method'];
            if (!method_exists($controller, $method)) {
                throw new \Exception('Method not found: ' . $method);
            }
            
            $result = null;
            
            // 使用反射获取方法参数 | Use reflection to get method parameters
            $reflection = new \ReflectionMethod($controllerClass, $method);
            $params = $reflection->getParameters();
            
            if (empty($params)) {
                // 无参数方法 | Method with no parameters
                $result = $controller->$method();
            } else if (count($params) === 1) {
                // 单参数方法 | Method with single parameter
                $result = $controller->$method($data);
            } else {
                // 多参数方法 | Method with multiple parameters
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
            
            if (!is_array($result)) {
                // 确保结果是数组 | Ensure result is array
                $result = [
                    'success' => true,
                    'data' => $result
                ];
            }
            
            // 添加元数据 | Add metadata
            $result['meta'] = [
                'timestamp' => time(),
                'processing_time' => round(microtime(true) - $startTime, 4),
                'controller' => $target['controller'],
                'method' => $target['method']
            ];
            
            return [
                'status' => 200,
                'headers' => ['Content-Type' => 'application/json'],
                'body' => json_encode($result)
            ];
        } catch (\Exception $e) {
            // API网关错误 | API gateway error
            $this->logger->error('API Gateway Error', [
                'error' => $e->getMessage(),
                'trace' => $e->getTraceAsString(),
                'controller' => $target['controller'],
                'method' => $target['method']
            ]);
            
            return [
                'status' => 500,
                'headers' => ['Content-Type' => 'application/json'],
                'body' => json_encode(['success' => false, 'message' => 'API gateway error: ' . $e->getMessage()])
            ];
        }
    }
    
    // 初始化 JWT 配置 | Initialize JWT configuration
    private function initJwtConfig() {
        try {
            $config = $this->container->get('config');
            if (method_exists($config, 'get')) {
                $this->jwtSecret = $config->get('jwt_secret') ?? 'default_jwt_secret_key';
                $this->jwtExpiration = $config->get('jwt_expiration') ?? 3600;
            } else {
                $this->jwtSecret = $config['jwt_secret'] ?? 'default_jwt_secret_key';
                $this->jwtExpiration = $config['jwt_expiration'] ?? 3600;
            }
        } catch (\Exception $e) {
            $this->logger->warning('Failed to load JWT config, using defaults', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 从请求中获取 JWT 令牌 | Get JWT token from request
    // @param array $headers 请求头 | Request headers
    // @return string|null JWT令牌或未找到时为null | JWT token or null if not found
    private function getJwtToken($headers) {
        $authorizationHeader = $headers['Authorization'] ?? $headers['authorization'] ?? '';
        if (preg_match('/^Bearer\s+(.*)$/', $authorizationHeader, $matches)) {
            return $matches[1];
        }
        return null;
    }
    
    // 验证 JWT 令牌 | Validate JWT token
    // @param string $token JWT令牌 | JWT token
    // @return array|null 解码后的JWT数据或验证失败时为null | Decoded JWT data or null if validation fails
    private function validateJwtToken($token) {
        try {
            // 初始化 JWT 配置 | Initialize JWT config if not already done
            if (empty($this->jwtSecret)) {
                $this->initJwtConfig();
            }
            
            // 解析 JWT 令牌 | Parse JWT token
            $parts = explode('.', $token);
            if (count($parts) !== 3) {
                return null;
            }
            
            // 解码头部和载荷 | Decode header and payload
            $header = json_decode(base64_decode(str_replace(['-', '_'], ['+', '/'], $parts[0])), true);
            $payload = json_decode(base64_decode(str_replace(['-', '_'], ['+', '/'], $parts[1])), true);
            
            // 检查签名 | Verify signature
            $signature = hash_hmac('sha256', $parts[0] . '.' . $parts[1], $this->jwtSecret, true);
            $encodedSignature = str_replace(['+', '/', '='], ['-', '_', ''], base64_encode($signature));
            
            if ($encodedSignature !== $parts[2]) {
                return null;
            }
            
            // 检查过期时间 | Check expiration time
            if (isset($payload['exp']) && time() > $payload['exp']) {
                return null;
            }
            
            return $payload;
        } catch (\Exception $e) {
            $this->logger->warning('JWT validation failed', [
                'error' => $e->getMessage()
            ]);
            return null;
        }
    }
    
    // 检查用户是否具有指定角色 | Check if user has specified role
    // @param array $payload JWT载荷 | JWT payload
    // @param string $role 角色名称 | Role name
    // @return bool 是否具有该角色 | Whether user has the role
    private function hasRole($payload, $role) {
        $roles = $payload['roles'] ?? [];
        if (is_string($roles)) {
            $roles = explode(',', $roles);
        }
        return in_array($role, $roles);
    }
    
    // 检查用户是否具有指定权限 | Check if user has specified permission
    // @param array $payload JWT载荷 | JWT payload
    // @param string $permission 权限名称 | Permission name
    // @return bool 是否具有该权限 | Whether user has the permission
    private function hasPermission($payload, $permission) {
        $permissions = $payload['permissions'] ?? [];
        if (is_string($permissions)) {
            $permissions = explode(',', $permissions);
        }
        return in_array($permission, $permissions);
    }
    
    // 检查用户是否具有访问路径的权限 | Check if user has access to path
    // @param string $apiPath API路径 | API path
    // @param array $payload JWT载荷 | JWT payload
    // @return bool 是否具有访问权限 | Whether user has access
    private function checkPathAccess($apiPath, $payload) {
        // 定义路径访问规则 | Define path access rules
        $accessRules = [
            '/admin' => ['role' => 'admin'],
            '/system' => ['role' => 'admin'],
            '/auth' => ['permission' => 'auth:access']
        ];
        
        // 检查路径匹配 | Check path matches
        foreach ($accessRules as $pathPattern => $rule) {
            if (strpos($apiPath, $pathPattern) === 0) {
                if (isset($rule['role']) && !$this->hasRole($payload, $rule['role'])) {
                    return false;
                }
                if (isset($rule['permission']) && !$this->hasPermission($payload, $rule['permission'])) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // 初始化负载均衡配置 | Initialize load balancing configuration
    private function initLoadBalancingConfig() {
        try {
            $config = $this->container->get('config');
            $loadBalancingConfig = [];
            if (method_exists($config, 'get')) {
                $loadBalancingConfig = $config->get('load_balancing') ?? [];
            } else {
                $loadBalancingConfig = $config['load_balancing'] ?? [];
            }
            $this->loadBalancingConfig = array_merge(
                $this->loadBalancingConfig,
                $loadBalancingConfig
            );
        } catch (\Exception $e) {
            $this->logger->warning('Failed to load load balancing config, using defaults', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 检查服务健康状态 | Check service health status
    // @param array $service 服务配置 | Service configuration
    // @return bool 服务是否健康 | Whether service is healthy
    private function checkServiceHealth($service) {
        $url = $service['url'] ?? '';
        if (empty($url)) {
            return false;
        }
        
        try {
            $ch = curl_init($url . '/health');
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_TIMEOUT, 5);
            $response = curl_exec($ch);
            $statusCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            curl_close($ch);
            
            return $statusCode >= 200 && $statusCode < 300;
        } catch (\Exception $e) {
            $this->logger->warning('Service health check failed', [
                'service' => $service['name'] ?? $url,
                'error' => $e->getMessage()
            ]);
            return false;
        }
    }
    
    // 更新所有服务健康状态 | Update all service health status
    private function updateServiceHealth() {
        $services = $this->loadBalancingConfig['services'] ?? [];
        $currentTime = time();
        
        foreach ($services as $index => $service) {
            $lastCheck = $this->serviceHealth[$index]['last_check'] ?? 0;
            $interval = $this->loadBalancingConfig['health_check_interval'] ?? 30;
            
            if ($currentTime - $lastCheck >= $interval) {
                $isHealthy = $this->checkServiceHealth($service);
                $this->serviceHealth[$index] = [
                    'healthy' => $isHealthy,
                    'last_check' => $currentTime
                ];
                
                if (!$isHealthy) {
                    $this->logger->warning('Service is unhealthy', [
                        'service' => $service['name'] ?? $service['url'] ?? 'unknown'
                    ]);
                }
            }
        }
    }
    
    // 使用轮询算法选择服务 | Select service using round robin algorithm
    // @param array $healthyServices 健康服务列表 | List of healthy services
    // @return array|null 选中的服务或未找到时为null | Selected service or null if not found
    private function selectServiceRoundRobin($healthyServices) {
        if (empty($healthyServices)) {
            return null;
        }
        
        $index = $this->roundRobinCounter % count($healthyServices);
        $this->roundRobinCounter++;
        
        return $healthyServices[$index];
    }
    
    // 使用随机算法选择服务 | Select service using random algorithm
    // @param array $healthyServices 健康服务列表 | List of healthy services
    // @return array|null 选中的服务或未找到时为null | Selected service or null if not found
    private function selectServiceRandom($healthyServices) {
        if (empty($healthyServices)) {
            return null;
        }
        
        $index = array_rand($healthyServices);
        return $healthyServices[$index];
    }
    
    // 初始化熔断机制配置 | Initialize circuit breaker configuration
    private function initCircuitBreakerConfig() {
        try {
            $config = $this->container->get('config');
            $circuitBreakerConfig = [];
            if (method_exists($config, 'get')) {
                $circuitBreakerConfig = $config->get('circuit_breaker') ?? [];
            } else {
                $circuitBreakerConfig = $config['circuit_breaker'] ?? [];
            }
            $this->circuitBreakerConfig = array_merge(
                $this->circuitBreakerConfig,
                $circuitBreakerConfig
            );
        } catch (\Exception $e) {
            $this->logger->warning('Failed to load circuit breaker config, using defaults', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 获取服务的断路器状态 | Get circuit breaker state for service
    // @param string $serviceName 服务名称 | Service name
    // @return array 断路器状态 | Circuit breaker state
    private function getCircuitBreakerState($serviceName) {
        // 初始化配置 | Initialize config if not already done
        if (empty($this->circuitBreakerConfig['failure_threshold'])) {
            $this->initCircuitBreakerConfig();
        }
        
        if (!isset($this->circuitBreakerStates[$serviceName])) {
            // 初始化断路器状态 | Initialize circuit breaker state
            $this->circuitBreakerStates[$serviceName] = [
                'state' => self::CIRCUIT_STATE_CLOSED,
                'failure_count' => 0,
                'last_failure_time' => 0,
                'half_open_requests' => 0
            ];
        }
        
        $state = $this->circuitBreakerStates[$serviceName];
        
        // 检查是否应该从打开状态转换到半开状态 | Check if should transition from open to half-open
        if ($state['state'] === self::CIRCUIT_STATE_OPEN) {
            $elapsedTime = time() - $state['last_failure_time'];
            if ($elapsedTime >= $this->circuitBreakerConfig['recovery_timeout']) {
                $state['state'] = self::CIRCUIT_STATE_HALF_OPEN;
                $state['half_open_requests'] = 0;
                $this->circuitBreakerStates[$serviceName] = $state;
                $this->logger->info('Circuit breaker transitioned to half-open state', [
                    'service' => $serviceName
                ]);
            }
        }
        
        return $state;
    }
    
    // 更新断路器状态 | Update circuit breaker state
    // @param string $serviceName 服务名称 | Service name
    // @param bool $success 请求是否成功 | Whether request was successful
    private function updateCircuitBreakerState($serviceName, $success) {
        $state = $this->getCircuitBreakerState($serviceName);
        
        if ($success) {
            switch ($state['state']) {
                case self::CIRCUIT_STATE_CLOSED:
                    // 重置失败计数 | Reset failure count
                    $state['failure_count'] = 0;
                    break;
                    
                case self::CIRCUIT_STATE_HALF_OPEN:
                    // 请求成功，关闭断路器 | Request successful, close circuit breaker
                    $state['state'] = self::CIRCUIT_STATE_CLOSED;
                    $state['failure_count'] = 0;
                    $state['half_open_requests'] = 0;
                    $this->logger->info('Circuit breaker transitioned to closed state', [
                        'service' => $serviceName
                    ]);
                    break;
            }
        } else {
            switch ($state['state']) {
                case self::CIRCUIT_STATE_CLOSED:
                    // 增加失败计数 | Increase failure count
                    $state['failure_count']++;
                    $state['last_failure_time'] = time();
                    
                    // 检查是否超过失败阈值 | Check if failure threshold exceeded
                    if ($state['failure_count'] >= $this->circuitBreakerConfig['failure_threshold']) {
                        $state['state'] = self::CIRCUIT_STATE_OPEN;
                        $this->logger->warning('Circuit breaker transitioned to open state', [
                            'service' => $serviceName,
                            'failure_count' => $state['failure_count']
                        ]);
                    }
                    break;
                    
                case self::CIRCUIT_STATE_HALF_OPEN:
                    // 半开状态下请求失败，重新打开断路器 | Request failed in half-open state, reopen circuit breaker
                    $state['state'] = self::CIRCUIT_STATE_OPEN;
                    $state['last_failure_time'] = time();
                    $this->logger->warning('Circuit breaker transitioned back to open state', [
                        'service' => $serviceName
                    ]);
                    break;
            }
        }
        
        $this->circuitBreakerStates[$serviceName] = $state;
    }
    
    // 检查服务是否允许访问 | Check if service is allowed to be accessed
    // @param string $serviceName 服务名称 | Service name
    // @return bool 是否允许访问 | Whether service is allowed to be accessed
    private function isServiceAllowed($serviceName) {
        $state = $this->getCircuitBreakerState($serviceName);
        
        switch ($state['state']) {
            case self::CIRCUIT_STATE_CLOSED:
                return true;
                
            case self::CIRCUIT_STATE_HALF_OPEN:
                // 检查半开状态下的请求数 | Check request count in half-open state
                if ($state['half_open_requests'] < $this->circuitBreakerConfig['half_open_max_requests']) {
                    $state['half_open_requests']++;
                    $this->circuitBreakerStates[$serviceName] = $state;
                    return true;
                }
                return false;
                
            case self::CIRCUIT_STATE_OPEN:
                $this->logger->warning('Circuit breaker is open, request rejected', [
                    'service' => $serviceName
                ]);
                return false;
        }
        
        return true;
    }
    
    // 初始化会话配置 | Initialize session configuration
    private function initSessionConfig() {
        try {
            $config = $this->container->get('config');
            $sessionConfig = [];
            if (method_exists($config, 'get')) {
                $sessionConfig = $config->get('session') ?? [];
            } else {
                $sessionConfig = $config['session'] ?? [];
            }
            $this->sessionConfig = array_merge(
                $this->sessionConfig,
                $sessionConfig
            );
        } catch (\Exception $e) {
            $this->logger->warning('Failed to load session config, using defaults', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 创建会话 | Create session
    // @param array $userData 用户数据 | User data
    // @return array 会话数据 | Session data
    public function createSession($userData) {
        // 初始化会话配置 | Initialize session config if not already done
        if (empty($this->sessionConfig['expiration'])) {
            $this->initSessionConfig();
        }
        
        // 生成会话 ID | Generate session ID
        $sessionId = bin2hex(random_bytes(16));
        
        // 生成 JWT 令牌 | Generate JWT token
        $payload = [
            'sub' => $userData['id'] ?? '',
            'user' => $userData,
            'exp' => time() + $this->sessionConfig['expiration'],
            'iat' => time(),
            'jti' => $sessionId
        ];
        
        $token = $this->generateJwtToken($payload);
        
        // 保存会话 | Save session
        $this->activeSessions[$sessionId] = [
            'session_id' => $sessionId,
            'user_data' => $userData,
            'token' => $token,
            'created_at' => time(),
            'last_activity' => time(),
            'expires_at' => time() + $this->sessionConfig['expiration']
        ];
        
        $this->logger->info('Session created', [
            'session_id' => $sessionId,
            'user_id' => $userData['id'] ?? 'unknown'
        ]);
        
        return [
            'session_id' => $sessionId,
            'token' => $token,
            'user_data' => $userData,
            'expires_at' => $this->activeSessions[$sessionId]['expires_at']
        ];
    }
    
    // 生成 JWT 令牌 | Generate JWT token
    // @param array $payload JWT载荷 | JWT payload
    // @return string JWT令牌 | JWT token
    private function generateJwtToken($payload) {
        // 初始化 JWT 配置 | Initialize JWT config if not already done
        if (empty($this->jwtSecret)) {
            $this->initJwtConfig();
        }
        
        // 生成头部 | Generate header
        $header = json_encode(['alg' => 'HS256', 'typ' => 'JWT']);
        $headerEncoded = str_replace(['+', '/', '='], ['-', '_', ''], base64_encode($header));
        
        // 生成载荷 | Generate payload
        $payloadEncoded = str_replace(['+', '/', '='], ['-', '_', ''], base64_encode(json_encode($payload)));
        
        // 生成签名 | Generate signature
        $signature = hash_hmac('sha256', $headerEncoded . '.' . $payloadEncoded, $this->jwtSecret, true);
        $signatureEncoded = str_replace(['+', '/', '='], ['-', '_', ''], base64_encode($signature));
        
        // 生成令牌 | Generate token
        return $headerEncoded . '.' . $payloadEncoded . '.' . $signatureEncoded;
    }
    
    // 获取会话 | Get session
    // @param string $sessionId 会话 ID | Session ID
    // @return array|null 会话数据或未找到时为null | Session data or null if not found
    public function getSession($sessionId) {
        if (!isset($this->activeSessions[$sessionId])) {
            return null;
        }
        
        $session = $this->activeSessions[$sessionId];
        
        // 检查会话是否过期 | Check if session is expired
        if (time() > $session['expires_at']) {
            $this->deleteSession($sessionId);
            return null;
        }
        
        // 更新最后活动时间 | Update last activity time
        $session['last_activity'] = time();
        $this->activeSessions[$sessionId] = $session;
        
        return $session;
    }
    
    // 更新会话 | Update session
    // @param string $sessionId 会话 ID | Session ID
    // @param array $userData 用户数据 | User data
    // @return bool 是否更新成功 | Whether update was successful
    public function updateSession($sessionId, $userData) {
        $session = $this->getSession($sessionId);
        if (!$session) {
            return false;
        }
        
        // 更新会话数据 | Update session data
        $this->activeSessions[$sessionId]['user_data'] = array_merge(
            $session['user_data'],
            $userData
        );
        $this->activeSessions[$sessionId]['last_activity'] = time();
        
        $this->logger->info('Session updated', [
            'session_id' => $sessionId,
            'user_id' => $userData['id'] ?? $session['user_data']['id'] ?? 'unknown'
        ]);
        
        return true;
    }
    
    // 删除会话 | Delete session
    // @param string $sessionId 会话 ID | Session ID
    // @return bool 是否删除成功 | Whether deletion was successful
    public function deleteSession($sessionId) {
        if (isset($this->activeSessions[$sessionId])) {
            $userId = $this->activeSessions[$sessionId]['user_data']['id'] ?? 'unknown';
            unset($this->activeSessions[$sessionId]);
            
            $this->logger->info('Session deleted', [
                'session_id' => $sessionId,
                'user_id' => $userId
            ]);
            
            return true;
        }
        return false;
    }
    
    // 刷新会话 | Refresh session
    // @param string $sessionId 会话 ID | Session ID
    // @return array|null 刷新后的会话数据或刷新失败时为null | Refreshed session data or null if refresh failed
    public function refreshSession($sessionId) {
        $session = $this->getSession($sessionId);
        if (!$session) {
            return null;
        }
        
        // 生成新的 JWT 令牌 | Generate new JWT token
        $payload = [
            'sub' => $session['user_data']['id'] ?? '',
            'user' => $session['user_data'],
            'exp' => time() + $this->sessionConfig['expiration'],
            'iat' => time(),
            'jti' => $sessionId
        ];
        
        $newToken = $this->generateJwtToken($payload);
        
        // 更新会话 | Update session
        $this->activeSessions[$sessionId]['token'] = $newToken;
        $this->activeSessions[$sessionId]['last_activity'] = time();
        $this->activeSessions[$sessionId]['expires_at'] = time() + $this->sessionConfig['expiration'];
        
        $this->logger->info('Session refreshed', [
            'session_id' => $sessionId,
            'user_id' => $session['user_data']['id'] ?? 'unknown'
        ]);
        
        return [
            'session_id' => $sessionId,
            'token' => $newToken,
            'user_data' => $session['user_data'],
            'expires_at' => $this->activeSessions[$sessionId]['expires_at']
        ];
    }
    
    // 清除过期会话 | Cleanup expired sessions
    public function cleanupExpiredSessions() {
        $now = time();
        $cleanedCount = 0;
        
        foreach ($this->activeSessions as $sessionId => $session) {
            if ($now > $session['expires_at']) {
                $this->deleteSession($sessionId);
                $cleanedCount++;
            }
        }
        
        if ($cleanedCount > 0) {
            $this->logger->info('Cleaned up expired sessions', [
                'count' => $cleanedCount
            ]);
        }
    }
    
    // 初始化分布式追踪配置 | Initialize distributed tracing configuration
    private function initTracingConfig() {
        try {
            $config = $this->container->get('config');
            $tracingConfig = [];
            if (method_exists($config, 'get')) {
                $tracingConfig = $config->get('tracing') ?? [];
            } else {
                $tracingConfig = $config['tracing'] ?? [];
            }
            $this->tracingConfig = array_merge(
                $this->tracingConfig,
                $tracingConfig
            );
        } catch (\Exception $e) {
            $this->logger->warning('Failed to load tracing config, using defaults', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 生成唯一的追踪 ID | Generate unique trace ID
    // @return string 追踪 ID | Trace ID
    private function generateTraceId() {
        return bin2hex(random_bytes(16));
    }
    
    // 生成唯一的 span ID | Generate unique span ID
    // @return string Span ID
    private function generateSpanId() {
        return bin2hex(random_bytes(8));
    }
    
    // 初始化追踪上下文 | Initialize tracing context
    // @param array $headers 请求头 | Request headers
    private function initTracingContext($headers) {
        // 初始化配置 | Initialize config if not already done
        if (empty($this->tracingConfig['enabled'])) {
            $this->initTracingConfig();
        }
        
        if (!$this->tracingConfig['enabled']) {
            $this->tracingContext = [];
            return;
        }
        
        // 检查是否应该采样 | Check if should sample
        $shouldSample = (float) $this->tracingConfig['sampling_rate'] >= 1.0 || mt_rand(0, 1000000) / 1000000 < (float) $this->tracingConfig['sampling_rate'];
        
        if (!$shouldSample) {
            $this->tracingContext = [];
            return;
        }
        
        // 从请求头中获取追踪上下文 | Get tracing context from headers
        $traceId = $headers['X-Trace-ID'] ?? $headers['x-trace-id'] ?? '';
        $spanId = $headers['X-Span-ID'] ?? $headers['x-span-id'] ?? '';
        $parentSpanId = $headers['X-Parent-Span-ID'] ?? $headers['x-parent-span-id'] ?? '';
        $sampled = $headers['X-Sampled'] ?? $headers['x-sampled'] ?? '';
        
        // 如果没有追踪 ID，则生成新的 | Generate new trace ID if not found
        if (empty($traceId)) {
            $traceId = $this->generateTraceId();
            $spanId = $this->generateSpanId();
            $parentSpanId = '';
            $sampled = '1';
        } else if (empty($spanId)) {
            $spanId = $this->generateSpanId();
            $parentSpanId = $headers['X-Span-ID'] ?? $headers['x-span-id'] ?? '';
        }
        
        $this->tracingContext = [
            'trace_id' => $traceId,
            'span_id' => $spanId,
            'parent_span_id' => $parentSpanId,
            'sampled' => $sampled,
            'trace_flags' => '01' // 表示采样 | Indicates sampled
        ];
        
        $this->logger->debug('Initialized tracing context', $this->tracingContext);
    }
    
    // 获取追踪上下文 | Get tracing context
    // @return array 追踪上下文 | Tracing context
    public function getTracingContext() {
        return $this->tracingContext;
    }
    
    // 更新追踪上下文 | Update tracing context
    // @param array $context 新的上下文信息 | New context information
    public function updateTracingContext($context) {
        $this->tracingContext = array_merge($this->tracingContext, $context);
    }
    
    // 初始化性能监控配置 | Initialize performance monitoring configuration
    private function initPerformanceMonitoringConfig() {
        try {
            $config = $this->container->get('config') ?? [];
            $this->performanceMonitoringConfig = array_merge(
                $this->performanceMonitoringConfig,
                $config['performance_monitoring'] ?? []
            );
        } catch (\Exception $e) {
            $this->logger->warning('Failed to load performance monitoring config, using defaults', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 记录性能指标 | Record performance metrics
    // @param string $metricName 指标名称 | Metric name
    // @param float $value 指标值 | Metric value
    // @param array $tags 标签 | Tags
    private function recordPerformanceMetric($metricName, $value, $tags = []) {
        // 初始化配置 | Initialize config if not already done
        if (empty($this->performanceMonitoringConfig['enabled'])) {
            $this->initPerformanceMonitoringConfig();
        }
        
        if (!$this->performanceMonitoringConfig['enabled']) {
            return;
        }
        
        $metric = [
            'name' => $metricName,
            'value' => $value,
            'timestamp' => time(),
            'tags' => array_merge($tags, $this->tracingContext)
        ];
        
        // 保存指标到内存 | Save metric to memory
        $this->performanceMetrics[] = $metric;
        
        // 定期发送指标 | Send metrics periodically
        if (count($this->performanceMetrics) >= $this->performanceMonitoringConfig['batch_size']) {
            $this->sendPerformanceMetrics();
        }
    }
    
    // 发送性能指标 | Send performance metrics
    private function sendPerformanceMetrics() {
        if (empty($this->performanceMetrics)) {
            return;
        }
        
        try {
            // 在实际实现中，这里应该将指标发送到监控系统 | In real implementation, metrics should be sent to monitoring system
            $this->logger->debug('Performance metrics sent', [
                'count' => count($this->performanceMetrics)
            ]);
            
            // 清空指标列表 | Clear metrics list
            $this->performanceMetrics = [];
        } catch (\Exception $e) {
            $this->logger->error('Failed to send performance metrics', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 初始化告警配置 | Initialize alerting configuration
    private function initAlertingConfig() {
        try {
            $config = $this->container->get('config') ?? [];
            $this->alertingConfig = array_merge(
                $this->alertingConfig,
                $config['alerting'] ?? []
            );
        } catch (\Exception $e) {
            $this->logger->warning('Failed to load alerting config, using defaults', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 触发告警 | Trigger alert
    // @param string $alertName 告警名称 | Alert name
    // @param array $data 告警数据 | Alert data
    private function triggerAlert($alertName, $data = []) {
        // 初始化配置 | Initialize config if not already done
        if (empty($this->alertingConfig['enabled'])) {
            $this->initAlertingConfig();
        }
        
        if (!$this->alertingConfig['enabled']) {
            return;
        }
        
        $alert = [
            'name' => $alertName,
            'data' => array_merge($data, $this->tracingContext),
            'timestamp' => time(),
            'severity' => $data['severity'] ?? 'warning'
        ];
        
        $this->logger->warning('Alert triggered', $alert);
        
        // 在实际实现中，这里应该将告警发送到告警系统 | In real implementation, alerts should be sent to alerting system
    }
    
    // 初始化插件系统 | Initialize plugin system
    private function initPluginSystem() {
        try {
            $config = $this->container->get('config');
            $plugins = [];
            if (method_exists($config, 'get')) {
                $plugins = $config->get('plugins') ?? [];
            } else {
                $plugins = $config['plugins'] ?? [];
            }
            
            foreach ($plugins as $pluginName => $pluginConfig) {
                if ($pluginConfig['enabled']) {
                    $this->loadPlugin($pluginName, $pluginConfig);
                }
            }
        } catch (\Exception $e) {
            $this->logger->warning('Failed to initialize plugin system', [
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 加载插件 | Load plugin
    // @param string $pluginName 插件名称 | Plugin name
    // @param array $pluginConfig 插件配置 | Plugin configuration
    private function loadPlugin($pluginName, $pluginConfig) {
        try {
            // 构建插件类名 | Build plugin class name
            $pluginClass = 'InkClock\\Plugin\\' . ucfirst($pluginName) . 'Plugin';
            
            if (class_exists($pluginClass)) {
                $plugin = new $pluginClass($pluginConfig, $this->container);
                $this->plugins[$pluginName] = $plugin;
                
                $this->logger->info('Plugin loaded successfully', [
                    'plugin' => $pluginName
                ]);
            } else {
                $this->logger->warning('Plugin class not found', [
                    'plugin' => $pluginName,
                    'class' => $pluginClass
                ]);
            }
        } catch (\Exception $e) {
            $this->logger->error('Failed to load plugin', [
                'plugin' => $pluginName,
                'error' => $e->getMessage()
            ]);
        }
    }
    
    // 执行插件钩子 | Execute plugin hook
    // @param string $hookName 钩子名称 | Hook name
    // @param array $data 钩子数据 | Hook data
    // @return array 修改后的数据 | Modified data
    private function executePluginHook($hookName, $data = []) {
        foreach ($this->plugins as $pluginName => $plugin) {
            if (method_exists($plugin, $hookName)) {
                try {
                    $data = $plugin->$hookName($data);
                } catch (\Exception $e) {
                    $this->logger->error('Plugin hook execution failed', [
                        'plugin' => $pluginName,
                        'hook' => $hookName,
                        'error' => $e->getMessage()
                    ]);
                }
            }
        }
        
        return $data;
    }
    
    // 初始化配置管理 | Initialize configuration management
    private function initConfig() {
        try {
            // 从容器获取基础配置 | Get base config from container
            $baseConfig = $this->container->get('config');
            
            // 确保baseConfig是数组 | Ensure baseConfig is an array
            $baseConfigArray = [];
            if (method_exists($baseConfig, 'toArray')) {
                $baseConfigArray = $baseConfig->toArray();
            } elseif (is_object($baseConfig)) {
                $baseConfigArray = (array) $baseConfig;
            } else {
                $baseConfigArray = $baseConfig ?? [];
            }
            
            // 合并默认配置 | Merge with default config
            $this->config = array_merge(
                $this->getDefaultConfig(),
                $baseConfigArray
            );
            
            // 加载动态配置 | Load dynamic config
            $this->loadDynamicConfig();
        } catch (\Exception $e) {
            $this->logger->warning('Failed to initialize config, using defaults', [
                'error' => $e->getMessage()
            ]);
            $this->config = $this->getDefaultConfig();
        }
    }
    
    // 获取默认配置 | Get default configuration
    // @return array 默认配置 | Default configuration
    private function getDefaultConfig() {
        return [
            'cors' => [
                'enabled' => true,
                'allow_origins' => ['*'],
                'allow_methods' => ['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS'],
                'allow_headers' => ['Content-Type', 'Authorization', 'X-API-Key', 'X-Requested-With', 'X-Trace-ID', 'X-Span-ID'],
                'max_age' => 3600
            ],
            'jwt' => [
                'secret' => 'default_jwt_secret_key',
                'expiration' => 3600,
                'algorithm' => 'HS256'
            ],
            'rate_limiting' => [
                'enabled' => true,
                'window' => 60,
                'limit' => 100
            ],
            'load_balancing' => [
                'algorithm' => 'round_robin',
                'health_check_interval' => 30,
                'services' => []
            ],
            'circuit_breaker' => [
                'enabled' => true,
                'failure_threshold' => 5,
                'recovery_timeout' => 30,
                'half_open_max_requests' => 3
            ],
            'session' => [
                'enabled' => true,
                'expiration' => 3600,
                'cleanup_interval' => 3600
            ],
            'tracing' => [
                'enabled' => true,
                'sampling_rate' => 0.1
            ],
            'performance_monitoring' => [
                'enabled' => true,
                'batch_size' => 100
            ],
            'alerting' => [
                'enabled' => true,
                'thresholds' => []
            ],
            'plugins' => []
        ];
    }
    
    // 加载动态配置 | Load dynamic configuration
    private function loadDynamicConfig() {
        // 在实际实现中，这里应该从外部存储加载动态配置 | In real implementation, dynamic config should be loaded from external storage
        // 例如：配置中心、数据库、Redis等 | For example: config center, database, Redis, etc.
    }
    
    // 获取配置项 | Get configuration item
    // @param string $key 配置键 | Config key
    // @param mixed $default 默认值 | Default value
    // @return mixed 配置值 | Config value
    private function getConfig($key, $default = null) {
        $keys = explode('.', $key);
        $value = $this->config;
        
        foreach ($keys as $k) {
            if (!isset($value[$k])) {
                return $default;
            }
            $value = $value[$k];
        }
        
        return $value;
    }
    
    // 更新配置项 | Update configuration item
    // @param string $key 配置键 | Config key
    // @param mixed $value 配置值 | Config value
    // @return bool 是否更新成功 | Whether update was successful
    private function updateConfig($key, $value) {
        $keys = explode('.', $key);
        $config = &$this->config;
        
        for ($i = 0; $i < count($keys) - 1; $i++) {
            $k = $keys[$i];
            if (!isset($config[$k])) {
                $config[$k] = [];
            }
            $config = &$config[$k];
        }
        
        $lastKey = end($keys);
        $config[$lastKey] = $value;
        
        return true;
    }
    
    // 动态添加路由 | Dynamically add route
    // @param string $path 路由路径 | Route path
    // @param array $target 路由目标 | Route target
    // @return bool 是否添加成功 | Whether addition was successful
    public function addRoute($path, $target) {
        try {
            // 静态变量存储路径映射 | Static variable to store path mappings
            static $pathMap = null;
            
            if ($pathMap === null) {
                // 重新初始化路径映射 | Re-initialize path mappings
                $this->getForwardTarget('/', 'GET'); // 触发初始化 | Trigger initialization
            }
            
            $pathMap[$path] = $target;
            
            $this->logger->info('Route added dynamically', [
                'path' => $path,
                'target' => $target
            ]);
            
            return true;
        } catch (\Exception $e) {
            $this->logger->error('Failed to add route dynamically', [
                'path' => $path,
                'error' => $e->getMessage()
            ]);
            return false;
        }
    }
    
    // 动态移除路由 | Dynamically remove route
    // @param string $path 路由路径 | Route path
    // @return bool 是否移除成功 | Whether removal was successful
    public function removeRoute($path) {
        try {
            // 静态变量存储路径映射 | Static variable to store path mappings
            static $pathMap = null;
            
            if ($pathMap === null) {
                // 重新初始化路径映射 | Re-initialize path mappings
                $this->getForwardTarget('/', 'GET'); // 触发初始化 | Trigger initialization
            }
            
            if (isset($pathMap[$path])) {
                unset($pathMap[$path]);
                
                $this->logger->info('Route removed dynamically', [
                    'path' => $path
                ]);
                
                return true;
            }
            
            return false;
        } catch (\Exception $e) {
            $this->logger->error('Failed to remove route dynamically', [
                'path' => $path,
                'error' => $e->getMessage()
            ]);
            return false;
        }
    }
    
    // 获取追踪头 | Get tracing headers
    // @return array 追踪头 | Tracing headers
    private function getTracingHeaders() {
        $headers = [];
        
        if (!empty($this->tracingContext['trace_id'])) {
            $headers['X-Trace-ID'] = $this->tracingContext['trace_id'];
        }
        
        if (!empty($this->tracingContext['span_id'])) {
            $headers['X-Span-ID'] = $this->tracingContext['span_id'];
        }
        
        if (!empty($this->tracingContext['parent_span_id'])) {
            $headers['X-Parent-Span-ID'] = $this->tracingContext['parent_span_id'];
        }
        
        if (!empty($this->tracingContext['sampled'])) {
            $headers['X-Sampled'] = $this->tracingContext['sampled'];
        }
        
        return $headers;
    }
    
    // 开始新的 span | Start new span
    // @param string $name Span 名称 | Span name
    // @return array 新的 span 信息 | New span information
    public function startSpan($name) {
        if (empty($this->tracingContext)) {
            return [];
        }
        
        $newSpanId = $this->generateSpanId();
        
        $span = [
            'name' => $name,
            'trace_id' => $this->tracingContext['trace_id'],
            'span_id' => $newSpanId,
            'parent_span_id' => $this->tracingContext['span_id'],
            'start_time' => microtime(true),
            'attributes' => []
        ];
        
        $this->logger->debug('Started span', $span);
        
        return $span;
    }
    
    // 结束 span | End span
    // @param array $span Span 信息 | Span information
    public function endSpan($span) {
        if (empty($this->tracingContext) || empty($span)) {
            return;
        }
        
        $span['end_time'] = microtime(true);
        $span['duration'] = round($span['end_time'] - $span['start_time'], 4);
        
        $this->logger->debug('Ended span', $span);
    }
    
    // 发送响应 | Send response
    // @param array $response 响应数据 | Response data
    protected function sendResponse($response) {
        // 设置HTTP状态码 | Set HTTP status code
        http_response_code($response['status']);
        
        // 添加 CORS 头 | Add CORS headers
        $corsHeaders = [
            'Access-Control-Allow-Origin' => '*',
            'Access-Control-Allow-Methods' => 'GET, POST, PUT, DELETE, PATCH, OPTIONS',
            'Access-Control-Allow-Headers' => 'Content-Type, Authorization, X-API-Key, X-Requested-With, X-Trace-ID, X-Span-ID',
            'Access-Control-Max-Age' => '3600'
        ];
        
        // 添加追踪头 | Add tracing headers
        $tracingHeaders = $this->getTracingHeaders();
        
        // 合并所有头 | Merge all headers
        $response['headers'] = array_merge($corsHeaders, $tracingHeaders, $response['headers']);
        
        // 处理 OPTIONS 请求 | Handle OPTIONS requests
        if ($_SERVER['REQUEST_METHOD'] ?? '' === 'OPTIONS') {
            $response['status'] = 204;
            $response['body'] = '';
        }
        
        // 设置响应头 | Set response headers
        foreach ($response['headers'] as $name => $value) {
            header($name . ': ' . $value);
        }
        
        // 输出响应体 | Output response body
        echo $response['body'];
        
        // 终止脚本执行 | Terminate script execution
        exit;
    }
}