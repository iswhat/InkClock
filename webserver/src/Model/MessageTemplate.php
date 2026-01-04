<?php
/**
 * 消息模板模型
 */
namespace App\Model;

use App\Utils\Database;

class MessageTemplate {
    private $db;
    
    public function __construct(Database $database) {
        $this->db = $database->getConnection();
    }
    
    /**
     * 创建消息模板
     */
    public function createTemplate($name, $content, $type = 'text', $variables = array(), $userId) {
        $createdAt = date('Y-m-d H:i:s');
        
        $variablesJson = json_encode($variables);
        
        $stmt = $this->db->prepare("INSERT INTO message_templates (name, content, type, variables, user_id, created_at) VALUES (?, ?, ?, ?, ?, ?)");
        $stmt->bindValue(1, $name, SQLITE3_TEXT);
        $stmt->bindValue(2, $content, SQLITE3_TEXT);
        $stmt->bindValue(3, $type, SQLITE3_TEXT);
        $stmt->bindValue(4, $variablesJson, SQLITE3_TEXT);
        $stmt->bindValue(5, $userId, SQLITE3_INTEGER);
        $stmt->bindValue(6, $createdAt, SQLITE3_TEXT);
        $result = $stmt->execute();
        $templateId = $this->db->lastInsertRowID();
        $stmt->close();
        
        return array('success' => $result !== false, 'template_id' => $templateId);
    }
    
    /**
     * 获取用户的消息模板列表
     */
    public function getTemplatesByUserId($userId, $limit = 50, $offset = 0) {
        $stmt = $this->db->prepare("SELECT * FROM message_templates WHERE user_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?");
        $stmt->bindValue(1, $userId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $limit, SQLITE3_INTEGER);
        $stmt->bindValue(3, $offset, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $templates = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $templates[] = $row;
        }
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
        $stmt->bindValue(1, $templateId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $template = $result->fetchArray(SQLITE3_ASSOC);
        $stmt->close();
        return $template;
    }
    
    /**
     * 获取单个消息模板（控制器调用的方法名）
     */
    public function getTemplate($templateId) {
        // 控制器会在调用前检查权限，这里直接查询
        $stmt = $this->db->prepare("SELECT * FROM message_templates WHERE id = ?");
        $stmt->bindValue(1, $templateId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $template = $result->fetchArray(SQLITE3_ASSOC);
        $stmt->close();
        return $template;
    }
    
    /**
     * 更新消息模板
     */
    public function updateTemplate($templateId, $userId, $name, $content, $type = 'text') {
        $stmt = $this->db->prepare("UPDATE message_templates SET name = ?, content = ?, type = ? WHERE id = ? AND user_id = ?");
        $stmt->bindValue(1, $name, SQLITE3_TEXT);
        $stmt->bindValue(2, $content, SQLITE3_TEXT);
        $stmt->bindValue(3, $type, SQLITE3_TEXT);
        $stmt->bindValue(4, $templateId, SQLITE3_INTEGER);
        $stmt->bindValue(5, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $affectedRows = $result !== false ? 1 : 0;
        $stmt->close();
        
        return array('success' => $result !== false, 'updated' => $affectedRows > 0);
    }
    
    /**
     * 删除消息模板
     */
    public function deleteTemplate($templateId, $userId) {
        $stmt = $this->db->prepare("DELETE FROM message_templates WHERE id = ? AND user_id = ?");
        $stmt->bindValue(1, $templateId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $affectedRows = $result !== false ? 1 : 0;
        $stmt->close();
        
        return array('success' => $result !== false, 'deleted' => $affectedRows > 0);
    }
    
    /**
     * 检查模板是否存在且属于该用户
     */
    public function templateExists($templateId, $userId) {
        $stmt = $this->db->prepare("SELECT id FROM message_templates WHERE id = ? AND user_id = ?");
        $stmt->bindValue(1, $templateId, SQLITE3_INTEGER);
        $stmt->bindValue(2, $userId, SQLITE3_INTEGER);
        $result = $stmt->execute();
        $row = $result->fetchArray(SQLITE3_ASSOC);
        $stmt->close();
        return !empty($row);
    }
}
?>