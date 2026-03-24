<?php

$modelsDir = __DIR__ . '/src/Model';
$models = glob($modelsDir . '*.php');

foreach ($models as $modelFile) {
    $content = file_get_contents($modelFile);
    
    // 替换 $this->db->prepare(" with $this->db->query(
    $content = preg_replace('/\$this->db->prepare\(/', '$this->db->query(', $content);
    
    // 替换 $this->db->prepare(" with $this->db->execute(
    $content = preg_replace('/\$this->db->prepare\(/', '$this->db->execute(', $content);
    
    // 替换 SQLITE3_TEXT with :value
    $content = preg_replace('/SQLITE3_TEXT/', ':value', $content);
    
    // 替换 SQLITE3_INTEGER with :value
    $content = preg_replace('/SQLITE3_INTEGER/', ':value', $content);
    
    // 替换 SQLITE3_ASSOC with :value
    $content = preg_replace('/SQLITE3_ASSOC/', ':value', $content);
    
    // 替换 SQLITE3_BLOB with :value
    $content = preg_replace('/SQLITE3_BLOB/', ':value', $content);
    
    // 替换 SQLITE3_NULL with :value
    $content = preg_replace('/SQLITE3_NULL/', ':value', $content);
    
    // 替换 ->bindValue( with :value
    $content = preg_replace('/->bindValue\(([^,]+),\s*(SQLITE3_\w+),\s*,\s*\$params[$1]', $content);
    
    // 替换 ->fetchArray(SQLITE3_ASSOC) with return
    $content = preg_replace('/->fetchArray\(SQLITE3_ASSOC\)/', 'return', $content);
    
    // 替换 ->execute() with ->query() or ->execute()
    $content = preg_replace('/->execute\(\)/', '->query(', $content);
    
    // 替换 ->lastInsertRowID() with ->lastInsertId()
    $content = preg_replace('/->lastInsertRowID\(\)/', '->lastInsertId()', $content);
    
    // 替换 ->lastErrorMsg() with getErrorMessage()
    $content = preg_replace('/->lastErrorMsg\(\)/', 'getErrorMessage()', $content);
    
    // 替换 ->close() with nothing
    $content = preg_replace('/->close\(\)/', '', $content);
    
    // 替换 while.*fetchArray.* with return
    $content = preg_replace('/while\s*\$result->fetchArray\(SQLITE3_ASSOC\)\s*\s*\$versions\[\]\s*=\s*\$row;\s*}/', 'return $versions;', $content);
    
    file_put_contents($modelFile, $content);
}

echo "All models fixed!\n";
?>
