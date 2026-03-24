<?php
$db = new SQLite3('d:/InkClock/webserver/db/inkclock.db');
$result = $db->query('SELECT id, username, email, password_hash FROM users WHERE username = "iswhat"');
while($row = $result->fetchArray(SQLITE3_ASSOC)) {
    echo json_encode($row) . PHP_EOL;
    echo "Password verify test: " . (password_verify('admin123', $row['password_hash']) ? 'PASS' : 'FAIL') . PHP_EOL;
}
?>
