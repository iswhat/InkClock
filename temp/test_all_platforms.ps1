# InkClock 所有平台固件编译测试脚本
# 用于测试真实固件在不同平台上的编译情况

# 设置工作目录
$WORKDIR = "d:/InkClock"
$CODEDIR = "$WORKDIR/code"

# 设置日志文件
$LOGFILE = "$WORKDIR/firmware_compile_test.log"

# 清空旧日志
Clear-Content -Path $LOGFILE -ErrorAction SilentlyContinue

# 写入日志函数
function Write-Log {
    param(
        [string]$Message,
        [string]$Level = "INFO"
    )
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $LogEntry = "[$Timestamp] [$Level] $Message"
    Write-Host $LogEntry
    Add-Content -Path $LOGFILE -Value $LogEntry
}

# 测试结果
$TestResults = @{}

# 所有支持的平台环境列表
$Environments = @(
    "esp32-c3-devkitc-02",
    "esp32-wroom-32",
    "esp32-s3-devkitc-1",
    "esp32-c6-n4",
    "esp32-s2-devkitc-1",
    "esp32-c3-supermini",
    "nodemcuv2",
    "nrf52840dk",
    "bluepill_f103c8",
    "raspberrypi_pico"
)

Write-Log "开始InkClock真实固件编译测试" "INFO"
Write-Log "工作目录: $WORKDIR" "INFO"
Write-Log "代码目录: $CODEDIR" "INFO"
Write-Log "日志文件: $LOGFILE" "INFO"
Write-Log "支持的平台数量: $($Environments.Count)" "INFO"
Write-Log "=======================================" "INFO"

# 进入代码目录
Set-Location -Path $CODEDIR

# 1. 清理旧的构建文件
Write-Log "1. 清理旧的构建文件..." "INFO"
try {
    Invoke-Expression "platformio run -t clean" -ErrorAction Stop
    Write-Log "清理构建文件成功！" "SUCCESS"
} catch {
    Write-Log "清理构建文件失败: $($_.Exception.Message)" "ERROR"
}
Write-Log "=======================================" "INFO"

# 2. 编译每个平台的固件
$TotalPlatforms = $Environments.Count
$SuccessCount = 0
$FailedCount = 0

for ($i = 0; $i -lt $TotalPlatforms; $i++) {
    $EnvName = $Environments[$i]
    $PlatformIndex = $i + 1
    
    Write-Log "$PlatformIndex/$TotalPlatforms. 编译平台: $EnvName" "INFO"
    
    try {
        # 编译固件
        $Output = Invoke-Expression "platformio run -e $EnvName" -ErrorAction Stop
        
        # 检查输出中是否包含成功信息
        if ($Output -match "SUCCESS") {
            Write-Log "平台 $EnvName 编译成功！" "SUCCESS"
            $TestResults[$EnvName] = "SUCCESS"
            $SuccessCount++
        } else {
            Write-Log "平台 $EnvName 编译可能失败，输出中未找到成功标识" "WARNING"
            $TestResults[$EnvName] = "UNKNOWN"
            $FailedCount++
        }
    } catch {
        Write-Log "平台 $EnvName 编译失败: $($_.Exception.Message)" "ERROR"
        $TestResults[$EnvName] = "FAILED"
        $FailedCount++
    }
    
    Write-Log "=======================================" "INFO"
}

# 3. 生成测试报告
Write-Log "开始生成测试报告..." "INFO"
Write-Log "=======================================" "INFO"
Write-Log "测试结果汇总" "INFO"
Write-Log "=======================================" "INFO"
Write-Log "测试平台总数: $TotalPlatforms" "INFO"
Write-Log "成功平台数: $SuccessCount" "SUCCESS"
Write-Log "失败平台数: $FailedCount" "ERROR"
Write-Log "未知状态平台数: $($TotalPlatforms - $SuccessCount - $FailedCount)" "WARNING"
Write-Log "=======================================" "INFO"
Write-Log "各平台详细结果:" "INFO"
Write-Log "=======================================" "INFO"

foreach ($EnvName in $Environments) {
    $Result = $TestResults[$EnvName]
    switch ($Result) {
        "SUCCESS" {
            Write-Log "$EnvName: 编译成功" "SUCCESS"
        }
        "FAILED" {
            Write-Log "$EnvName: 编译失败" "ERROR"
        }
        "UNKNOWN" {
            Write-Log "$EnvName: 状态未知" "WARNING"
        }
        default {
            Write-Log "$EnvName: 未测试" "WARNING"
        }
    }
}

Write-Log "=======================================" "INFO"
Write-Log "固件编译测试完成！" "INFO"
Write-Log "测试结果已保存到: $LOGFILE" "INFO"
Write-Log "=======================================" "INFO"

# 4. 显示编译成功的固件文件位置
Write-Log "编译成功的固件文件位置:" "INFO"
foreach ($EnvName in $Environments) {
    if ($TestResults[$EnvName] -eq "SUCCESS") {
        $FirmwarePath = "$CODEDIR/.pio/build/$EnvName/firmware.bin"
        if (Test-Path $FirmwarePath) {
            Write-Log "$EnvName: $FirmwarePath" "INFO"
        }
    }
}

Write-Log "=======================================" "INFO"
Write-Log "所有测试完成！" "INFO"

# 返回原始目录
Set-Location -Path $WORKDIR
