<?php
/**
 * 插件控制器
 */

namespace App\Controller;

class PluginController extends BaseController {
    /**
     * 获取插件列表
     */
    public function getPlugins() {
        $user = $this->checkApiPermission(true);
        $this->logAction('plugin_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../../plugin/manage.php';
        $plugins = get_plugins();
        
        $this->response->success('获取成功', $plugins);
    }
    
    /**
     * 添加插件
     */
    public function addPlugin() {
        $user = $this->checkApiPermission(true);
        $this->logAction('plugin_add', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../../plugin/manage.php';
        $result = add_plugin($data);
        
        if ($result['success']) {
            $this->response->success('插件添加成功', array('plugin_id' => $result['plugin_id']));
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 更新插件
     */
    public function updatePlugin($params) {
        $index = $params['index'];
        $user = $this->checkApiPermission(true);
        $this->logAction('plugin_update', array('plugin_index' => $index, 'user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../../plugin/manage.php';
        $result = update_plugin($index, $data);
        
        if ($result['success']) {
            $this->response->success('插件更新成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
    
    /**
     * 删除插件
     */
    public function deletePlugin($params) {
        $index = $params['index'];
        $user = $this->checkApiPermission(true);
        $this->logAction('plugin_delete', array('plugin_index' => $index, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../../plugin/manage.php';
        $result = delete_plugin($index);
        
        if ($result['success']) {
            $this->response->success('插件删除成功');
        } else {
            $this->response->error($result['error'], 400);
        }
    }
}
?>