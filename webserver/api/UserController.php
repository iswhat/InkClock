<?php
/**
 * 用户控制器
 */

namespace InkClock\Api;

class UserController extends BaseController {
    /**
     * 用户注册
     */
    public function register($params) {
        $this->logAction('user_register');
        $data = $this->parseRequestBody();
        
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
        
        // 使用服务层处理登录
        $result = $this->authService->loginUser($data['username'], $data['password']);
        
        if ($result['success']) {
            $this->response->success('登录成功', array('user_id' => $result['user_id'], 'api_key' => $result['api_key']));
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
            $this->response::error($result['error'], 400);
        }
    }
}
?>