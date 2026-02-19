@echo off
echo Testing GeomProcessor process check...
echo.

echo 1. Checking if GeomProcessor.exe is running...
tasklist /FI "IMAGENAME eq GeomProcessor.exe"

echo.
echo 2. Testing with PowerShell...
powershell -Command "tasklist /FI 'IMAGENAME eq GeomProcessor.exe' | findstr GeomProcessor.exe"

echo.
echo 3. If no output above, GeomProcessor is NOT running.
echo.
echo 4. Testing the exact command used in code:
echo    tasklist /FI "IMAGENAME eq GeomProcessor.exe"
echo.
tasklist /FI "IMAGENAME eq GeomProcessor.exe"

echo.
echo 5. Checking output for "GeomProcessor.exe" string...
tasklist /FI "IMAGENAME eq GeomProcessor.exe" | findstr /C:"GeomProcessor.exe"

if %errorlevel% equ 0 (
    echo SUCCESS: GeomProcessor.exe is running.
) else (
    echo FAILURE: GeomProcessor.exe is NOT running.
)

pause