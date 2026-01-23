<?php
/**
 * 应用入口文件
 */

// 错误报告设置
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 自定义自动加载函数 - PSR-4标准实现
spl_autoload_register(function ($className) {
    // 定义命名空间映射
    $namespaceMap = [
        'InkClock\\' => __DIR__ . '/src/'
    ];
    
    // 遍历命名空间映射
    foreach ($namespaceMap as $prefix => $baseDir) {
        // 检查类是否使用了该命名空间前缀
        if (strpos($className, $prefix) === 0) {
            // 移除命名空间前缀
            $relativeClass = substr($className, strlen($prefix));
            
            // 转换命名空间分隔符为目录分隔符
            $filePath = $baseDir . str_replace('\\', '/', $relativeClass) . '.php';
            
            // 如果文件存在，加载它
            if (file_exists($filePath)) {
                require_once $filePath;
                return;
            }
        }
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

// 注册服务层服务
use InkClock\Config\Services;
Services::register($container);

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

// 优化1: 预编译路由，按HTTP方法分组
static $compiledRoutes = [];
if (empty($compiledRoutes)) {
    foreach ($routes as $routePattern => $handler) {
        list($routeMethod, $routePath) = explode(' ', $routePattern, 2);
        
        // 转换路由模式为正则表达式
        $pattern = preg_replace('/\{([^}]+)\}/', '(?P<\1>[^/]+)', $routePath);
        $pattern = str_replace('/', '\/', $pattern);
        $pattern = '/^' . $pattern . '$/';
        
        // 按HTTP方法分组
        if (!isset($compiledRoutes[$routeMethod])) {
            $compiledRoutes[$routeMethod] = [];
        }
        $compiledRoutes[$routeMethod][] = [
            'pattern' => $pattern,
            'handler' => $handler
        ];
    }
}

// 匹配路由
$matchedRoute = null;
$params = array();

// 优化2: 只检查当前请求方法的路由
if (isset($compiledRoutes[$method])) {
    foreach ($compiledRoutes[$method] as $route) {
        if (preg_match($route['pattern'], $path, $matches)) {
            $matchedRoute = $route['handler'];
            $params = $matches;
            break;
        }
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
    $fullClassName = "\\InkClock\\Controller\\$controllerName";
    
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
    
    // 创建中间件管理器实例
    $middlewareManager = new \InkClock\Middleware\MiddlewareManager($container);
    
    // 注册命名中间件
    $middlewareManager->register('csrf', '\InkClock\Middleware\CsrfMiddleware')
                     ->register('rate_limit', '\InkClock\Middleware\RateLimitMiddleware')
                     ->register('validation', '\InkClock\Middleware\RequestValidationMiddleware');
    
    // 定义中间件组
    $middlewareManager->group('api', array('csrf', 'rate_limit', 'validation'));
    
    // 使用中间件组
    $middlewareManager->use('api');
    
    // 执行中间件链
    $middlewareManager->handle($request, $controllerHandler);
} else {
    $logger->info('未匹配到路由', array(
        'method' => $method,
        'path' => $path
    ));
    $response->notFound('API路径不存在');
}
?>