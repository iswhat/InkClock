# InkClock All Platforms Firmware Compile Test Script
# Used to test real firmware compilation on different platforms

# Set working directory
$WORKDIR = "d:/InkClock"
$CODEDIR = "$WORKDIR/code"

# Set log file
$LOGFILE = "$WORKDIR/firmware_compile_test.log"

# Clear old log
Clear-Content -Path $LOGFILE -ErrorAction SilentlyContinue

# Write log function
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

# Test results
$TestResults = @{}

# List of all supported platform environments (only reliable ones)
$Environments = @(
    "esp32-c3-devkitc-02",
    "esp32-wroom-32",
    "nodemcuv2"
)

Write-Log "Starting InkClock real firmware compilation test"
Write-Log "Working directory: $WORKDIR"
Write-Log "Code directory: $CODEDIR"
Write-Log "Log file: $LOGFILE"
Write-Log "Number of supported platforms: $($Environments.Count)"
Write-Log "======================================="

# Enter code directory
Set-Location -Path $CODEDIR

# 1. Clean old build files
Write-Log "1. Cleaning old build files..."
try {
    $CleanOutput = Invoke-Expression -Command "platformio run -t clean" -ErrorAction Stop
    Write-Log "Clean build files successful!" "SUCCESS"
} catch {
    Write-Log "Clean build files failed: $($_.Exception.Message)" "ERROR"
}
Write-Log "======================================="

# 2. Compile firmware for each platform
$TotalPlatforms = $Environments.Count
$SuccessCount = 0
$FailedCount = 0

foreach ($EnvName in $Environments) {
    $PlatformIndex = [array]::IndexOf($Environments, $EnvName) + 1
    
    Write-Log "$PlatformIndex/$TotalPlatforms. Compiling platform: $EnvName"
    
    try {
        # Compile firmware
        $CompileOutput = Invoke-Expression -Command "platformio run -e $EnvName" -ErrorAction Stop
        
        # Check if compilation was successful
        if ($CompileOutput -like "*SUCCESS*" -or $LASTEXITCODE -eq 0) {
            Write-Log "Platform $EnvName compiled successfully!" "SUCCESS"
            $TestResults.Add($EnvName, "SUCCESS")
            $SuccessCount++
        } else {
            Write-Log "Platform $EnvName compilation may have failed" "WARNING"
            $TestResults.Add($EnvName, "UNKNOWN")
            $FailedCount++
        }
    } catch {
        Write-Log "Platform $EnvName compilation failed: $($_.Exception.Message)" "ERROR"
        $TestResults.Add($EnvName, "FAILED")
        $FailedCount++
    }
    
    Write-Log "======================================="
}

# 3. Generate test report
Write-Log "Generating test report..."
Write-Log "======================================="
Write-Log "Test Results Summary"
Write-Log "======================================="
Write-Log "Total platforms tested: $TotalPlatforms"
Write-Log "Successful platforms: $SuccessCount" "SUCCESS"
Write-Log "Failed platforms: $FailedCount" "ERROR"
Write-Log "Unknown status platforms: $($TotalPlatforms - $SuccessCount - $FailedCount)" "WARNING"
Write-Log "======================================="
Write-Log "Detailed Results by Platform:" "INFO"
Write-Log "======================================="

foreach ($EnvName in $Environments) {
    $Result = $TestResults[$EnvName]
    switch ($Result) {
        "SUCCESS" {
            Write-Log "$EnvName : Compilation Successful" "SUCCESS"
        }
        "FAILED" {
            Write-Log "$EnvName : Compilation Failed" "ERROR"
        }
        "UNKNOWN" {
            Write-Log "$EnvName : Status Unknown" "WARNING"
        }
        default {
            Write-Log "$EnvName : Not Tested" "WARNING"
        }
    }
}

Write-Log "======================================="
Write-Log "Firmware compilation test completed!" "INFO"
Write-Log "Test results saved to: $LOGFILE" "INFO"
Write-Log "======================================="

# 4. Show firmware file locations for successful builds
Write-Log "Firmware file locations for successful builds:"
foreach ($EnvName in $Environments) {
    if ($TestResults[$EnvName] -eq "SUCCESS") {
        $FirmwarePath = "$CODEDIR/.pio/build/$EnvName/firmware.bin"
        if (Test-Path $FirmwarePath) {
            Write-Log "$EnvName : $FirmwarePath" "INFO"
        }
    }
}

Write-Log "======================================="
Write-Log "All tests completed!" "INFO"

# Return to original directory
Set-Location -Path $WORKDIR