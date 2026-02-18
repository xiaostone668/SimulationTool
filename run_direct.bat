@echo off
echo =========================================
echo Simulation Tool - Direct Runner
echo =========================================
echo.

REM Go to the release directory where the executable is
cd /d "%~dp0\build_cmake\Release"

echo Current directory:
cd
echo.

echo Checking critical files...
if exist "SimulationTool.exe" (
    echo ✓ SimulationTool.exe found
) else (
    echo ✗ SimulationTool.exe not found
    pause
    exit /b 1
)

if exist "Qt6Core.dll" (
    echo ✓ Qt6Core.dll found
) else (
    echo ✗ Qt6Core.dll not found
)

if exist "Qt6Gui.dll" (
    echo ✓ Qt6Gui.dll found
) else (
    echo ✗ Qt6Gui.dll not found
)

if exist "Qt6Widgets.dll" (
    echo ✓ Qt6Widgets.dll found
) else (
    echo ✗ Qt6Widgets.dll not found
)

if exist "Qt6OpenGL.dll" (
    echo ✓ Qt6OpenGL.dll found
) else (
    echo ✗ Qt6OpenGL.dll not found
)

if exist "platforms\qwindows.dll" (
    echo ✓ platforms\qwindows.dll found
) else (
    echo ✗ platforms\qwindows.dll not found
)

echo.
echo Setting environment variables...
set QT_QPA_PLATFORM_PLUGIN_PATH=%CD%\platforms
echo QT_QPA_PLATFORM_PLUGIN_PATH=%QT_QPA_PLATFORM_PLUGIN_PATH%

REM Add Qt bin to PATH temporarily
set QT_PATH=D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin
set PATH=%QT_PATH%;%PATH%

echo.
echo Running SimulationTool.exe...
echo.
SimulationTool.exe

echo.
echo Exit code: %ERRORLEVEL%
pause