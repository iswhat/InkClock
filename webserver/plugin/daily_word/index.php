<?php
/**
 * 每日英语单词插件
 * 从网络接口获取英语单词数据，并转换为适合万年历URL插件的格式
 */

// 设置响应头
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

// 网络英语单词接口 - 这里使用的是一个公共的英语单词API示例
$wordApiUrl = 'https://api.dictionaryapi.dev/api/v2/entries/en/random';

// 初始化cURL
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $wordApiUrl);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_TIMEOUT, 10);

// 执行请求
$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
curl_close($ch);

// 检查请求是否成功
if ($httpCode !== 200) {
    // 如果请求失败，使用备用数据
    $data = [
        'word' => 'Hello',
        'phonetic' => '/həˈləʊ/',
        'definition' => '用于问候或引起注意',
        'example' => 'Hello, how are you today?',
        'partOfSpeech' => '感叹词'
    ];
} else {
    // 解析响应数据
    $result = json_decode($response, true);
    if (isset($result[0])) {
        $wordData = $result[0];
        $phonetic = isset($wordData['phonetic']) ? $wordData['phonetic'] : '';
        $definition = '';
        $example = '';
        $partOfSpeech = '';
        
        // 获取第一个定义
        if (isset($wordData['meanings'][0])) {
            $meaning = $wordData['meanings'][0];
            $partOfSpeech = $meaning['partOfSpeech'];
            
            if (isset($meaning['definitions'][0])) {
                $definition = $meaning['definitions'][0]['definition'];
                if (isset($meaning['definitions'][0]['example'])) {
                    $example = $meaning['definitions'][0]['example'];
                }
            }
        }
        
        $data = [
            'word' => $wordData['word'],
            'phonetic' => $phonetic,
            'definition' => $definition,
            'example' => $example,
            'partOfSpeech' => $partOfSpeech
        ];
    } else {
        // 使用备用数据
        $data = [
            'word' => 'Hello',
            'phonetic' => '/həˈləʊ/',
            'definition' => '用于问候或引起注意',
            'example' => 'Hello, how are you today?',
            'partOfSpeech' => '感叹词'
        ];
    }
}

// 转换为适合万年历URL插件的格式
$pluginData = [
    'status' => 'success',
    'data' => [
        'title' => '每日英语单词',
        'word' => $data['word'],
        'phonetic' => $data['phonetic'],
        'definition' => $data['definition'],
        'example' => $data['example'],
        'part_of_speech' => $data['partOfSpeech'],
        'update_time' => date('Y-m-d H:i:s')
    ]
];

echo json_encode($pluginData, JSON_UNESCAPED_UNICODE);
?>