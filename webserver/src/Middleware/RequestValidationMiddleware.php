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
     */
    public function __construct() {
        $this->logger = Logger::getInstance();
        $this->response = new Response();
        
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
            // 设备注册API
            'api/devices/register' => [
                'POST' => [
                    'device_id' => ['required', 'string', 'min:10'],
                    'device_name' => ['required', 'string', 'max:100'],
                    'device_type' => ['required', 'string', 'in:inkclock,other'],
                    'device_info' => ['optional', 'array']
                ]
            ],
            // 发送消息API
            'api/messages/send' => [
                'POST' => [
                    'device_id' => ['required', 'string'],
                    'content' => ['required', 'string', 'max:1000'],
                    'type' => ['required', 'string', 'in:text,image,audio']
                ]
            ],
            // 批量发送消息API
            'api/messages/batch' => [
                'POST' => [
                    'device_ids' => ['required', 'array', 'min:1'],
                    'content' => ['required', 'string', 'max:1000'],
                    'type' => ['required', 'string', 'in:text,image,audio']
                ]
            ],
            // 用户注册API
            'api/auth/register' => [
                'POST' => [
                    'username' => ['required', 'string', 'min:3', 'max:50'],
                    'email' => ['required', 'email'],
                    'password' => ['required', 'string', 'min:6']
                ]
            ],
            // 用户登录API
            'api/auth/login' => [
                'POST' => [
                    'username' => ['required', 'string'],
                    'password' => ['required', 'string']
                ]
            ]
        ];
    }
    
    /**
     * 处理中间件
     * @param array $request 请求信息
     * @param callable $next 下一个中间件
     * @return mixed 响应结果
     */
    public function handle($request, callable $next) {
        $this->logger->info('请求验证中间件执行', ['request' => $request]);
        
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
            return $this->response->error(400, 'Invalid request data', ['errors' => $errors]);
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
            foreach ($fieldRules as $rule) {
                // 解析规则（支持带参数的规则，如max:100）
                $ruleName = $rule;
                $ruleParams = [];
                
                if (strpos($rule, ':') !== false) {
                    list($ruleName, $ruleParamsStr) = explode(':', $rule, 2);
                    $ruleParams = explode(',', $ruleParamsStr);
                }
                
                // 执行验证
                $error = $this->validateField($data, $field, $ruleName, $ruleParams);
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
     * 验证单个字段
     * @param array $data 请求数据
     * @param string $field 字段名
     * @param string $rule 规则名
     * @param array $params 规则参数
     * @return string|null 错误信息
     */
    private function validateField($data, $field, $rule, $params) {
        // 检查字段是否存在
        $value = $data[$field] ?? null;
        
        // 可选字段处理
        if ($rule === 'optional' && $value === null) {
            return null;
        }
        
        // 必填字段检查
        if ($rule === 'required' && ($value === null || $value === '')) {
            return "{$field} is required";
        }
        
        // 如果字段值为null或空字符串，且不是必填字段，跳过后续验证
        if (($value === null || $value === '') && $rule !== 'required') {
            return null;
        }
        
        // 字符串类型检查
        if ($rule === 'string' && !is_string($value)) {
            return "{$field} must be a string";
        }
        
        // 数组类型检查
        if ($rule === 'array' && !is_array($value)) {
            return "{$field} must be an array";
        }
        
        // 最小长度检查
        if ($rule === 'min' && isset($params[0])) {
            $min = (int)$params[0];
            if (is_string($value) && strlen($value) < $min) {
                return "{$field} must be at least {$min} characters long";
            }
            if (is_array($value) && count($value) < $min) {
                return "{$field} must have at least {$min} items";
            }
        }
        
        // 最大长度检查
        if ($rule === 'max' && isset($params[0])) {
            $max = (int)$params[0];
            if (is_string($value) && strlen($value) > $max) {
                return "{$field} must not exceed {$max} characters";
            }
        }
        
        // 枚举值检查
        if ($rule === 'in' && !empty($params)) {
            if (!in_array($value, $params)) {
                return "{$field} must be one of: " . implode(', ', $params);
            }
        }
        
        // 邮箱格式检查
        if ($rule === 'email' && !filter_var($value, FILTER_VALIDATE_EMAIL)) {
            return "{$field} must be a valid email address";
        }
        
        return null;
    }
}
