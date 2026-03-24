# VS Code Configuration Script
Write-Host "Checking configuration files..." -ForegroundColor Cyan

$configFile = ".vscode\c_cpp_properties.json"
$arduinoCore = "$env:USERPROFILE\.platformio\packages\framework-arduinoespressif32\cores\esp32"

if (Test-Path $configFile) {
    Write-Host "[OK] c_cpp_properties.json exists" -ForegroundColor Green
} else {
    Write-Host "[ERROR] c_cpp_properties.json not found!" -ForegroundColor Red
    exit 1
}

if (Test-Path "$arduinoCore\Arduino.h") {
    Write-Host "[OK] Arduino.h found at: $arduinoCore" -ForegroundColor Green
} else {
    Write-Host "[WARNING] Arduino.h not found. Check PlatformIO installation." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Configuration complete!" -ForegroundColor Green
Write-Host "Please reload VS Code window (Ctrl+Shift+P, then 'Reload Window')" -ForegroundColor Yellow
