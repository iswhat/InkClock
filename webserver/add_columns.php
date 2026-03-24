<?php
$db = new SQLite3('d:/InkClock/webserver/db/inkclock.db');
$db->exec('ALTER TABLE users ADD COLUMN two_factor_enabled INTEGER DEFAULT 0');
$db->exec('ALTER TABLE users ADD COLUMN two_factor_secret TEXT');
echo "Columns added successfully\n";
?>
