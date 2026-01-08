<?php
/**
 * 用户控制器
 */

namespace InkClock\Controller;

class UserController extends BaseController {
    /**
     * 用户注册
     */
    public function register($params) {
        $this->logAction('user_register');
        $data = $this->parseRequestBody();
        
        // 验证输入
        if (!isset($data['username']) || !isset($data['email']) || !isset($data['password'])) {
            $this->response->error('缺少必要参数', 400);
        }
        
        if (empty($data['username']) || empty($data['email']) || empty($data['password'])) {
            $this->response->error('参数不能为空', 400);
        }
        
        if (!filter_var($data['email'], FILTER_VALIDATE_EMAIL)) {
            $this->response->error('邮箱格式无效', 400);
        }
        
        if (strlen($data['password']) < 6) {
            $this->response->error('密码长度至少为6个字符', 400);
        }
        
        if (strlen($data['username']) < 3) {
            $this->response->error('用户名长度至少为3个字符', 400);
        }
        
        // 使用服务层处理注册
        $result = $this->authService->registerUser($data);
        
        if ($result['success']) {
            $this->response->success('注册成功', array('user_id' => $result['user_id'], 'api_key' => $result['api_key']));
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 用户登录
     */
    public function login($params) {
        $this->logAction('user_login');
        $data = $this->parseRequestBody();
        
        // 验证输入
        if (!isset($data['username']) || !isset($data['password'])) {
            $this->response->error('缺少用户名或密码', 400);
        }
        
        if (empty($data['username']) || empty($data['password'])) {
            $this->response->error('用户名和密码不能为空', 400);
        }
        
        // 使用服务层处理登录
        $result = $this->authService->loginUser($data['username'], $data['password']);
        
        if ($result['success']) {
            // 根据is_admin字段确定用户角色
            $userRole = $result['is_admin'] ? 'admin' : 'user';
            $this->response->success('登录成功', array('user_id' => $result['user_id'], 'api_key' => $result['api_key'], 'role' => $userRole));
        } else {
            $this->response->error($result['error'], 401);
        }
    }
    
    /**
     * 获取用户信息
     */
    public function getInfo($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('user_get_info', array('user_id' => $user['id']));
        
        // 移除敏感信息
        unset($user['password_hash']);
        unset($user['api_key']);
        
        $this->response->success('获取成功', $user);
    }
    
    /**
     * 获取用户设备列表
     */
    public function getDevices($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('user_get_devices', array('user_id' => $user['id']));
        
        // 使用服务层获取设备列表
        $result = $this->deviceService->getDeviceList($user['id']);
        
        if ($result['success']) {
            $this->response->success('获取成功', $result['devices']);
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 绑定设备
     */
    public function bindDevice($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        
        $this->logAction('user_bind_device', array('user_id' => $user['id'], 'device_id' => $data['device_id']));
        
        // 使用服务层绑定设备
        $result = $this->deviceService->bindDevice($user['id'], $data['device_id'], $data['nickname'] ?? '');
        
        if ($result['success']) {
            $this->response->success('绑定成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 解绑设备
     */
    public function unbindDevice($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        
        $this->logAction('user_unbind_device', array('user_id' => $user['id'], 'device_id' => $data['device_id']));
        
        // 使用服务层解绑设备
        $result = $this->deviceService->unbindDevice($user['id'], $data['device_id']);
        
        if ($result['success']) {
            $this->response->success('解绑成功');
        } else {
            $this->response->error('解绑失败', 400);
        }
    }
    
    /**
     * 更新设备昵称
     */
    public function updateDeviceNickname($params) {
        $user = $this->checkApiPermission(true);
        $data = $this->parseRequestBody();
        
        $this->logAction('user_update_device_nickname', array('user_id' => $user['id'], 'device_id' => $data['device_id']));
        
        // 使用服务层更新设备昵称
        $result = $this->deviceService->updateDevice($user['id'], $data['device_id'], array('nickname' => $data['nickname']));
        
        if ($result['success']) {
            $this->response->success('更新成功');
        } else {
            $this->response->error('更新失败', 400);
        }
    }
    
    /**
     * 检查是否有用户存在
     */
    public function checkUsers($params) {
        $hasUsers = $this->authService->hasUsers();
        $this->response->success('检查成功', array('has_users' => $hasUsers));
    }
    
    /**
     * 创建第一个管理员用户
     */
    public function createFirstAdmin($params) {
        $data = $this->parseRequestBody();
        
        $this->logAction('create_first_admin', $data);
        
        $result = $this->authService->createFirstAdmin($data);
        
        if ($result['success']) {
            $this->response->success('管理员创建成功', array('user_id' => $result['user_id'], 'api_key' => $result['api_key']));
        } else {
            $this->response->error($result['error'], 400);
        }
    }
}
?>