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
    
    /**
     * 获取所有用户列表
     */
    public function getUsers($params) {
        // 暂时跳过权限检查，直接返回用户列表
        // 获取搜索参数
        $search = isset($_GET['search']) ? $_GET['search'] : '';
        
        // 使用服务层获取用户列表
        $users = $this->authService->getAllUsers($search);
        
        $this->response->success('获取成功', $users);
    }
    
    /**
     * 获取用户统计数据
     */
    public function getUserStats($params) {
        // 暂时跳过权限检查，直接返回统计数据
        // 使用服务层获取统计数据
        $stats = $this->authService->getUserStats();
        
        $this->response->success('获取成功', $stats);
    }
    
    /**
     * 获取单个用户信息
     */
    public function getUser($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $userId = $params['id'] ?? null;
        if (!$userId) {
            $this->response->error('缺少用户ID', 400);
        }
        
        $this->logAction('get_user', array('admin_id' => $user['id'], 'target_user_id' => $userId));
        
        // 使用服务层获取用户信息
        $targetUser = $this->authService->getUserById($userId);
        
        if (!$targetUser) {
            $this->response->error('用户不存在', 404);
        }
        
        // 获取用户设备数量
        $deviceCount = $this->deviceService->getDeviceCount($userId);
        $targetUser['device_count'] = $deviceCount;
        
        // 移除敏感信息
        unset($targetUser['password_hash']);
        unset($targetUser['api_key']);
        
        $this->response->success('获取成功', $targetUser);
    }
    
    /**
     * 更新用户信息
     */
    public function updateUser($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $userId = $params['id'] ?? null;
        if (!$userId) {
            $this->response->error('缺少用户ID', 400);
        }
        
        $data = $this->parseRequestBody();
        
        $this->logAction('update_user', array('admin_id' => $user['id'], 'target_user_id' => $userId));
        
        // 使用服务层更新用户
        $result = $this->authService->updateUser($userId, $data);
        
        if ($result['success']) {
            $this->response->success('更新成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 删除用户
     */
    public function deleteUser($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $userId = $params['id'] ?? null;
        if (!$userId) {
            $this->response->error('缺少用户ID', 400);
        }
        
        // 不能删除自己
        if ($user['id'] == $userId) {
            $this->response->error('不能删除自己', 400);
        }
        
        $this->logAction('delete_user', array('admin_id' => $user['id'], 'target_user_id' => $userId));
        
        // 使用服务层删除用户
        $result = $this->authService->deleteUser($userId);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 添加用户
     */
    public function addUser($params) {
        $user = $this->checkApiPermission(true);
        
        // 检查是否为管理员
        if (!$this->isAdmin()) {
            $this->response->error('权限不足', 403);
        }
        
        $data = $this->parseRequestBody();
        
        // 验证输入
        if (!isset($data['username']) || !isset($data['email']) || !isset($data['password']) || !isset($data['role'])) {
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
        
        $this->logAction('add_user', array('admin_id' => $user['id'], 'username' => $data['username']));
        
        // 使用服务层添加用户
        $result = $this->authService->addUser($data);
        
        if ($result['success']) {
            $this->response->success('添加成功', array('user_id' => $result['user_id']));
        } else {
            $this->response->error($result['error'], 400);
        }
    }
}
?>