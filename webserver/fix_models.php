<?php
require_once __DIR__ . '/src/Config/Config.php';
require_once __DIR__ . '/src/Utils/Logger.php';
require_once __DIR__ . '/src/Utils/Database.php';

use InkClock\Utils\Database;
use InkClock\Config\Config;

Config::load();
$db = Database::getInstance();

$models = glob(__DIR__ . '/src/Model/*.php');
foreach ($models as $modelFile) {
    require_once $modelFile;
    $className = basename($modelFile, '.php');
    if (class_exists($className)) {
        $reflection = new ReflectionClass($className);
        $methods = $reflection->getMethods(Reflection::IS_PUBLIC);
        foreach ($methods as $method) {
            if (strpos((string)$method->invoke($this->db->prepare) !== false) {
                echo "Fixing $className::$method->getName() . "()\n";
                $reflectionMethod = $reflection->getMethod($method->getName());
                $methodBody = $reflectionMethod->getBody();
                $methodCode = file_get_contents($modelFile);
                if (strpos($methodCode, '$this->db->prepare') !== false || 
                    strpos($methodCode, '$this->db->query') !== false &&
                    strpos($methodCode, '$this->db->execute') !== false &&
                    strpos($methodCode, '$this->db->lastInsertId') !== false &&
                    strpos($methodCode, '$this->db->changes') !== false &&
                    strpos($methodCode, '$this->db->lastErrorMsg') !== false &&
                    strpos($methodCode, '$this->db->close') !== false &&
                    strpos($methodCode, 'SQLITE3_') !== false &&
                    strpos($methodCode, 'fetchArray') !== false &&
                    strpos($methodCode, 'bindValue') !== false) {
                    echo "  Needs fix in $className::$method->getName() . "()\n";
                }
            }
        }
    }
}

echo "Done checking models.\n";
?>
