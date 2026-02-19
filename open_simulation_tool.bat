@echo off
echo =========================================
echo Opening SimulationTool
echo =========================================
echo.

REM Set Qt DLL path
set QT_PATH=D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin
set PATH=%QT_PATH%;%PATH%

REM Set Qt platform plugin path
set QT_QPA_PLATFORM_PLUGIN_PATH=platforms

REM Change to Debug directory where the executable is
cd /d "%~dp0\build_cmake\Debug"

REM Check if executable exists
if not exist "SimulationTool.exe" (
    echo Error: SimulationTool.exe not found!
    echo Please build the project first.
    pause
    exit /b 1
)

echo Found SimulationTool.exe
echo Starting SimulationTool...
echo.

REM Run the application
start SimulationTool.exe

echo SimulationTool should now be running.
echo.