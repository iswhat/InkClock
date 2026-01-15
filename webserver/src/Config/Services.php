<?php
/**
 * 服务注册配置
 */

namespace InkClock\Config;

use InkClock\Utils\DIContainer;
use InkClock\Utils\Database;
use InkClock\Utils\Logger;
use InkClock\Utils\Cache;
use InkClock\Utils\Response;
use InkClock\Config\Config;
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
            return Config::getInstance();
        });

        // 注册数据库服务
        $container->register('db', function() {
            // 检查SQLite3扩展是否存在
            if (!class_exists('SQLite3')) {
                // 返回一个假的连接对象
                return (object) [
                    'prepare' => function() {
                        return (object) [
                            'bindValue' => function() {},
                            'execute' => function() {
                                return (object) [
                                    'fetchArray' => function() { return false; }
                                ];
                            }
                        ];
                    },
                    'query' => function() {
                        return (object) [
                            'fetchArray' => function() { return false; }
                        ];
                    },
                    'exec' => function() { return false; },
                    'lastInsertRowID' => function() { return 0; },
                    'changes' => function() { return 0; },
                    'close' => function() { return true; }
                ];
            }
            $db = Database::getInstance();
            return $db ? $db->getConnection() : null;
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