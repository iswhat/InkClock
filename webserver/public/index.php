<?php
/**
 * InkClock Web Server - 新公共入口文件
 */

// 设置默认时区
date_default_timezone_set('Asia/Shanghai');

// 错误报告设置
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 自定义自动加载函数
spl_autoload_register(function ($className) {
    // 转换命名空间为文件路径
    $filePath = __DIR__ . '/../src/' . str_replace('\\', '/', $className) . '.php';
    
    if (file_exists($filePath)) {
        require_once $filePath;
    }
});

// 初始化配置
$config = require __DIR__ . '/../config/config.php';

// 初始化依赖注入容器
use App\Utils\DIContainer;
use App\Config\Services;

// 创建依赖注入容器实例
$container = DIContainer::getInstance();

// 注册所有服务
Services::register($container);

// 初始化中间件管理器
use App\Middleware\MiddlewareManager;
use App\Middleware\CorsMiddleware;
use App\Middleware\LoggingMiddleware;

$middlewareManager = new MiddlewareManager();

// 获取服务实例
$logger = $container->get('logger');
$response = $container->get('response');

// 添加全局中间件
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

// 处理静态资源请求
$staticExtensions = array('.html', '.css', '.js', '.json', '.png', '.jpg', '.jpeg', '.gif', '.ico', '.svg', '.pdf', '.txt');
$pathInfo = pathinfo($request['path']);
$extension = isset($pathInfo['extension']) ? '.' . $pathInfo['extension'] : '';

// 静态资源目录
$staticDir = __DIR__;

// 如果是静态资源请求，直接返回文件
if (in_array($extension, $staticExtensions)) {
    $filePath = $staticDir . $request['path'];
    
    // 确保文件存在且在允许的目录内
    if (file_exists($filePath) && strpos(realpath($filePath), realpath($staticDir)) === 0) {
        // 设置适当的Content-Type
        $mimeTypes = array(
            '.html' => 'text/html',
            '.css' => 'text/css',
            '.js' => 'application/javascript',
            '.json' => 'application/json',
            '.png' => 'image/png',
            '.jpg' => 'image/jpeg',
            '.jpeg' => 'image/jpeg',
            '.gif' => 'image/gif',
            '.ico' => 'image/x-icon',
            '.svg' => 'image/svg+xml',
            '.pdf' => 'application/pdf',
            '.txt' => 'text/plain'
        );
        
        header('Content-Type: ' . ($mimeTypes[$extension] ?? 'application/octet-stream'));
        header('Cache-Control: max-age=3600');
        readfile($filePath);
        exit;
    } else {
        // 文件不存在，返回404
        header('HTTP/1.1 404 Not Found');
        echo '404 Not Found';
        exit;
    }
}

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
    
    // 使用新的控制器命名空间
    $fullControllerName = 'App\\Controller\\' . $controllerName;
    
    try {
        // 创建控制器实例，使用依赖注入
        $controller = new $fullControllerName($container);
        
        // 调用动作方法
        $controller->$actionName($params);
        
        return ['status_code' => 200];
    } catch (Exception $e) {
        $logger->error('控制器执行错误', [
            'exception' => $e->getMessage(),
            'trace' => $e->getTraceAsString(),
            'controller' => $fullControllerName,
            'action' => $actionName,
            'path' => $request['path']
        ]);
        $response->serverError('服务器内部错误');
        return ['status_code' => 500];
    }
};

// 11. 执行中间件链
$middlewareManager->handle($request, $requestHandler);