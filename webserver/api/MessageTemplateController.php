<?php
/**
 * 消息模板控制器
 */

class MessageTemplateController extends BaseController {
    /**
     * 创建消息模板
     */
    public function createTemplate($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('message_template_create', array('user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/MessageTemplate.php';
        $templateModel = new MessageTemplate();
        $result = $templateModel->createTemplate(
            $data['name'],
            $data['content'],
            $data['type'] ?? 'text',
            $data['variables'] ?? array(),
            $user['id']
        );
        
        if ($result['success']) {
            $this->response::success('消息模板创建成功', array('template_id' => $result['template_id']));
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 获取消息模板列表
     */
    public function getTemplates($params) {
        $user = $this->checkApiPermission(true);
        $this->logAction('message_template_get_list', array('user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/MessageTemplate.php';
        $templateModel = new MessageTemplate();
        $templates = $templateModel->getTemplates($user['id']);
        
        $this->response::success('获取成功', $templates);
    }
    
    /**
     * 获取消息模板详情
     */
    public function getTemplate($params) {
        $template_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('message_template_get_detail', array('template_id' => $template_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/MessageTemplate.php';
        $templateModel = new MessageTemplate();
        $template = $templateModel->getTemplate($template_id);
        
        if (!$template) {
            $this->response::error('消息模板不存在', 404);
        }
        
        // 检查模板是否属于当前用户
        if ($template['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $this->response::success('获取成功', $template);
    }
    
    /**
     * 更新消息模板
     */
    public function updateTemplate($params) {
        $template_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('message_template_update', array('template_id' => $template_id, 'user_id' => $user['id']));
        
        $data = $this->parseRequestBody();
        
        require_once __DIR__ . '/../models/MessageTemplate.php';
        $templateModel = new MessageTemplate();
        $template = $templateModel->getTemplate($template_id);
        
        if (!$template) {
            $this->response::error('消息模板不存在', 404);
        }
        
        // 检查模板是否属于当前用户
        if ($template['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $templateModel->updateTemplate(
            $template_id,
            $data['name'] ?? $template['name'],
            $data['content'] ?? $template['content'],
            $data['type'] ?? $template['type'],
            $data['variables'] ?? $template['variables']
        );
        
        if ($result['success']) {
            $this->response::success('消息模板更新成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
    
    /**
     * 删除消息模板
     */
    public function deleteTemplate($params) {
        $template_id = $params['id'];
        $user = $this->checkApiPermission(true);
        $this->logAction('message_template_delete', array('template_id' => $template_id, 'user_id' => $user['id']));
        
        require_once __DIR__ . '/../models/MessageTemplate.php';
        $templateModel = new MessageTemplate();
        $template = $templateModel->getTemplate($template_id);
        
        if (!$template) {
            $this->response::error('消息模板不存在', 404);
        }
        
        // 检查模板是否属于当前用户
        if ($template['user_id'] !== $user['id']) {
            $this->response::forbidden();
        }
        
        $result = $templateModel->deleteTemplate($template_id);
        
        if ($result['success']) {
            $this->response::success('消息模板删除成功');
        } else {
            $this->response::error($result['error'], 400);
        }
    }
}
?>