<?php
/**
 * 请求验证中间件
 */

namespace InkClock\Middleware;

use InkClock\Utils\Logger;
use InkClock\Utils\Response;

class RequestValidationMiddleware implements MiddlewareInterface {
    private $logger;
    private $response;
    private $validationRules = [];
    
    /**
     * 构造函数
     * @param \InkClock\Utils\Logger $logger 日志服务
     * @param \InkClock\Utils\Response $response 响应服务
     */
    public function __construct($logger = null, $response = null) {
        if ($logger === null) {
            $logger = Logger::getInstance();
        }
        if ($response === null) {
            $response = Response::getInstance();
        }
        $this->logger = $logger;
        $this->response = $response;
        
        // 初始化验证规则
        $this->initValidationRules();
    }
    
    /**
     * 初始化验证规则
     */
    private function initValidationRules() {
        // 配置请求验证规则
        // 格式：[路径模式 => [方法 => [字段规则]]]
        $this->validationRules = [
            // 用户相关API
            'api/user/register' => [
                'POST' => [
                    'username' => ['required', 'string', 'min:3', 'max:50'],
                    'email' => ['required', 'email'],
                    'password' => ['required', 'string', 'min:6']
                ]
            ],
            'api/user/login' => [
                'POST' => [
                    'username' => ['required', 'string'],
                    'password' => ['required', 'string']
                ]
            ],
            'api/user/bind' => [
                'POST' => [
                    'device_id' => ['required', 'string'],
                    'nickname' => ['optional', 'string', 'max:100']
                ]
            ],
            'api/user/unbind' => [
                'POST' => [
                    'device_id' => ['required', 'string']
                ]
            ],
            'api/user/device' => [
                'PUT' => [
                    'device_id' => ['required', 'string'],
                    'nickname' => ['required', 'string', 'max:100']
                ]
            ],
            
            // 设备相关API
            'api/device' => [
                'POST' => [
                    'device_id' => ['required', 'string'],
                    'nickname' => ['optional', 'string', 'max:100'],
                    'model' => ['optional', 'string', 'max:50'],
                    'firmware_version' => ['optional', 'string', 'max:20']
                ],
                'GET' => [
                    'id' => ['optional', 'integer']
                ]
            ],
            
            // 消息相关API
            'api/message' => [
                'POST' => [
                    'device_id' => ['required', 'string'],
                    'content' => ['required', 'string', 'max:2000'],
                    'type' => ['optional', 'string', 'in:text,image,audio'],
                    'sender' => ['optional', 'string', 'max:50']
                ]
            ],
            'api/message/\{deviceId\}' => [
                'GET' => [
                    'page' => ['optional', 'integer', 'min:1'],
                    'limit' => ['optional', 'integer', 'min:1', 'max:100']
                ]
            ],
            'api/message/\{deviceId\}/read' => [
                'PUT' => [
                    'message_ids' => ['optional', 'array']
                ]
            ],
            
            // 固件相关API
            'api/firmware' => [
                'POST' => [
                    'model' => ['required', 'string', 'max:50'],
                    'version' => ['required', 'string', 'max:20'],
                    'description' => ['optional', 'string', 'max:500'],
                    'is_active' => ['optional', 'boolean']
                ]
            ],
            
            // 设备分组相关API
            'api/group' => [
                'POST' => [
                    'name' => ['required', 'string', 'max:50'],
                    'description' => ['optional', 'string', 'max:200']
                ]
            ],
            'api/group/add_device' => [
                'POST' => [
                    'group_id' => ['required', 'integer'],
                    'device_id' => ['required', 'string']
                ]
            ],
            
            // 设备标签相关API
            'api/tag' => [
                'POST' => [
                    'name' => ['required', 'string', 'max:50'],
                    'color' => ['optional', 'string', 'regex:/^#[0-9A-Fa-f]{6}$/']
                ]
            ],
            
            // 固件推送任务相关API
            'api/push_task' => [
                'POST' => [
                    'firmware_id' => ['required', 'integer'],
                    'device_ids' => ['required', 'array'],
                    'scheduled_time' => ['optional', 'datetime']
                ]
            ],
            
            // 消息模板相关API
            'api/template' => [
                'POST' => [
                    'name' => ['required', 'string', 'max:50'],
                    'content' => ['required', 'string', 'max:1000'],
                    'type' => ['required', 'string', 'in:text,image,audio']
                ]
            ]
        ];
    }
    
    /**
     * 处理请求
     * @param array $request 请求信息
     * @param mixed $next 下一个中间件或处理函数
     * @return mixed 响应结果
     */
    public function handle($request, $next) {
        $this->logger->info('请求验证中间件执行', [
            'path' => $request['path'] ?? '',
            'method' => $request['method'] ?? ''
        ]);
        
        // 获取请求路径和方法
        $path = $request['path'] ?? '';
        $method = strtoupper($request['method'] ?? 'GET');
        
        // 查找匹配的验证规则
        $rule = $this->findMatchingValidationRule($path, $method);
        
        // 如果没有找到匹配的规则，直接放行
        if (!$rule) {
            return $next($request);
        }
        
        // 获取请求数据
        $requestData = $request['body'] ?? [];
        
        // 验证请求数据
        $errors = $this->validateRequestData($requestData, $rule);
        
        // 如果验证失败，返回错误响应
        if (!empty($errors)) {
            $this->logger->warning('请求验证失败', ['errors' => $errors, 'data' => $requestData]);
            
            // 格式化错误信息为字符串
            $errorMessages = [];
            foreach ($errors as $fieldErrors) {
                $errorMessages = array_merge($errorMessages, $fieldErrors);
            }
            
            return $this->response->error(
                '请求参数验证失败',
                400,
                'VALIDATION_ERROR',
                $errorMessages
            );
        }
        
        // 验证通过，继续执行下一个中间件
        return $next($request);
    }
    
    /**
     * 查找匹配的验证规则
     * @param string $path 请求路径
     * @param string $method 请求方法
     * @return array|null 匹配的验证规则
     */
    private function findMatchingValidationRule($path, $method) {
        // 精确匹配
        if (isset($this->validationRules[$path][$method])) {
            return $this->validationRules[$path][$method];
        }
        
        // 模式匹配
        foreach ($this->validationRules as $pattern => $methods) {
            // 将模式转换为正则表达式
            $regex = str_replace('*', '.*', $pattern);
            $regex = '/^' . $regex . '$/';
            
            if (preg_match($regex, $path) && isset($methods[$method])) {
                return $methods[$method];
            }
        }
        
        return null;
    }
    
    /**
     * 验证请求数据
     * @param array $data 请求数据
     * @param array $rules 验证规则
     * @return array 错误信息数组
     */
    private function validateRequestData($data, $rules) {
        $errors = [];
        
        foreach ($rules as $field => $fieldRules) {
            // 支持嵌套字段（如 user.name）
            $value = $this->getNestedValue($data, $field);
            
            foreach ($fieldRules as $rule) {
                // 解析规则（支持带参数的规则，如max:100）
                $ruleName = $rule;
                $ruleParams = [];
                
                if (strpos($rule, ':') !== false) {
                    list($ruleName, $ruleParamsStr) = explode(':', $rule, 2);
                    $ruleParams = explode(',', $ruleParamsStr);
                }
                
                // 执行验证
                $error = $this->validateField($field, $value, $ruleName, $ruleParams);
                if ($error) {
                    $errors[$field][] = $error;
                    // 对于同一个字段，只需记录第一个错误
                    break;
                }
            }
        }
        
        return $errors;
    }
    
    /**
     * 获取嵌套字段的值
     * @param array $data 数据数组
     * @param string $field 字段名（支持嵌套，如 user.name）
     * @return mixed 字段值
     */
    private function getNestedValue($data, $field) {
        $keys = explode('.', $field);
        $value = $data;
        
        foreach ($keys as $key) {
            if (!is_array($value) || !isset($value[$key])) {
                return null;
            }
            $value = $value[$key];
        }
        
        return $value;
    }
    
    /**
     * 验证单个字段
     * @param string $field 字段名
     * @param mixed $value 字段值
     * @param string $rule 规则名
     * @param array $params 规则参数
     * @return string|null 错误信息
     */
    private function validateField($field, $value, $rule, $params) {
        // 可选字段处理
        if ($rule === 'optional' && $value === null) {
            return null;
        }
        
        // 必填字段检查
        if ($rule === 'required' && ($value === null || $value === '')) {
            return "{$field} 不能为空";
        }
        
        // 如果字段值为null或空字符串，且不是必填字段，跳过后续验证
        if (($value === null || $value === '') && $rule !== 'required') {
            return null;
        }
        
        // 字符串类型检查
        if ($rule === 'string' && !is_string($value)) {
            return "{$field} 必须是字符串";
        }
        
        // 数组类型检查
        if ($rule === 'array' && !is_array($value)) {
            return "{$field} 必须是数组";
        }
        
        // 数值类型检查
        if ($rule === 'numeric' && !is_numeric($value)) {
            return "{$field} 必须是数值";
        }
        
        // 整数类型检查
        if ($rule === 'integer' && !filter_var($value, FILTER_VALIDATE_INT)) {
            return "{$field} 必须是整数";
        }
        
        // 布尔类型检查
        if ($rule === 'boolean' && !is_bool($value) && !in_array($value, [0, 1, '0', '1'], true)) {
            return "{$field} 必须是布尔值";
        }
        
        // 最小长度检查
        if ($rule === 'min' && isset($params[0])) {
            $min = (int)$params[0];
            if (is_string($value) && strlen($value) < $min) {
                return "{$field} 长度不能少于 {$min} 个字符";
            }
            if (is_array($value) && count($value) < $min) {
                return "{$field} 至少需要包含 {$min} 个元素";
            }
            if (is_numeric($value) && $value < $min) {
                return "{$field} 不能小于 {$min}";
            }
        }
        
        // 最大长度检查
        if ($rule === 'max' && isset($params[0])) {
            $max = (int)$params[0];
            if (is_string($value) && strlen($value) > $max) {
                return "{$field} 长度不能超过 {$max} 个字符";
            }
            if (is_array($value) && count($value) > $max) {
                return "{$field} 最多只能包含 {$max} 个元素";
            }
            if (is_numeric($value) && $value > $max) {
                return "{$field} 不能大于 {$max}";
            }
        }
        
        // 枚举值检查
        if ($rule === 'in' && !empty($params)) {
            if (!in_array($value, $params)) {
                return "{$field} 必须是以下值之一: " . implode(', ', $params);
            }
        }
        
        // 邮箱格式检查
        if ($rule === 'email' && !filter_var($value, FILTER_VALIDATE_EMAIL)) {
            return "{$field} 必须是有效的邮箱地址";
        }
        
        // URL格式检查
        if ($rule === 'url' && !filter_var($value, FILTER_VALIDATE_URL)) {
            return "{$field} 必须是有效的URL";
        }
        
        // 正则表达式检查
        if ($rule === 'regex' && isset($params[0])) {
            $pattern = $params[0];
            if (!preg_match($pattern, $value)) {
                return "{$field} 格式不符合要求";
            }
        }
        
        // 日期时间格式检查
        if ($rule === 'datetime') {
            $dateTime = new \DateTime($value);
            if (!$dateTime) {
                return "{$field} 必须是有效的日期时间格式";
            }
        }
        
        return null;
    }
}
