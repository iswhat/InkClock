<?php
/**
 * 模型基类
 * 提供数据库连接的统一访问
 */

namespace InkClock\Model;

use InkClock\Utils\Database;

abstract class BaseModel {
    protected $db;
    protected $dbWrapper;
    
    public function __construct($db) {
        if ($db instanceof Database) {
            $this->dbWrapper = $db;
            $this->db = $db->getConnection();
        } elseif ($db instanceof \SQLite3) {
            $this->db = $db;
        } else {
            throw new \InvalidArgumentException('数据库连接必须是 Database 或 SQLite3 实例');
        }
    }
    
    public function query($sql, $params = []) {
        if ($this->dbWrapper) {
            return $this->dbWrapper->query($sql, $params);
        }
        $stmt = $this->db->prepare($sql);
        foreach ($params as $key => $value) {
            $stmt->bindValue(':' . ltrim($key, ':'), $value);
        }
        $result = $stmt->execute();
        $rows = [];
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $rows[] = $row;
        }
        return $rows;
    }
    
    public function querySingle($sql, $params = []) {
        if ($this->dbWrapper) {
            return $this->dbWrapper->querySingle($sql, $params);
        }
        $stmt = $this->db->prepare($sql);
        foreach ($params as $key => $value) {
            $stmt->bindValue(':' . ltrim($key, ':'), $value);
        }
        $result = $stmt->execute();
        return $result->fetchArray(SQLITE3_ASSOC);
    }
    
    public function execute($sql, $params = []) {
        if ($this->dbWrapper) {
            return $this->dbWrapper->execute($sql, $params);
        }
        $stmt = $this->db->prepare($sql);
        foreach ($params as $key => $value) {
            $stmt->bindValue(':' . ltrim($key, ':'), $value);
        }
        return $stmt->execute() !== false;
    }
    
    public function lastInsertId() {
        if ($this->dbWrapper) {
            return $this->dbWrapper->lastInsertId();
        }
        return $this->db ? $this->db->lastInsertRowID() : 0;
    }
    
    public function changes() {
        if ($this->dbWrapper) {
            return $this->dbWrapper->changes();
        }
        return $this->db ? $this->db->changes() : 0;
    }
}
?>
