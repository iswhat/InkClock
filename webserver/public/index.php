<?php
/**
 * InkClock Web Server - 新公共入口文件
 */

// 设置默认时区
date_default_timezone_set('Asia/Shanghai');

// 错误报告设置
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 移除手动加载DIContainer.php的代码，使用统一的自动加载机制

// 自定义自动加载函数
spl_autoload_register(function ($className) {
    // 定义命名空间映射
    $namespaceMap = [
        'InkClock\\' => __DIR__ . '/../src/'
    ];
    
    // 遍历命名空间映射
    foreach ($namespaceMap as $prefix => $baseDir) {
        // 检查类是否使用了该命名空间前缀
        if (strpos($className, $prefix) === 0) {
            // 移除命名空间前缀
            $relativeClass = substr($className, strlen($prefix));
            
            // 转换命名空间分隔符为目录分隔符
            $filePath = $baseDir . str_replace('\\', '/', $relativeClass) . '.php';
            
            // 特殊处理：检查Interface/Interfaces目录
            if (!file_exists($filePath)) {
                // 尝试Interface目录
                $interfaceFilePath = $baseDir . 'Interface/' . basename($filePath);
                if (file_exists($interfaceFilePath)) {
                    $filePath = $interfaceFilePath;
                }
            }
            
            // 如果文件存在，加载它
            if (file_exists($filePath)) {
                require_once $filePath;
                return;
            }
            
            // 尝试大小写变体
            $filePathLower = $baseDir . str_replace('\\', '/', strtolower($relativeClass)) . '.php';
            if (file_exists($filePathLower)) {
                require_once $filePathLower;
                return;
            }
        }
    }
});

// 初始化配置（使用Config类加载，避免重复加载）
// 移除手动加载config.php的代码，由Config类自动加载

// 初始化依赖注入容器
use InkClock\Utils\DIContainer;
use InkClock\Config\Services;

// 创建依赖注入容器实例
$container = DIContainer::getInstance();

// 注册所有服务
Services::register($container);

// 初始化中间件管理器
use InkClock\Middleware\MiddlewareManager;
use InkClock\Middleware\CorsMiddleware;
use InkClock\Middleware\LoggingMiddleware;
use InkClock\Middleware\RateLimitMiddleware;

$middlewareManager = new MiddlewareManager();

// 获取服务实例
$logger = $container->get('logger');
$response = $container->get('response');
$cache = $container->get('cache');

// 添加全局中间件
$middlewareManager
    ->add(new CorsMiddleware($response))
    ->add(new LoggingMiddleware($logger))
    ->add(new RateLimitMiddleware($logger, $response, $cache));

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

// 处理根路径请求
if (empty($request['path']) || $request['path'] === '/') {
    $indexFile = __DIR__ . '/index.html';
    if (file_exists($indexFile)) {
        header('Content-Type: text/html');
        readfile($indexFile);
        exit;
    }
}

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

// 调试信息
$logger->info('路由匹配开始', [
    'method' => $request['method'],
    'path' => $request['path']
]);

// 打印所有路由，看看顺序
$logger->info('所有路由', [
    'routes' => array_keys($routes)
]);

foreach ($routes as $routePattern => $handler) {
    list($routeMethod, $routePath) = explode(' ', $routePattern, 2);
    
    if ($routeMethod !== $request['method']) {
        continue;
    }
    
    // 转换路由模式为正则表达式
    $pattern = preg_replace('/\{([^}]+)\}/', '(?P<\1>[^/]+)', $routePath);
    $pattern = str_replace('/', '\/', $pattern);
    $pattern = '/^' . $pattern . '$/';
    
    // 调试信息
    $logger->info('尝试匹配路由', [
        'route_pattern' => $routePattern,
        'route_path' => $routePath,
        'pattern' => $pattern,
        'request_path' => $request['path']
    ]);
    
    if (preg_match($pattern, $request['path'], $matches)) {
        $logger->info('路由匹配成功', [
            'route_pattern' => $routePattern,
            'handler' => $handler,
            'matches' => $matches
        ]);
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
    $fullControllerName = 'InkClock\\Controller\\' . $controllerName;
    
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