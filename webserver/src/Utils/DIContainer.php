<?php
/**
 * 依赖注入容器 - 支持构造函数自动注入和服务别名
 */

namespace InkClock\Utils;

class DIContainer {
    private $services = [];
    private $instances = [];
    private $aliases = [];
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
     * 注册服务类（支持自动注入）
     * @param string $name 服务名称
     * @param string $className 类名
     * @return void
     */
    public function registerClass($name, $className) {
        $this->services[$name] = function($container) use ($className) {
            return $container->createInstance($className);
        };
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
     * 创建类实例，自动注入构造函数依赖
     * @param string $className 类名
     * @return mixed 类实例
     * @throws \Exception 如果依赖注入失败
     */
    public function createInstance($className) {
        // 获取类的反射
        $reflection = new \ReflectionClass($className);
        
        // 检查类是否可实例化
        if (!$reflection->isInstantiable()) {
            throw new \Exception("Class {$className} is not instantiable");
        }
        
        // 获取构造函数
        $constructor = $reflection->getConstructor();
        
        // 如果没有构造函数，直接创建实例
        if (!$constructor) {
            return new $className();
        }
        
        // 获取构造函数参数
        $parameters = $constructor->getParameters();
        $dependencies = [];
        
        // 解析每个参数的依赖
        foreach ($parameters as $parameter) {
            $parameterType = $parameter->getType();
            
            // 检查参数是否有默认值
            if ($parameter->isOptional() && $parameter->isDefaultValueAvailable()) {
                // 如果有默认值，使用默认值
                $dependencies[] = $parameter->getDefaultValue();
            } elseif (!$parameterType || $parameterType instanceof \ReflectionNamedType === false) {
                // 如果参数没有类型提示且没有默认值，无法自动注入
                throw new \Exception("Cannot resolve dependency for parameter {$parameter->getName()} in {$className}");
            } else {
                // 如果有类型提示，尝试从容器中获取或创建实例
                $dependencyClass = $parameterType->getName();
                
                // 检查是否可以从容器中获取依赖
                try {
                    $dependencies[] = $this->get($dependencyClass);
                } catch (\Exception $e) {
                    // 如果直接获取失败，尝试创建实例
                    $dependencies[] = $this->createInstance($dependencyClass);
                }
            }
        }
        
        // 创建实例并注入依赖
        return $reflection->newInstanceArgs($dependencies);
    }

    /**
     * 获取服务实例
     * @param string $name 服务名称或类名
     * @return mixed
     * @throws \Exception 如果服务不存在
     */
    public function get($name) {
        // 检查是否是别名
        if (isset($this->aliases[$name])) {
            $name = $this->aliases[$name];
        }
        
        // 如果实例已存在，直接返回
        if (isset($this->instances[$name])) {
            return $this->instances[$name];
        }
        
        // 如果有服务工厂，调用工厂创建实例
        if (isset($this->services[$name])) {
            $this->instances[$name] = $this->services[$name]($this);
            return $this->instances[$name];
        }
        
        // 尝试直接创建类实例
        if (class_exists($name)) {
            $this->instances[$name] = $this->createInstance($name);
            return $this->instances[$name];
        }
        
        throw new \Exception("Service {$name} not found");
    }
    
    /**
     * 添加服务别名
     * @param string $alias 别名
     * @param string $name 服务名称
     * @return void
     */
    public function alias($alias, $name) {
        $this->aliases[$alias] = $name;
    }

    /**
     * 检查服务是否存在
     * @param string $name 服务名称
     * @return bool
     */
    public function has($name) {
        // 检查是否是别名
        if (isset($this->aliases[$name])) {
            $name = $this->aliases[$name];
        }
        
        return isset($this->services[$name]) || isset($this->instances[$name]) || class_exists($name);
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