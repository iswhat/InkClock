<?php
/**
 * 消息模板模型
 */
require_once __DIR__ . '/../utils/Database.php';

class MessageTemplate {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance()->getConnection();
    }
    
    /**
     * 创建消息模板
     */
    public function createTemplate($name, $content, $type = 'text', $variables = array(), $userId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $variablesJson = json_encode($variables);
        
        $stmt = $this->db->prepare("INSERT INTO message_templates (name, content, type, variables, user_id, created_at) VALUES (?, ?, ?, ?, ?, ?)");
        $stmt->bind_param("ssssis", $name, $content, $type, $variablesJson, $userId, $createdAt);
        $result = $stmt->execute();
        $templateId = $this->db->insert_id;
        $stmt->close();
        
        return array('success' => $result, 'template_id' => $templateId);
    }
    
    /**
     * 获取用户的消息模板列表
     */
    public function getTemplatesByUserId($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM message_templates WHERE user_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bind_param("iii", $userId, $limit, $offset);
        $stmt->execute();
        $result = $stmt->get_result();
        $templates = $result->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        return $templates;
    }
    
    /**
     * 获取用户的消息模板列表（控制器调用的方法名）
     */
    public function getTemplates($userId, $limit = 50, $offset = 0) {
        return $this->getTemplatesByUserId($userId, $limit, $offset);
    }
    
    /**
     * 获取单个消息模板
     */
    public function getTemplateById($templateId, $userId) {
        $stmt = $this->db->prepare("SELECT * FROM message_templates WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ii", $templateId, $userId);
        $stmt->execute();
        $result = $stmt->get_result();
        $template = $result->fetch_assoc();
        $stmt->close();
        return $template;
    }
    
    /**
     * 获取单个消息模板（控制器调用的方法名）
     */
    public function getTemplate($templateId) {
        // 控制器会在调用前检查权限，这里直接查询
        $stmt = $this->db->prepare("SELECT * FROM message_templates WHERE id = ?");
        $stmt->bind_param("i", $templateId);
        $stmt->execute();
        $result = $stmt->get_result();
        $template = $result->fetch_assoc();
        $stmt->close();
        return $template;
    }
    
    /**
     * 更新消息模板
     */
    public function updateTemplate($templateId, $userId, $name, $content, $type = 'text') {
        $stmt = $this->db->prepare("UPDATE message_templates SET name = ?, content = ?, type = ? WHERE id = ? AND user_id = ?");
        $stmt->bind_param("sssii", $name, $content, $type, $templateId, $userId);
        $result = $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $result, 'updated' => $affectedRows > 0);
    }
    
    /**
     * 删除消息模板
     */
    public function deleteTemplate($templateId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM message_templates WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ii", $templateId, $userId);
        $result = $stmt->execute();
        $affectedRows = $stmt->affected_rows;
        $stmt->close();
        
        return array('success' => $result, 'deleted' => $affectedRows > 0);
    }
    
    /**
     * 检查模板是否存在且属于该用户
     */
    public function templateExists($templateId, $userId) {
        $stmt = $this->db->prepare("SELECT id FROM message_templates WHERE id = ? AND user_id = ?");
        $stmt->bind_param("ii", $templateId, $userId);
        $stmt->execute();
        $result = $stmt->get_result();
        $exists = $result->num_rows > 0;
        $stmt->close();
        return $exists;
    }
}
?>