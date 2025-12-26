<?php
/**
 * 每日古诗插件
 * 从网络接口获取古诗数据，并转换为适合万年历URL插件的格式
 */

// 设置响应头
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

// 网络古诗接口 - 这里使用的是一个公共的古诗API示例
$poemApiUrl = 'https://v1.jinrishici.com/all';

// 初始化cURL
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $poemApiUrl);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_TIMEOUT, 10);
curl_setopt($ch, CURLOPT_HTTPHEADER, [
    'Content-Type: application/json',
    'X-User-Token: 你的API令牌' // 如果需要API令牌，请替换这里
]);

// 执行请求
$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
curl_close($ch);

// 检查请求是否成功
if ($httpCode !== 200) {
    // 如果请求失败，使用备用数据
    $data = [
        'content' => '春眠不觉晓，处处闻啼鸟。夜来风雨声，花落知多少。',
        'author' => '孟浩然',
        'title' => '春晓'
    ];
} else {
    // 解析响应数据
    $result = json_decode($response, true);
    $data = [
        'content' => $result['content'],
        'author' => $result['author'],
        'title' => $result['origin']
    ];
}

// 转换为适合万年历URL插件的格式
$pluginData = [
    'status' => 'success',
    'data' => [
        'title' => '每日古诗',
        'content' => $data['content'],
        'author' => $data['author'],
        'source' => $data['title'],
        'update_time' => date('Y-m-d H:i:s')
    ]
];

echo json_encode($pluginData, JSON_UNESCAPED_UNICODE);
?>