<?php
/**
 * 消息模板控制器
 */

namespace App\Controller;

use App\Model\MessageTemplate;

class MessageTemplateController extends BaseController {
    /**
     * 获取消息模板列表
     */
    public function getTemplates($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('message_template_get_list', array('user_id' => $user['id']));
        
        $limit = isset($_GET['limit']) ? (int)$_GET['limit'] : 50;
        $offset = isset($_GET['offset']) ? (int)$_GET['offset'] : 0;
        
        $templateModel = new MessageTemplate($this->db);
        $templates = $templateModel->getTemplates($user['id'], $limit, $offset);
        
        $this->response->success('获取成功', $templates);
    }
    
    /**
     * 获取消息模板详情
     */
    public function getTemplate($params) {
        $user = $this->checkApiPermission(true);
        $templateId = $params['id'];
        $this->logAction('message_template_get_detail', array('user_id' => $user['id'], 'template_id' => $templateId));
        
        $templateModel = new MessageTemplate($this->db);
        $template = $templateModel->getTemplateById($templateId, $user['id']);
        
        if ($template) {
            $this->response->success('获取成功', $template);
        } else {
            $this->response->error('模板不存在', 404);
        }
    }
    
    /**
     * 创建消息模板
     */
    public function createTemplate($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('message_template_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        $templateModel = new MessageTemplate($this->db);
        $result = $templateModel->createTemplate($data['name'], $data['content'], $data['type'] ?? 'text', $data['variables'] ?? array(), $user['id']);
        
        if ($result['success']) {
            $this->response->success('创建成功', array('template_id' => $result['template_id']));
        } else {
            $this->response->error('创建失败', 400);
        }
    }
    
    /**
     * 更新消息模板
     */
    public function updateTemplate($params) {
        $user = $this->checkApiPermission(true);
        $templateId = $params['id'];
        $this->logAction('message_template_update', array('user_id' => $user['id'], 'template_id' => $templateId));
        
        $data = $this->parseRequestBody();
        
        $templateModel = new MessageTemplate($this->db);
        $result = $templateModel->updateTemplate($templateId, $user['id'], $data['name'], $data['content'], $data['type'] ?? 'text');
        
        if ($result['success']) {
            $this->response->success('更新成功');
        } else {
            $this->response->error('更新失败', 400);
        }
    }
    
    /**
     * 删除消息模板
     */
    public function deleteTemplate($params) {
        $user = $this->checkApiPermission(true);
        $templateId = $params['id'];
        $this->logAction('message_template_delete', array('user_id' => $user['id'], 'template_id' => $templateId));
        
        $templateModel = new MessageTemplate($this->db);
        $result = $templateModel->deleteTemplate($templateId, $user['id']);
        
        if ($result['success']) {
            $this->response->success('删除成功');
        } else {
            $this->response->error('删除失败', 400);
        }
    }
}
?>