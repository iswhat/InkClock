<?php
/**
 * 服务注册配置
 */

namespace App\Config;

use App\Utils\DIContainer;
use App\Utils\Database;
use App\Utils\Logger;
use App\Utils\Cache;
use App\Utils\Response;
use App\Config\Config;
use App\Service\AuthService;
use App\Service\DeviceService;
use App\Service\MessageService;

class Services {
    /**
     * 注册所有服务
     * @param DIContainer $container 依赖注入容器
     * @return void
     */
    public static function register(DIContainer $container) {
        // 注册配置服务
        $container->register('config', function() {
            return Config::getInstance();
        });

        // 注册数据库服务
        $container->register('db', function() {
            return Database::getInstance()->getConnection();
        });

        // 注册日志服务
        $container->register('logger', function() {
            $logger = Logger::getInstance();
            $logger->setLevel(Config::get('log.level', 'info'));
            return $logger;
        });

        // 注册缓存服务
        $container->register('cache', function() {
            $cache = new Cache();
            return $cache;
        });

        // 注册响应服务
        $container->register('response', function() {
            $response = Response::getInstance();
            return $response;
        });

        // 注册认证服务
        $container->register('authService', function($container) {
            return new AuthService($container->get('db'), $container->get('logger'), $container->get('cache'));
        });

        // 注册设备服务
        $container->register('deviceService', function($container) {
            return new DeviceService($container->get('db'), $container->get('logger'), $container->get('cache'));
        });

        // 注册消息服务
        $container->register('messageService', function($container) {
            return new MessageService($container->get('db'), $container->get('logger'), $container->get('cache'));
        });
    }
}