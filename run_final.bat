@echo off
echo =========================================
echo Simulation Tool - Final Runner
echo =========================================
echo.

REM Set current directory to script location
cd /d "%~dp0"

REM Check for executable - try multiple locations
if exist "build\Release\SimulationTool.exe" (
    set EXE_PATH=build\Release\SimulationTool.exe
    echo Found executable in build\Release
) else if exist "build_cmake\Release\SimulationTool.exe" (
    set EXE_PATH=build_cmake\Release\SimulationTool.exe
    echo Found executable in build_cmake\Release
) else if exist "out\build\x64-Debug\SimulationTool.exe" (
    set EXE_PATH=out\build\x64-Debug\SimulationTool.exe
    echo Found executable in out\build\x64-Debug
) else (
    echo Error: SimulationTool.exe not found in any build directory
    echo Please build the project first
    pause
    exit /b 1
)

echo Executable: %EXE_PATH%
echo.

REM Set Qt environment
set QT_PATH=D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin
set PATH=%QT_PATH%;%PATH%

REM Set Qt platform plugin path
for %%I in ("%EXE_PATH%") do set EXE_DIR=%%~dpI
set QT_QPA_PLATFORM_PLUGIN_PATH=%EXE_DIR%platforms
echo QT_QPA_PLATFORM_PLUGIN_PATH: %QT_QPA_PLATFORM_PLUGIN_PATH%

REM Check if platforms directory exists
if not exist "%EXE_DIR%platforms" (
    echo Creating platforms directory...
    mkdir "%EXE_DIR%platforms"
)

REM Copy Qt platform plugin if needed
if not exist "%EXE_DIR%platforms\qwindows.dll" (
    echo Copying qwindows.dll...
    copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\plugins\platforms\qwindows.dll" "%EXE_DIR%platforms\" >nul 2>&1
    if errorlevel 1 (
        echo Warning: Could not copy qwindows.dll
    ) else (
        echo qwindows.dll copied successfully
    )
)

REM Copy required DLLs to executable directory
echo.
echo Ensuring required DLLs are present...
set DLLS_TO_COPY=Qt6Core.dll Qt6Gui.dll Qt6Widgets.dll Qt6OpenGL.dll
for %%D in (%DLLS_TO_COPY%) do (
    if not exist "%EXE_DIR%%%D" (
        echo Copying %%D...
        copy "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin\%%D" "%EXE_DIR%" >nul 2>&1
    )
)

REM Copy TBB DLLs
if not exist "%EXE_DIR%tbb12.dll" (
    echo Copying TBB DLLs...
    copy "D:\SDK\3rdparty-vc14-64\tbb-2021.13.0-x64\bin\tbb12.dll" "%EXE_DIR%" >nul 2>&1
    copy "D:\SDK\3rdparty-vc14-64\tbb-2021.13.0-x64\bin\tbbmalloc.dll" "%EXE_DIR%" >nul 2>&1
)

REM Run the application
echo.
echo Running Simulation Tool...
echo.
cd "%EXE_DIR%"
"%EXE_PATH%"

if errorlevel 1 (
    echo.
    echo "Application exited with error code %ERRORLEVEL%"
    if %ERRORLEVEL% == -1073741515 (
        echo "Error: STATUS_DLL_NOT_FOUND - Check for missing DLL dependencies"
    )
) else (
    echo.
    echo "Application exited successfully"
)

pause