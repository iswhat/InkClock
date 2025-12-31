<?php
/**
 * 数据库连接工具类
 */

class Database {
    private static $instance = null;
    private $db;
    private $config;

    /**
     * 私有构造函数，防止直接实例化
     */
    private function __construct() {
        $this->loadConfig();
        $this->connect();
    }

    /**
     * 加载配置
     */
    private function loadConfig() {
        require_once __DIR__ . '/../config/config.php';
        global $config;
        $this->config = $config['db'];
    }

    /**
     * 连接数据库
     */
    private function connect() {
        $this->db = new mysqli(
            $this->config['host'],
            $this->config['user'],
            $this->config['password'],
            $this->config['database'],
            $this->config['port']
        );

        if ($this->db->connect_error) {
            die("数据库连接失败: " . $this->db->connect_error);
        }

        $this->db->set_charset($this->config['charset']);
    }

    /**
     * 获取数据库连接实例
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new Database();
        }
        return self::$instance;
    }

    /**
     * 获取数据库连接对象
     */
    public function getConnection() {
        return $this->db;
    }

    /**
     * 执行查询并返回结果
     */
    public function query($sql, $params = array(), $types = '') {
        $stmt = $this->db->prepare($sql);
        if ($stmt === false) {
            return array('error' => $this->db->error);
        }

        if (!empty($params)) {
            $stmt->bind_param($types, ...$params);
        }

        $result = $stmt->execute();
        if (!$result) {
            return array('error' => $stmt->error);
        }

        $resultObj = $stmt->get_result();
        $data = array();

        if ($resultObj !== false) {
            while ($row = $resultObj->fetch_assoc()) {
                $data[] = $row;
            }
        }

        $stmt->close();
        return array('success' => true, 'data' => $data, 'affected_rows' => $this->db->affected_rows);
    }

    /**
     * 执行插入操作
     */
    public function insert($table, $data) {
        $columns = implode(', ', array_keys($data));
        $placeholders = implode(', ', array_fill(0, count($data), '?'));
        $values = array_values($data);
        $types = str_repeat('s', count($values));

        $sql = "INSERT INTO $table ($columns) VALUES ($placeholders)";
        $result = $this->query($sql, $values, $types);
        
        if ($result['success']) {
            $result['insert_id'] = $this->db->insert_id;
        }
        
        return $result;
    }

    /**
     * 执行更新操作
     */
    public function update($table, $data, $where, $whereParams = array(), $whereTypes = '') {
        $setClause = implode(', ', array_map(function($key) { return "$key = ?"; }, array_keys($data)));
        $values = array_values($data);
        $types = str_repeat('s', count($values)) . $whereTypes;

        // 合并数据值和条件值
        $allParams = array_merge($values, $whereParams);

        $sql = "UPDATE $table SET $setClause WHERE $where";
        return $this->query($sql, $allParams, $types);
    }

    /**
     * 执行删除操作
     */
    public function delete($table, $where, $whereParams = array(), $whereTypes = '') {
        $sql = "DELETE FROM $table WHERE $where";
        return $this->query($sql, $whereParams, $whereTypes);
    }

    /**
     * 预处理语句
     */
    public function prepare($sql) {
        return $this->db->prepare($sql);
    }

    /**
     * 获取最后插入的ID
     */
    public function getLastInsertId() {
        return $this->db->insert_id;
    }

    /**
     * 开始事务
     */
    public function beginTransaction() {
        return $this->db->begin_transaction();
    }

    /**
     * 提交事务
     */
    public function commit() {
        return $this->db->commit();
    }

    /**
     * 回滚事务
     */
    public function rollback() {
        return $this->db->rollback();
    }
}
?>