<?php
/**
 * InkClock Webserver 自动化测试脚本
 * 测试所有主要 API 端点
 */

$baseUrl = 'http://localhost:8080';
$testResults = [];
$totalTests = 0;
$passedTests = 0;
$failedTests = 0;

// 颜色输出函数
function colorize($text, $status) {
    $colors = [
        'success' => "\033[32m",  // 绿色
        'error' => "\033[31m",    // 红色
        'warning' => "\033[33m",  // 黄色
        'info' => "\033[36m",     // 青色
        'reset' => "\033[0m"
    ];
    return $colors[$status] . $text . $colors['reset'];
}

// 测试函数
function runTest($name, $method, $url, $data = null, $expectedStatus = 200) {
    global $baseUrl, $totalTests, $passedTests, $failedTests;
    
    $totalTests++;
    $fullUrl = $baseUrl . $url;
    
    $options = [
        'http' => [
            'method' => $method,
            'header' => "Content-Type: application/json\r\n",
            'timeout' => 10
        ]
    ];
    
    if ($data !== null) {
        $options['http']['content'] = json_encode($data);
    }
    
    $context = stream_context_create($options);
    $result = @file_get_contents($fullUrl, false, $context);
    $httpCode = $http_response_header[0] ?? 'Unknown';
    preg_match('/HTTP\/\d\.\d\s+(\d+)/', $httpCode, $matches);
    $statusCode = $matches[1] ?? 0;
    
    $passed = ($statusCode == $expectedStatus);
    if ($passed) {
        $passedTests++;
    } else {
        $failedTests++;
    }
    
    return [
        'name' => $name,
        'method' => $method,
        'url' => $url,
        'expected' => $expectedStatus,
        'actual' => $statusCode,
        'passed' => $passed,
        'response' => $result
    ];
}

// 打印结果
function printResult($result) {
    $status = $result['passed'] ? colorize('✓ PASS', 'success') : colorize('✗ FAIL', 'error');
    echo sprintf("%-40s %s [%d/%d]\n", 
        $result['name'], 
        $status, 
        $result['expected'], 
        $result['actual']
    );
    
    if (!$result['passed'] && $result['response']) {
        echo "  Response: " . substr($result['response'], 0, 200) . "\n";
    }
}

echo colorize("\n========================================\n", 'info');
echo colorize("InkClock Webserver 自动化测试\n", 'info');
echo colorize("========================================\n\n", 'info');

// 1. 健康检查测试
echo colorize("1. 健康检查测试\n", 'warning');
$testResults[] = runTest('健康检查', 'GET', '/health', null, 200);
$testResults[] = runTest('API 健康检查', 'GET', '/api/health', null, 200);
$testResults[] = runTest('状态检查', 'GET', '/api/status', null, 200);

// 2. 用户登录测试
echo colorize("\n2. 用户认证测试\n", 'warning');
$testResults[] = runTest('登录 - 空凭证', 'POST', '/api/user/login', [], 400);
$testResults[] = runTest('登录 - 错误密码', 'POST', '/api/user/login', ['username' => 'admin', 'password' => 'wrong'], 401);

// 3. 配置管理测试
echo colorize("\n3. 配置管理测试\n", 'warning');
$testResults[] = runTest('获取配置', 'GET', '/api/config', null, 200);

// 4. 静态资源测试
echo colorize("\n4. 静态资源测试\n", 'warning');
$testResults[] = runTest('首页', 'GET', '/', null, 200);
$testResults[] = runTest('登录页面', 'GET', '/login.html', null, 200);
$testResults[] = runTest('配置页面', 'GET', '/config.html', null, 200);

// 打印所有结果
echo colorize("\n========================================\n", 'info');
echo colorize("测试结果详情:\n", 'info');
echo colorize("========================================\n", 'info');
foreach ($testResults as $result) {
    printResult($result);
}

// 总结
echo colorize("\n========================================\n", 'info');
echo colorize("测试总结:\n", 'info');
echo colorize("========================================\n", 'info');
echo colorize("总测试数：$totalTests\n", 'info');
echo colorize("通过：$passedTests\n", 'success');
echo colorize("失败：$failedTests\n", $failedTests > 0 ? 'error' : 'success');
echo colorize("========================================\n\n", 'info');

exit($failedTests > 0 ? 1 : 0);
?>
