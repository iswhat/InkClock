<?php
$db = new SQLite3('d:/InkClock/webserver/db/inkclock.db');
$result = $db->query('SELECT id, username, email, status, is_admin, api_key, api_key_expires_at FROM users WHERE username = "iswhat"');
while($row = $result->fetchArray(SQLITE3_ASSOC)) {
    echo json_encode($row, JSON_PRETTY_PRINT) . PHP_EOL;
}
?>
