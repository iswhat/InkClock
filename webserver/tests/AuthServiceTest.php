<?php
/**
 * AuthService测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../vendor/autoload.php';

use InkClock\Service\AuthService;
use InkClock\Utils\Logger;
use InkClock\Utils\Database;

class AuthServiceTest extends TestCase {
    private $authService;
    private $db;
    private $logger;
    
    public function __construct($testName) {
        parent::__construct($testName);
        
        // 初始化依赖
        $this->logger = new Logger();
        $this->logger->setLevel('error'); // 测试时只记录错误
        
        // 获取数据库连接
        $database = Database::getInstance();
        $this->db = $database->getConnection();
        
        // 创建AuthService实例
        $this->authService = new AuthService($this->db, $this->logger);
    }
    
    /**
     * 测试用户注册
     */
    public function testRegisterUser() {
        // 测试有效用户注册
        $userInfo = [
            'username' => 'testuser_' . time(),
            'email' => 'testuser_' . time() . '@example.com',
            'password' => 'password123'
        ];
        
        $result = $this->authService->registerUser($userInfo);
        $this->assertTrue($result['success'], '有效用户应该注册成功');
        $this->assertNotNull($result['user_id'], '注册成功应该返回用户ID');
        $this->assertNotNull($result['api_key'], '注册成功应该返回API密钥');
        
        // 测试无效用户注册（缺少必填字段）
        $invalidUser = [
            'username' => 'testuser',
            'email' => 'test@example.com'
            // 缺少password字段
        ];
        
        $result = $this->authService->registerUser($invalidUser);
        $this->assertFalse($result['success'], '缺少必填字段应该注册失败');
        
        // 测试无效邮箱注册
        $invalidEmailUser = [
            'username' => 'testuser',
            'email' => 'invalid_email',
            'password' => 'password123'
        ];
        
        $result = $this->authService->registerUser($invalidEmailUser);
        $this->assertFalse($result['success'], '无效邮箱应该注册失败');
    }
    
    /**
     * 测试用户登录
     */
    public function testLoginUser() {
        // 先注册一个用户
        $username = 'testuser_' . time();
        $email = $username . '@example.com';
        $password = 'password123';
        
        $this->authService->registerUser([
            'username' => $username,
            'email' => $email,
            'password' => $password
        ]);
        
        // 测试有效登录
        $result = $this->authService->loginUser($username, $password);
        $this->assertTrue($result['success'], '有效用户应该登录成功');
        $this->assertNotNull($result['user_id'], '登录成功应该返回用户ID');
        $this->assertNotNull($result['api_key'], '登录成功应该返回API密钥');
        
        // 测试邮箱登录
        $result = $this->authService->loginUser($email, $password);
        $this->assertTrue($result['success'], '使用邮箱应该登录成功');
        
        // 测试无效密码登录
        $result = $this->authService->loginUser($username, 'wrongpassword');
        $this->assertFalse($result['success'], '无效密码应该登录失败');
        
        // 测试不存在的用户登录
        $result = $this->authService->loginUser('nonexistentuser', $password);
        $this->assertFalse($result['success'], '不存在的用户应该登录失败');
    }
    
    /**
     * 测试API密钥验证
     */
    public function testValidateApiKey() {
        // 先注册一个用户
        $username = 'testuser_' . time();
        $email = $username . '@example.com';
        $password = 'password123';
        
        $registerResult = $this->authService->registerUser([
            'username' => $username,
            'email' => $email,
            'password' => $password
        ]);
        
        // 测试有效API密钥
        $result = $this->authService->validateApiKey($registerResult['api_key'], '127.0.0.1');
        $this->assertTrue($result['success'], '有效API密钥应该验证成功');
        $this->assertNotNull($result['user'], '验证成功应该返回用户信息');
        
        // 测试无效API密钥
        $result = $this->authService->validateApiKey('invalid_api_key', '127.0.0.1');
        $this->assertFalse($result['success'], '无效API密钥应该验证失败');
        
        // 测试IP白名单（如果支持的话）
        $result = $this->authService->validateApiKey($registerResult['api_key'], '192.168.1.1');
        $this->assertTrue($result['success'], '没有设置IP白名单的API密钥应该能被任何IP访问');
    }
    
    /**
     * 测试管理员检查
     */
    public function testIsAdmin() {
        // 测试管理员用户
        $adminUser = [
            'id' => 1,
            'is_admin' => 1
        ];
        $isAdmin = $this->authService->isAdmin($adminUser);
        $this->assertTrue($isAdmin, '管理员用户应该返回true');
        
        // 测试普通用户
        $normalUser = [
            'id' => 2,
            'is_admin' => 0
        ];
        $isAdmin = $this->authService->isAdmin($normalUser);
        $this->assertFalse($isAdmin, '普通用户应该返回false');
        
        // 测试空用户
        $isAdmin = $this->authService->isAdmin(null);
        $this->assertFalse($isAdmin, '空用户应该返回false');
    }
}
