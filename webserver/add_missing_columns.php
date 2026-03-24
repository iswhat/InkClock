<?php
$db = new SQLite3('d:/InkClock/webserver/db/inkclock.db');

$columns = [
    "ALTER TABLE users ADD COLUMN two_factor_enabled INTEGER DEFAULT 0",
    "ALTER TABLE users ADD COLUMN two_factor_secret TEXT",
    "ALTER TABLE users ADD COLUMN api_key_ip_whitelist TEXT",
    "ALTER TABLE users ADD COLUMN last_login TEXT"
];

foreach ($columns as $sql) {
    try {
        $db->exec($sql);
        echo "Added column successfully\n";
    } catch (Exception $e) {
        if (strpos($e->getMessage(), 'duplicate column name') !== false) {
            echo "Column already exists\n";
        } else {
            echo "Error: " . $e->getMessage() . "\n";
        }
    }
}

echo "Done!\n";
?>
