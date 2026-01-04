<?php
/**
 * 依赖注入容器
 */

namespace App\Utils;

class DIContainer {
    private $services = [];
    private $instances = [];
    private static $instance = null;
    
    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        // 私有构造函数
    }
    
    /**
     * 获取单例实例
     * @return DIContainer
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }

    /**
     * 注册服务
     * @param string $name 服务名称
     * @param callable $callback 服务工厂函数
     * @return void
     */
    public function register($name, callable $callback) {
        $this->services[$name] = $callback;
    }
    
    /**
     * 直接设置服务实例
     * @param string $name 服务名称
     * @param mixed $instance 服务实例
     * @return void
     */
    public function set($name, $instance) {
        $this->instances[$name] = $instance;
    }

    /**
     * 获取服务实例
     * @param string $name 服务名称
     * @return mixed
     * @throws \Exception 如果服务不存在
     */
    public function get($name) {
        if (!isset($this->instances[$name])) {
            if (isset($this->services[$name])) {
                $this->instances[$name] = $this->services[$name]($this);
            } else {
                throw new \Exception("Service {$name} not found");
            }
        }
        return $this->instances[$name];
    }

    /**
     * 检查服务是否存在
     * @param string $name 服务名称
     * @return bool
     */
    public function has($name) {
        return isset($this->services[$name]) || isset($this->instances[$name]);
    }

    /**
     * 清除服务实例
     * @param string $name 服务名称
     * @return void
     */
    public function clear($name) {
        if (isset($this->instances[$name])) {
            unset($this->instances[$name]);
        }
    }

    /**
     * 清除所有服务实例
     * @return void
     */
    public function clearAll() {
        $this->instances = [];
    }
}