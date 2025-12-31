<?php
/**
 * 服务注册配置
 */

namespace InkClock\Config;

use InkClock\Utils\DIContainer;
use InkClock\Utils\Database;
use InkClock\Utils\Logger;
use InkClock\Service\AuthService;
use InkClock\Service\DeviceService;
use InkClock\Service\MessageService;

class Services {
    /**
     * 注册所有服务
     * @param DIContainer $container 依赖注入容器
     * @return void
     */
    public static function register(DIContainer $container) {
        // 注册配置服务
        $container->register('config', function() {
            return Config::class;
        });

        // 注册数据库服务
        $container->register('db', function() {
            return Database::getInstance()->getConnection();
        });

        // 注册日志服务
        $container->register('logger', function() {
            $logger = Logger::getInstance();
            $logger->setLevel(Config::get('log.level', 'info'));
            $logger->setLogFile(Config::get('log.file_path', __DIR__ . '/../../logs/app.log'));
            return $logger;
        });

        // 注册认证服务
        $container->register('authService', function($container) {
            return new AuthService($container->get('db'), $container->get('logger'));
        });

        // 注册设备服务
        $container->register('deviceService', function($container) {
            return new DeviceService($container->get('db'), $container->get('logger'));
        });

        // 注册消息服务
        $container->register('messageService', function($container) {
            return new MessageService($container->get('db'), $container->get('logger'));
        });
    }
}