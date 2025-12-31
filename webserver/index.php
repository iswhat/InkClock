<?php
/**
 * 应用入口文件
 */

// 错误报告设置
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 加载核心工具类
require_once __DIR__ . '/utils/Database.php';
require_once __DIR__ . '/utils/Response.php';
require_once __DIR__ . '/utils/Logger.php';

// 加载配置文件
require_once __DIR__ . '/config/config.php';

// 初始化日志
$logger = Logger::getInstance();
$logger->setLevel($config['log']['level']);
$logger->setLogFile($config['log']['file_path']);

// 处理CORS
Response::handleCORS();

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

// 处理请求
if ($matchedRoute) {
    list($controllerName, $actionName) = explode('@', $matchedRoute);
    
    // 加载控制器文件
    $controllerFile = __DIR__ . '/api/' . $controllerName . '.php';
    if (file_exists($controllerFile)) {
        require_once $controllerFile;
        
        try {
            // 创建控制器实例
            $controller = new $controllerName();
            
            // 调用动作方法
            $controller->$actionName($params);
        } catch (Exception $e) {
            $logger->error('控制器执行错误', array(
                'exception' => $e->getMessage(),
                'controller' => $controllerName,
                'action' => $actionName,
                'path' => $path
            ));
            Response::serverError('服务器内部错误');
        }
    } else {
        $logger->warning('控制器文件不存在', array(
            'controller' => $controllerName,
            'file' => $controllerFile
        ));
        Response::notFound('控制器不存在');
    }
} else {
    $logger->info('未匹配到路由', array(
        'method' => $method,
        'path' => $path
    ));
    Response::notFound('API路径不存在');
}
?>