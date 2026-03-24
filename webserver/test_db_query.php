<?php
require_once __DIR__ . '/src/Config/Config.php';
require_once __DIR__ . '/src/Utils/Logger.php';
require_once __DIR__ . '/src/Utils/Database.php';

use InkClock\Utils\Database;
use InkClock\Config\Config;

Config::load();
$db = Database::getInstance();

$sql = "SELECT id, username, email, password_hash, api_key, api_key_expires_at, status, is_admin FROM users WHERE username = :username OR email = :email";
$params = [
    'username' => 'iswhat',
    'email' => 'iswhat'
];

$result = $db->query($sql, $params);
echo "Result count: " . count($result) . PHP_EOL;
if (!empty($result)) {
    echo "First row: " . json_encode($result[0], JSON_PRETTY_PRINT) . PHP_EOL;
    echo "Password verify: " . (password_verify('admin123', $result[0]['password_hash']) ? 'PASS' : 'FAIL') . PHP_EOL;
}
?>
