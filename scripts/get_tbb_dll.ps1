# 获取 tbb12.dll（OCCT 运行时依赖）
# 用法: .\get_tbb_dll.ps1 [-OutputDir "D:\SDK\tbb-dll"]
# 完成后在 CMake 中设置: -DOCC_EXTRA_DLL_DIR=D:/SDK/tbb-dll

param(
    [string]$OutputDir = "D:\SDK\tbb-dll"
)

$ErrorActionPreference = "Stop"
$tbbUrl = "https://github.com/oneapi-src/oneTBB/releases/download/v2021.11.0/oneTBB-2021.11.0-win.zip"
$tempZip = [System.IO.Path]::GetTempFileName() + ".zip"

Write-Host "目标目录: $OutputDir" -ForegroundColor Cyan

# 1) 若已存在 tbb12.dll 则跳过
if (Test-Path "$OutputDir\tbb12.dll") {
    Write-Host "已存在 tbb12.dll，跳过下载。" -ForegroundColor Green
    Write-Host "CMake 配置示例: cmake .. -DOCC_EXTRA_DLL_DIR=$($OutputDir -replace '\\','/')"
    exit 0
}

# 2) 尝试 vcpkg
$vcpkg = $env:VCPKG_ROOT
if (-not $vcpkg) { $vcpkg = "D:\vcpkg"; if (-not (Test-Path $vcpkg)) { $vcpkg = "$env:USERPROFILE\vcpkg" } }
if (Test-Path "$vcpkg\installed\x64-windows\bin\tbb12.dll") {
    Write-Host "从 vcpkg 复制: $vcpkg\installed\x64-windows\bin" -ForegroundColor Yellow
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
    Copy-Item "$vcpkg\installed\x64-windows\bin\tbb12.dll" -Destination $OutputDir -Force
    Write-Host "完成。CMake 配置: cmake .. -DOCC_EXTRA_DLL_DIR=$($OutputDir -replace '\\','/')" -ForegroundColor Green
    exit 0
}

# 3) 下载 oneTBB
Write-Host "正在下载 oneTBB ..." -ForegroundColor Yellow
try {
    Invoke-WebRequest -Uri $tbbUrl -OutFile $tempZip -UseBasicParsing
} catch {
    Write-Host "下载失败: $_" -ForegroundColor Red
    Write-Host "请手动从 https://github.com/oneapi-src/oneTBB/releases 下载 Windows 版，将 tbb12.dll 放入 $OutputDir"
    exit 1
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$extractDir = [System.IO.Path]::GetTempPath() + [Guid]::NewGuid().ToString("N")
Expand-Archive -Path $tempZip -DestinationPath $extractDir -Force
Remove-Item $tempZip -ErrorAction SilentlyContinue

$dll = Get-ChildItem -Path $extractDir -Recurse -Filter "tbb12.dll" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($dll) {
    Copy-Item $dll.FullName -Destination $OutputDir -Force
    Write-Host "已复制 tbb12.dll 到 $OutputDir" -ForegroundColor Green
} else {
    Write-Host "解压包中未找到 tbb12.dll，请检查 oneTBB 版本或手动放置 DLL 到 $OutputDir" -ForegroundColor Red
}
Remove-Item -Recurse -Force $extractDir -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "重新配置并编译: cmake .. -DOCC_EXTRA_DLL_DIR=$($OutputDir -replace '\\','/')" -ForegroundColor Cyan
Write-Host "              cmake --build . --config Release"
