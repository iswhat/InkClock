<?php
/**
 * AuthService测试类
 */

require_once __DIR__ . '/TestCase.php';
require_once __DIR__ . '/../services/AuthService.php';

class AuthServiceTest extends TestCase {
    private $authService;
    
    public function __construct($testName) {
        parent::__construct($testName);
        $this->authService = new AuthService();
    }
    
    /**
     * 测试管理员检查功能
     */
    public function testIsAdmin() {
        // 测试admin是管理员
        $isAdmin = $this->authService->isAdmin('admin');
        $this->assertTrue($isAdmin, 'admin应该是管理员');
        
        // 测试普通用户不是管理员
        $isAdmin = $this->authService->isAdmin('user123');
        $this->assertFalse($isAdmin, '普通用户不应该是管理员');
        
        // 测试空用户名不是管理员
        $isAdmin = $this->authService->isAdmin('');
        $this->assertFalse($isAdmin, '空用户名不应该是管理员');
    }
    
    /**
     * 测试API密钥验证功能
     */
    public function testValidateApiKey() {
        // 测试无效API密钥
        $result = $this->authService->validateApiKey('invalid_api_key');
        $this->assertTrue($result['success'] === false, '无效API密钥应该验证失败');
        $this->assertTrue(isset($result['error']), '验证失败应该返回错误信息');
    }
    
    /**
     * 测试用户注册信息验证
     */
    public function testRegisterUserValidation() {
        // 测试缺少必填字段的情况
        $invalidUser = ['username' => 'test', 'email' => 'test@example.com'];
        $result = $this->authService->registerUser($invalidUser);
        $this->assertTrue($result['success'] === false, '缺少密码应该注册失败');
        
        // 测试无效邮箱
        $invalidUser = [
            'username' => 'test',
            'email' => 'invalid_email',
            'password' => 'password123'
        ];
        $result = $this->authService->registerUser($invalidUser);
        $this->assertTrue($result['success'] === false, '无效邮箱应该注册失败');
        
        // 测试短密码
        $invalidUser = [
            'username' => 'test',
            'email' => 'test@example.com',
            'password' => 'short'
        ];
        $result = $this->authService->registerUser($invalidUser);
        $this->assertTrue($result['success'] === false, '短密码应该注册失败');
        
        // 测试短用户名
        $invalidUser = [
            'username' => 'te',
            'email' => 'test@example.com',
            'password' => 'password123'
        ];
        $result = $this->authService->registerUser($invalidUser);
        $this->assertTrue($result['success'] === false, '短用户名应该注册失败');
    }
    
    /**
     * 测试用户登录信息验证
     */
    public function testLoginUserValidation() {
        // 测试空用户名登录
        $result = $this->authService->loginUser('', 'password123');
        $this->assertTrue($result['success'] === false, '空用户名应该登录失败');
        
        // 测试空密码登录
        $result = $this->authService->loginUser('test', '');
        $this->assertTrue($result['success'] === false, '空密码应该登录失败');
    }
}
?>