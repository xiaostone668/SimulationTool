@echo off
chcp 65001 >nul
echo 验证STEP文件几何特征...
echo.

set FILE=examples\two_sheetbodies.stp

echo 统计ADVANCED_FACE数量...
find /c "ADVANCED_FACE" %FILE%
echo.

echo 统计EDGE_CURVE数量...
find /c "EDGE_CURVE" %FILE%
echo.

echo 统计VERTEX_POINT数量...
find /c "VERTEX_POINT" %FILE%
echo.

echo 统计OPEN_SHELL数量...
find /c "OPEN_SHELL" %FILE%
echo.

echo.
echo === 验证结果 ===
echo 期望: 2个面，8条边

rem 直接统计
for /f "tokens=2" %%a in ('find /c "ADVANCED_FACE" %FILE%') do set FACES=%%a
for /f "tokens=2" %%a in ('find /c "EDGE_CURVE" %FILE%') do set EDGES=%%a

echo 实际: %FACES%个面，%EDGES%条边
echo.

if "%FACES%"=="2" (
  echo ✓ 面数量正确: 2个面
) else (
  echo ✗ 面数量错误: 期望2个，实际%FACES%个
)

if "%EDGES%"=="8" (
  echo ✓ 边数量正确: 8条边
) else (
  echo ✗ 边数量错误: 期望8条，实际%EDGES%条
)

echo.
if "%FACES%"=="2" if "%EDGES%"=="8" (
  echo ✓ 所有验证通过！文件包含2个面和8条边。
  exit /b 0
) else (
  echo ✗ 验证失败！
  exit /b 1
)