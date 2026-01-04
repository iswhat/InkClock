<?php
/**
 * 应用入口文件
 */

// 错误报告设置
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 自定义自动加载函数
spl_autoload_register(function ($className) {
    // 转换命名空间为文件路径
    $filePath = __DIR__ . '/src/' . str_replace('\\', '/', $className) . '.php';
    
    // 特殊处理：检查是否在api目录下
    if (!file_exists($filePath)) {
        $filePath = __DIR__ . '/' . str_replace('\\', '/', $className) . '.php';
    }
    
    if (file_exists($filePath)) {
        require_once $filePath;
    }
});

// 加载配置文件
require_once __DIR__ . '/config/config.php';

// 初始化依赖注入容器
use InkClock\Utils\DIContainer;
use InkClock\Utils\Database;
use InkClock\Utils\Response;
use InkClock\Utils\Logger;
use InkClock\Utils\Cache;

// 创建DI容器实例
$container = DIContainer::getInstance();

// 注册配置
$container->set('config', $config);

// 初始化日志
$logger = Logger::getInstance();
$logger->setLevel($config['log']['level']);
$container->set('logger', $logger);

// 初始化数据库
$db = new Database($config['database'], $logger);
$container->set('db', $db);

// 初始化缓存
$cache = new Cache($config['cache']['dir'], $config['cache']['expire']);
$container->set('cache', $cache);

// 初始化响应
$response = Response::getInstance();
$response->setLogger($logger);
$container->set('response', $response);

// 处理CORS
$response->handleCORS();

// 获取请求信息
$method = $_SERVER['REQUEST_METHOD'];
$path = $_SERVER['REQUEST_URI'];

// 移除查询字符串
$path = parse_url($path, PHP_URL_PATH);
$path = rtrim($path, '/');

// 加载路由配置
$routes = require __DIR__ . '/config/routes.php';

// 匹配路由
$matchedRoute = null;
$params = array();

foreach ($routes as $routePattern => $handler) {
    list($routeMethod, $routePath) = explode(' ', $routePattern, 2);
    
    if ($routeMethod !== $method) {
        continue;
    }
    
    // 转换路由模式为正则表达式
    $pattern = preg_replace('/\{([^}]+)\}/', '(?P<\1>[^/]+)', $routePath);
    $pattern = str_replace('/', '\/', $pattern);
    $pattern = '/^' . $pattern . '$/';
    
    if (preg_match($pattern, $path, $matches)) {
        $matchedRoute = $handler;
        $params = $matches;
        break;
    }
}

// 构建请求对象
$request = array(
    'method' => $method,
    'path' => $path,
    'params' => $params,
    'headers' => getallheaders(),
    'query' => $_GET,
    'body' => json_decode(file_get_contents('php://input'), true),
    'ip' => $_SERVER['REMOTE_ADDR']
);

// 处理请求
if ($matchedRoute) {
    list($controllerName, $actionName) = explode('@', $matchedRoute);
    
    // 完整类名
    $fullClassName = "\InkClock\Controller\$controllerName";
    
    // 定义控制器处理函数
    $controllerHandler = function($request) use ($fullClassName, $actionName, $container, $logger, $response) {
        try {
            // 创建控制器实例，传入DI容器
            $controller = new $fullClassName($container);
            
            // 调用动作方法
            $controller->$actionName($request['params']);
            return true;
        } catch (Exception $e) {
            $logger->error('控制器执行错误', array(
                'exception' => $e->getMessage(),
                'exception_trace' => $e->getTraceAsString(),
                'controller' => $fullClassName,
                'action' => $actionName,
                'path' => $request['path']
            ));
            $response->serverError('服务器内部错误');
            return false;
        }
    };
    
    // 设置中间件堆栈
    $middlewareStack = array(
        '\InkClock\Middleware\CsrfMiddleware',
        '\InkClock\Middleware\RateLimitMiddleware',
        '\InkClock\Middleware\RequestValidationMiddleware'
    );
    
    // 构建中间件管道
    $pipeline = $controllerHandler;
    
    // 从后往前构建中间件链
    foreach (array_reverse($middlewareStack) as $middlewareClass) {
        $currentPipeline = $pipeline;
        $pipeline = function($request) use ($middlewareClass, $currentPipeline, $container) {
            // 根据中间件类型注入不同的依赖
            if ($middlewareClass === '\InkClock\Middleware\RateLimitMiddleware') {
                // RateLimitMiddleware 需要额外的 cache 依赖
                $middleware = new $middlewareClass(
                    $container->get('logger'),
                    $container->get('response'),
                    $container->get('cache')
                );
            } else {
                // 其他中间件使用基本依赖
                $middleware = new $middlewareClass(
                    $container->get('logger'),
                    $container->get('response')
                );
            }
            return $middleware->handle($request, $currentPipeline);
        };
    }
    
    // 执行中间件管道
    $pipeline($request);
} else {
    $logger->info('未匹配到路由', array(
        'method' => $method,
        'path' => $path
    ));
    $response->notFound('API路径不存在');
}
?>