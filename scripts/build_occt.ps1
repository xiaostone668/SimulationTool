# 从源码编译 OCCT（含 STEP），并可选安装到指定目录
# 用法: .\build_occt.ps1 [-OCC_SRC path] [-OCC_INSTALL path] [-3RDPARTY path] [-SkipClone] [-SkipBuild]

param(
    [string]$OCC_SRC     = "D:\MyAICADCode\occt-src",
    [string]$OCC_BUILD    = "D:\MyAICADCode\occt-build",
    [string]$OCC_INSTALL  = "D:\SDK\occt-7.9.3-vc14-64-full",
    [string]$3RDPARTY    = "D:\SDK\occt-3rdparty-vc14-64",
    [string]$Generator    = "Visual Studio 17 2022 -A x64",  # 默认 VS2022；VS2019 用 "Visual Studio 16 2019 -A x64"
    [switch]$SkipClone,
    [switch]$SkipBuild,
    [switch]$InstallOnly
)

$ErrorActionPreference = "Stop"

Write-Host "=== OCCT 源码编译脚本 (含 STEP) ===" -ForegroundColor Cyan
Write-Host "  OCC_SRC     = $OCC_SRC"
Write-Host "  OCC_BUILD   = $OCC_BUILD"
Write-Host "  OCC_INSTALL = $OCC_INSTALL"
Write-Host "  3RDPARTY    = $3RDPARTY"
Write-Host ""

# CMake 生成器：默认 VS2022，64 位
$GenName = "Visual Studio 17 2022"
$GenArch = "x64"
if ($Generator -match "16 2019") { $GenName = "Visual Studio 16 2019" }
if ($Generator -match "15 2017") { $GenName = "Visual Studio 15 2017" }

if (-not $SkipClone) {
    if (-not (Test-Path "$OCC_SRC\CMakeLists.txt")) {
        Write-Host "正在克隆 OCCT 源码 (V7_9_0)..." -ForegroundColor Yellow
        $parent = Split-Path -Parent $OCC_SRC
        if (-not (Test-Path $parent)) { New-Item -ItemType Directory -Path $parent -Force | Out-Null }
        if (Test-Path $OCC_SRC) { Remove-Item -Recurse -Force $OCC_SRC }
        git clone --depth 1 --branch V7_9_0 "https://github.com/Open-Cascade-SAS/OCCT.git" $OCC_SRC
        if (-not (Test-Path "$OCC_SRC\CMakeLists.txt")) { throw "克隆后未找到 CMakeLists.txt" }
    } else {
        Write-Host "已存在源码目录，跳过克隆。" -ForegroundColor Green
    }
}

if (-not (Test-Path $3RDPARTY)) {
    Write-Host "错误: 未找到第三方库目录: $3RDPARTY" -ForegroundColor Red
    Write-Host "请从 https://dev.opencascade.org/resources/download/3rd-party-components 下载 vc14 64bit 的 Freetype，解压到该目录下。" -ForegroundColor Red
    exit 1
}

if (-not $InstallOnly) {
    New-Item -ItemType Directory -Force -Path $OCC_BUILD | Out-Null
    Push-Location $OCC_BUILD

    Write-Host "正在配置 CMake..." -ForegroundColor Yellow
    & cmake -S $OCC_SRC -B $OCC_BUILD `
        -G $GenName -A $GenArch `
        -DCMAKE_INSTALL_PREFIX="$OCC_INSTALL" `
        -D3RDPARTY_DIR="$3RDPARTY" `
        -DBUILD_MODULE_Draw=OFF `
        -DBUILD_DOC_Overview=OFF `
        -DBUILD_SAMPLES_MFC=OFF `
        -DBUILD_SAMPLES_QT=OFF `
        -DUSE_FREETYPE=ON

    if ($LASTEXITCODE -ne 0) { Pop-Location; exit $LASTEXITCODE }

    if (-not $SkipBuild) {
        Write-Host "正在编译 (Release)，请稍候..." -ForegroundColor Yellow
        & cmake --build $OCC_BUILD --config Release
        if ($LASTEXITCODE -ne 0) { Pop-Location; exit $LASTEXITCODE }
        Pop-Location
    } else {
        Pop-Location
    }
}

Write-Host "正在安装到 $OCC_INSTALL ..." -ForegroundColor Yellow
& cmake --install $OCC_BUILD --config Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "=== 完成 ===" -ForegroundColor Green
Write-Host "请使用以下命令重新配置并编译 SimulationTool:"
Write-Host "  cd D:\MyAICADCode\SimulationTool\build"
Write-Host "  Remove-Item CMakeCache.txt -ErrorAction SilentlyContinue"
Write-Host "  cmake .. -DOCC_ROOT=$($OCC_INSTALL -replace '\\','/')"
Write-Host "  cmake --build . --config Release"
