<?php
$db = new SQLite3('d:/InkClock/webserver/db/inkclock.db');
$passwordHash = password_hash('admin123', PASSWORD_DEFAULT);
$stmt = $db->prepare('UPDATE users SET password_hash = :hash WHERE username = "iswhat"');
$stmt->bindValue(':hash', $passwordHash, SQLITE3_TEXT);
$result = $stmt->execute();
if ($result) {
    echo "密码已重置为 admin123\n";
} else {
    echo "重置失败\n";
}
?>
