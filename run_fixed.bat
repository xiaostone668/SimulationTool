@echo off
echo =========================================
echo Simulation Tool - Fixed Runner
echo =========================================
echo.

REM Set current directory to script location
cd /d "%~dp0"

REM Set Qt DLL path
set QT_PATH=D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin
set PATH=%QT_PATH%;%PATH%

REM Set Qt platform plugin path
set QT_QPA_PLATFORM_PLUGIN_PATH=%CD%\build_cmake\Release\platforms

REM Check for executable
if not exist "build_cmake\Release\SimulationTool.exe" (
    echo Error: SimulationTool.exe not found
    echo Please build the project first using CMake
    echo Run: mkdir build_cmake && cd build_cmake && cmake .. -G "Visual Studio 17 2022" -A x64
    echo Then: cd build_cmake && cmake --build . --config Release
    pause
    exit /b 1
)

REM Copy required third-party DLLs
echo Copying required DLLs...
set SDK_PATH=D:\SDK\3rdparty-vc14-64

REM TBB DLLs
copy "%SDK_PATH%\tbb-2021.13.0-x64\bin\tbb12.dll" "build_cmake\Release\" >nul 2>&1
copy "%SDK_PATH%\tbb-2021.13.0-x64\bin\tbbmalloc.dll" "build_cmake\Release\" >nul 2>&1

REM jemalloc
copy "%SDK_PATH%\jemalloc-vc14-64\bin\jemalloc.dll" "build_cmake\Release\" >nul 2>&1

REM freetype
copy "%SDK_PATH%\freetype-2.13.3-x64\bin\freetype.dll" "build_cmake\Release\" >nul 2>&1

REM Copy essential Qt DLLs to program directory
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6Core.dll" "build_cmake\Release\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6Gui.dll" "build_cmake\Release\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6Widgets.dll" "build_cmake\Release\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6OpenGL.dll" "build_cmake\Release\" >nul 2>&1

REM Copy Qt platform plugins
echo Copying Qt platform plugins...
if not exist "build_cmake\Release\platforms" mkdir "build_cmake\Release\platforms"
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\plugins\platforms\qwindows.dll" "build_cmake\Release\platforms\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\plugins\platforms\qminimal.dll" "build_cmake\Release\platforms\" >nul 2>&1

REM Display environment
echo.
echo Environment:
echo PATH includes Qt: %QT_PATH%
echo QT_QPA_PLATFORM_PLUGIN_PATH: %QT_QPA_PLATFORM_PLUGIN_PATH%
echo.

REM Run the application
echo Starting Simulation Tool...
cd build_cmake\Release
start SimulationTool.exe

REM If start fails, try running directly
if errorlevel 1 (
    echo.
    echo "start command failed, trying direct execution..."
    SimulationTool.exe
)