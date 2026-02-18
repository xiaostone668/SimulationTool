$releaseDir = 'd:\MyAICADCode\SimulationTool\build_cmake\Release'

# Ensure required DLLs are present (they may have been overwritten by cmake post-build)
$sdkDlls = @{
    'openvr_api.dll'  = 'D:\SDK\3rdparty-vc14-64\openvr-1.14.15-64\bin\win64\openvr_api.dll'
    'FreeImage.dll'   = 'D:\SDK\3rdparty-vc14-64\freeimage-3.18.0-x64\bin\FreeImage.dll'
    'avcodec-57.dll'  = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avcodec-57.dll'
    'avformat-57.dll' = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avformat-57.dll'
    'swscale-4.dll'   = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\swscale-4.dll'
    'avutil-55.dll'   = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avutil-55.dll'
}

foreach ($dll in $sdkDlls.Keys) {
    $dest = Join-Path $releaseDir $dll
    if (-not (Test-Path $dest)) {
        Copy-Item $sdkDlls[$dll] $dest -Force
        Write-Host "Copied: $dll"
    } else {
        Write-Host "OK: $dll"
    }
}

# Set environment
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "$releaseDir\platforms"
Set-Location $releaseDir

Write-Host ""
Write-Host "Launching SimulationTool.exe..."
Start-Process -FilePath "$releaseDir\SimulationTool.exe"
Start-Sleep -Seconds 2

$proc = Get-Process SimulationTool -ErrorAction SilentlyContinue
if ($proc) {
    Write-Host "SUCCESS: SimulationTool.exe is running (PID $($proc.Id))"
} else {
    Write-Host "ERROR: SimulationTool.exe failed to start or exited immediately"
}
