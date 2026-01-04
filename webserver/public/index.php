<?php
/**
 * InkClock Web Server - 新公共入口文件
 */

// 设置默认时区
date_default_timezone_set('Asia/Shanghai');

// 错误报告设置
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 加载Composer自动加载器
require_once __DIR__ . '/../vendor/autoload.php';

// 初始化配置
$config = require __DIR__ . '/../config/config.php';

// 初始化依赖注入容器
use InkClock\Utils\DIContainer;
use InkClock\Utils\Database;
use InkClock\Utils\Logger;
use InkClock\Utils\Response;
use InkClock\Service\AuthService;
use InkClock\Middleware\MiddlewareManager;
use InkClock\Middleware\CorsMiddleware;
use InkClock\Middleware\LoggingMiddleware;
use InkClock\Middleware\AuthMiddleware;

// 创建依赖注入容器实例
$container = DIContainer::getInstance();

// 注册服务到容器

// 1. 日志服务
$logger = new Logger();
$logger->setLevel($config['log']['level']);
$logger->setLogFile($config['log']['file_path']);
$container->set('logger', $logger);

// 2. 数据库服务
$db = Database::getInstance($config['database']['path']);
$container->set('db', $db->getConnection());

// 3. 响应服务
$response = new Response();
$container->set('response', $response);

// 4. 认证服务
$authService = new AuthService($container->get('db'), $container->get('logger'));
$container->set('authService', $authService);

// 5. 初始化中间件管理器
$middlewareManager = new MiddlewareManager();

// 6. 添加全局中间件
$middlewareManager
    ->add(new CorsMiddleware($response))
    ->add(new LoggingMiddleware($logger));

// 7. 获取请求信息
$request = array(
    'method' => $_SERVER['REQUEST_METHOD'],
    'uri' => $_SERVER['REQUEST_URI'],
    'path' => parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH),
    'query' => $_GET,
    'ip' => $_SERVER['REMOTE_ADDR'],
    'headers' => getallheaders(),
    'body' => file_get_contents('php://input')
);

// 移除查询字符串并规范化路径
$request['path'] = rtrim($request['path'], '/');

// 8. 加载路由配置
$routes = require __DIR__ . '/../config/routes.php';

// 9. 匹配路由
$matchedRoute = null;
$params = array();

foreach ($routes as $routePattern => $handler) {
    list($routeMethod, $routePath) = explode(' ', $routePattern, 2);
    
    if ($routeMethod !== $request['method']) {
        continue;
    }
    
    // 转换路由模式为正则表达式
    $pattern = preg_replace('/\{([^}]+)\}/', '(?P<\1>[^/]+)', $routePath);
    $pattern = str_replace('/', '\/', $pattern);
    $pattern = '/^' . $pattern . '$/';
    
    if (preg_match($pattern, $request['path'], $matches)) {
        $matchedRoute = $handler;
        $params = $matches;
        break;
    }
}

// 10. 定义请求处理函数
$requestHandler = function($request) use ($matchedRoute, $params, $container, $logger, $response) {
    if (!$matchedRoute) {
        $logger->info('未匹配到路由', [
            'method' => $request['method'],
            'path' => $request['path']
        ]);
        $response->notFound('API路径不存在');
        return ['status_code' => 404];
    }
    
    list($controllerName, $actionName) = explode('@', $matchedRoute);
    
    // 控制器文件路径
    $controllerFile = __DIR__ . '/../api/' . $controllerName . '.php';
    
    if (!file_exists($controllerFile)) {
        $logger->warning('控制器文件不存在', [
            'controller' => $controllerName,
            'file' => $controllerFile
        ]);
        $response->notFound('控制器不存在');
        return ['status_code' => 404];
    }
    
    require_once $controllerFile;
    
    try {
        // 创建控制器实例，使用依赖注入
        $controller = new $controllerName($container);
        
        // 调用动作方法
        $controller->$actionName($params);
        
        return ['status_code' => 200];
    } catch (Exception $e) {
        $logger->error('控制器执行错误', [
            'exception' => $e->getMessage(),
            'trace' => $e->getTraceAsString(),
            'controller' => $controllerName,
            'action' => $actionName,
            'path' => $request['path']
        ]);
        $response->serverError('服务器内部错误');
        return ['status_code' => 500];
    }
};

// 11. 执行中间件链
$middlewareManager->handle($request, $requestHandler);