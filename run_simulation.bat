@echo off
echo =========================================
echo Simulation Tool - Qt & OpenCASCADE
echo =========================================
echo.

REM Set Qt DLL path
set QT_PATH=D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin
set PATH=%QT_PATH%;%PATH%

REM Set Qt platform plugin path (required for Qt to find platform DLLs)
set QT_QPA_PLATFORM_PLUGIN_PATH=%~dp0build\Release\platforms

REM Set current directory
cd /d "%~dp0"

REM Check for executable
if not exist "build\Release\SimulationTool.exe" (
    echo Error: SimulationTool.exe not found
    echo Please build the project first using CMake
    echo Run: cd build && cmake --build . --config Release
    pause
    exit /b 1
)

REM Copy required DLLs
echo Copying required DLLs...
copy "D:\SDK\3rdparty-vc14-64\jemalloc-vc14-64\bin\jemalloc.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\tbb-2021.13.0-x64\bin\tbb12.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\tbb-2021.13.0-x64\bin\tbbmalloc.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\openvr-1.14.15-64\bin\win64\openvr_api.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\freeimage-3.18.0-x64\bin\FreeImage.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\freetype-2.13.3-x64\bin\freetype.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avcodec-57.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avdevice-57.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avfilter-6.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avformat-57.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\avutil-55.dll" "build\Release\" >nul 2>&1
copy "D:\SDK\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin\swscale-4.dll" "build\Release\" >nul 2>&1

REM Copy essential Qt DLLs to program directory (since PATH may not always work reliably)
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6Core.dll" "build\Release\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6Gui.dll" "build\Release\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6Widgets.dll" "build\Release\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\Qt6OpenGL.dll" "build\Release\" >nul 2>&1

REM Copy Qt platform plugins (required for Qt to initialize)
echo Copying Qt platform plugins...
if not exist "build\Release\platforms" mkdir "build\Release\platforms"
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\plugins\platforms\qwindows.dll" "build\Release\platforms\" >nul 2>&1
copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\plugins\platforms\qminimal.dll" "build\Release\platforms\" >nul 2>&1

REM Run the application
echo Starting Simulation Tool...
echo.

cd build\Release
start SimulationTool.exe