# 启动 GeomProcessor 并设置环境变量
$geomDir = "d:\MyAICADCode\GeomProcessor\build\Release"
if (!(Test-Path "$geomDir\GeomProcessor.exe")) {
    Write-Host "Error: GeomProcessor.exe not found at $geomDir"
    exit 1
}

# 设置 Qt 插件路径
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "$geomDir\platforms"

Write-Host "Starting GeomProcessor.exe..."
Start-Process -FilePath "$geomDir\GeomProcessor.exe" -WorkingDirectory $geomDir
Write-Host "GeomProcessor started."