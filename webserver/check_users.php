<?php
$db = new SQLite3('d:/InkClock/webserver/db/inkclock.db');
$result = $db->query('SELECT id, username, email, is_admin FROM users');
while($row = $result->fetchArray(SQLITE3_ASSOC)) {
    echo json_encode($row) . PHP_EOL;
}
?>
