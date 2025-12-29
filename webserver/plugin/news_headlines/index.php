<?php
/**
 * 新闻头条插件
 * 获取最新新闻头条，并转换为适合万年历URL插件的格式
 */

// 设置响应头
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

// 这里可以使用任何新闻API，现在使用模拟数据
// $newsApiUrl = 'https://api.example.com/news/headlines';

// 模拟新闻数据
$newsData = [
    [
        'title' => '中国成功发射新一代通信卫星',
        'source' => '央视新闻',
        'url' => 'https://news.cctv.com/2025/12/29/ARTI1234567890.shtml',
        'time' => date('Y-m-d H:i:s', strtotime('-1 hour'))
    ],
    [
        'title' => '全球人工智能大会在上海开幕',
        'source' => '新华网',
        'url' => 'http://www.xinhuanet.com/tech/2025-12/29/c_1130789012.htm',
        'time' => date('Y-m-d H:i:s', strtotime('-2 hours'))
    ],
    [
        'title' => '新型环保材料研发取得重大突破',
        'source' => '科技日报',
        'url' => 'http://digitalpaper.stdaily.com/http_www.kjrb.com/kjrb/html/2025-12/29/content_530000.htm',
        'time' => date('Y-m-d H:i:s', strtotime('-3 hours'))
    ]
];

// 转换为适合万年历URL插件的格式
$pluginData = [
    'status' => 'success',
    'data' => [
        'title' => '新闻头条',
        'news' => $newsData,
        'update_time' => date('Y-m-d H:i:s')
    ]
];

echo json_encode($pluginData, JSON_UNESCAPED_UNICODE);
?>