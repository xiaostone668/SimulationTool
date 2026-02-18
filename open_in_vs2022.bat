@echo off
echo =========================================
echo 用 Visual Studio 2022 打开 SimulationTool
echo =========================================
echo.

set SLN_PATH=%~dp0build_cmake\SimulationTool.sln

if not exist "%SLN_PATH%" (
    echo 未找到解决方案文件，正在重新生成...
    cmake -S "%~dp0" -B "%~dp0build_cmake" -G "Visual Studio 17 2022" -A x64
    if errorlevel 1 (
        echo CMake 配置失败，请检查环境
        pause
        exit /b 1
    )
)

echo 解决方案路径: %SLN_PATH%
echo.
echo 正在打开 Visual Studio 2022...
start "" "%SLN_PATH%"
echo.
echo 在 VS 2022 中操作步骤:
echo   1. 顶部菜单: 生成 ^> 生成解决方案 (Ctrl+Shift+B)
echo   2. 按 F5 即可编译并运行 (Release 或 Debug 配置均可)
echo   3. 所有 DLL 路径已在 SimulationTool.vcxproj.user 中预配置
echo.
pause
