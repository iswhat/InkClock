<?php
/**
 * 测试运行器，用于运行所有测试用例
 */

// 设置错误报告
error_reporting(E_ALL);
ini_set('display_errors', 1);

// 加载测试基类
require_once __DIR__ . '/TestCase.php';

// 加载项目核心文件
require_once __DIR__ . '/../utils/Database.php';
require_once __DIR__ . '/../utils/Logger.php';
require_once __DIR__ . '/../utils/Response.php';

// 加载配置
require_once __DIR__ . '/../config/config.php';

// 初始化日志
$logger = Logger::getInstance();
$logger->setLevel('error');

// 查找所有测试文件
$testFiles = glob(__DIR__ . '/*Test.php');

if (empty($testFiles)) {
    echo "未找到测试文件\n";
    exit(0);
}

$allResults = [];

// 运行所有测试
foreach ($testFiles as $testFile) {
    require_once $testFile;
    
    // 获取测试类名
    $testClassName = pathinfo($testFile, PATHINFO_FILENAME);
    
    // 创建测试实例并运行
    $test = new $testClassName($testClassName);
    $test->run();
    
    // 记录结果
    $allResults[] = $test->getResults();
}

// 生成总报告
echo "\n\n=== 所有测试结果汇总 ===\n";

$totalTests = 0;
$totalPassed = 0;
$totalFailed = 0;

foreach ($allResults as $result) {
    $totalTests += $result['total'];
    $totalPassed += $result['passed'];
    $totalFailed += $result['failed'];
    
    echo "{$result['name']}: {$result['passed']}/{$result['total']} 通过 (" . round(($result['passed_rate'] * 100), 2) . "%)\n";
}

echo "\n总测试数: {$totalTests}\n";
echo "总通过数: {$totalPassed}\n";
echo "总失败数: {$totalFailed}\n";

echo "\n=== 测试完成 ===\n";

// 返回退出码
exit($totalFailed > 0 ? 1 : 0);
?>