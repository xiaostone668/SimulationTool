# PowerShell脚本分析STEP文件几何元素
$filePath = "examples/two_sheetbodies.stp"

Write-Host "分析STEP文件: $filePath" -ForegroundColor Cyan

# 读取文件内容
$content = Get-Content $filePath -Raw

# 统计各种元素
$advancedFaceCount = ($content | Select-String -Pattern '#\d+\s*=\s*ADVANCED_FACE' -AllMatches).Matches.Count
$faceSurfaceCount = ($content | Select-String -Pattern '#\d+\s*=\s*FACE_SURFACE' -AllMatches).Matches.Count
$totalFaces = $advancedFaceCount + $faceSurfaceCount

$edgeCurveCount = ($content | Select-String -Pattern '#\d+\s*=\s*EDGE_CURVE' -AllMatches).Matches.Count
$vertexCount = ($content | Select-String -Pattern '#\d+\s*=\s*VERTEX_POINT' -AllMatches).Matches.Count
$openShellCount = ($content | Select-String -Pattern '#\d+\s*=\s*OPEN_SHELL' -AllMatches).Matches.Count
$closedShellCount = ($content | Select-String -Pattern '#\d+\s*=\s*CLOSED_SHELL' -AllMatches).Matches.Count
$totalShells = $openShellCount + $closedShellCount

Write-Host "=== STEP文件分析结果 ===" -ForegroundColor Yellow
Write-Host "ADVANCED_FACE 数量: $advancedFaceCount"
Write-Host "FACE_SURFACE 数量: $faceSurfaceCount"
Write-Host "总面数: $totalFaces"
Write-Host "EDGE_CURVE 数量: $edgeCurveCount"
Write-Host "VERTEX_POINT 数量: $vertexCount"
Write-Host "OPEN_SHELL 数量: $openShellCount"
Write-Host "CLOSED_SHELL 数量: $closedShellCount"
Write-Host "总SHELL数: $totalShells"
Write-Host ""

Write-Host "=== 验证要求 ===" -ForegroundColor Yellow
$faceOk = $totalFaces -eq 2
$edgeOk = $edgeCurveCount -eq 8

if ($faceOk) {
    Write-Host "✓ 面数量正确: $totalFaces 个面" -ForegroundColor Green
} else {
    Write-Host "✗ 面数量错误: 期望2个，实际$totalFaces个" -ForegroundColor Red
}

if ($edgeOk) {
    Write-Host "✓ 边数量正确: $edgeCurveCount 条边" -ForegroundColor Green
} else {
    Write-Host "✗ 边数量错误: 期望8条，实际$edgeCurveCount条" -ForegroundColor Red
}

Write-Host ""
Write-Host "=== 详细匹配 ===" -ForegroundColor Yellow

# 显示所有ADVANCED_FACE实例
Write-Host "ADVANCED_FACE实例:" -ForegroundColor Cyan
$content | Select-String -Pattern '#\d+\s*=\s*ADVANCED_FACE' | ForEach-Object {
    Write-Host "  $_"
}

Write-Host ""
Write-Host "EDGE_CURVE实例:" -ForegroundColor Cyan
$content | Select-String -Pattern '#\d+\s*=\s*EDGE_CURVE' | ForEach-Object {
    Write-Host "  $_"
}

# 退出码
if ($faceOk -and $edgeOk) {
    Write-Host "`n✓ 所有验证通过！" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n✗ 验证失败！" -ForegroundColor Red
    exit 1
}