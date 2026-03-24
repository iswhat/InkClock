<?php
$uri = urldecode(parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH));
$requestedFile = __DIR__ . $uri;

if ($uri !== '/' && file_exists($requestedFile)) {
    return false;
}

include_once __DIR__ . '/index.php';
?>
