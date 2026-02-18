$releaseDir = 'd:\MyAICADCode\SimulationTool\build_cmake\Release'

# Verify DLLs
$dlls = @('openvr_api.dll','FreeImage.dll','avcodec-57.dll','avformat-57.dll','swscale-4.dll','avutil-55.dll')
foreach ($dll in $dlls) {
    $p = Join-Path $releaseDir $dll
    if (Test-Path $p) {
        Write-Host "[OK] $dll"
    } else {
        Write-Host "[MISSING] $dll - copying..."
        # Try to copy from SDK
        $srcMap = @{
            'openvr_api.dll' = 'D:\SDK\3rdparty-vc14-64\openvr-1.14.15-64\bin\win64\openvr_api.dll'
            'FreeImage.dll' = 'D:\SDK\3rdparty-vc14-64\freeimage-3.18.0-x64\bin\FreeImage.dll'
            'avcodec-57.dll' = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avcodec-57.dll'
            'avformat-57.dll' = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avformat-57.dll'
            'swscale-4.dll' = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\swscale-4.dll'
            'avutil-55.dll' = 'D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avutil-55.dll'
        }
        if ($srcMap.ContainsKey($dll)) {
            Copy-Item $srcMap[$dll] $p -Force
            Write-Host "  -> Copied from SDK"
        }
    }
}

Write-Host ""
Write-Host "Running SimulationTool.exe..."

Set-Location $releaseDir
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "$releaseDir\platforms"

$proc = Start-Process -FilePath "$releaseDir\SimulationTool.exe" -PassThru -Wait
Write-Host "Exit code: $($proc.ExitCode)"
if ($proc.ExitCode -ne 0) {
    Write-Host "Application failed with exit code: $($proc.ExitCode) (0x$(([uint32]$proc.ExitCode).ToString('X8')))"
}
