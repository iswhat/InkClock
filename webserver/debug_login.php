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

echo "Query result: " . json_encode($result, JSON_PRETTY_PRINT) . PHP_EOL;

if (!empty($result)) {
    $user = $result[0];
    echo "Status type: " . gettype($user['status']) . PHP_EOL;
    echo "Status value: " . $user['status'] . PHP_EOL;
    echo "Status === 0: " . ($user['status'] === 0 ? 'true' : 'false') . PHP_EOL;
    echo "Status == 0: " . ($user['status'] == 0 ? 'true' : 'false') . PHP_EOL;
    echo "Password verify: " . (password_verify('admin123', $user['password_hash']) ? 'PASS' : 'FAIL') . PHP_EOL;
}
?>
