# 测试完整的几何处理工作流
# 1. 检查应用程序状态
# 2. 验证临时文件夹
# 3. 检查共享内存状态

Write-Host "=== 几何处理工作流验证 ==="
Write-Host

# 检查应用程序进程
Write-Host "1. 检查应用程序进程状态..."
$simToolProcess = Get-Process SimulationTool -ErrorAction SilentlyContinue
$geomProcess = Get-Process GeomProcessor -ErrorAction SilentlyContinue

if ($simToolProcess) {
    Write-Host "   SimulationTool.exe 正在运行 (PID: $($simToolProcess.Id))"
} else {
    Write-Host "   SimulationTool.exe 未运行 - 请使用 launch.ps1 启动"
}

if ($geomProcess) {
    Write-Host "   GeomProcessor.exe 正在运行 (PID: $($geomProcess.Id))"
} else {
    Write-Host "   GeomProcessor.exe 未运行 - 请使用 start_geomprocessor.ps1 启动"
}

Write-Host

# 检查临时文件夹
Write-Host "2. 检查临时文件夹..."
$geomReceivedDir = "d:\MyAICADCode\GeomProcessor\example\received"
$simReturnedDir = "d:\MyAICADCode\SimulationTool\example\returned"

if (Test-Path $geomReceivedDir) {
    Write-Host "   GeomProcessor 接收目录: $geomReceivedDir (已存在)"
    Get-ChildItem $geomReceivedDir | ForEach-Object {
        $size = $_.Length
        Write-Host "     - $($_.Name) ($size bytes)"
    }
} else {
    Write-Host "   GeomProcessor 接收目录: $geomReceivedDir (不存在)"
    Write-Host "     已自动创建目录"
    New-Item -ItemType Directory -Path $geomReceivedDir -Force | Out-Null
}

if (Test-Path $simReturnedDir) {
    Write-Host "   SimulationTool 返回目录: $simReturnedDir (已存在)"
    Get-ChildItem $simReturnedDir | ForEach-Object {
        $size = $_.Length
        Write-Host "     - $($_.Name) ($size bytes)"
    }
} else {
    Write-Host "   SimulationTool 返回目录: $simReturnedDir (不存在)"
    Write-Host "     已自动创建目录"
    New-Item -ItemType Directory -Path $simReturnedDir -Force | Out-Null
}

Write-Host

# 检查STEP文件
Write-Host "3. 检查STEP文件..."
$stepFile = "d:\MyAICADCode\SimulationTool\examples\adjacent_faces_sheetbodies.stp"
if (Test-Path $stepFile) {
    $fileSize = (Get-Item $stepFile).Length
    Write-Host "   STEP文件: $stepFile ($fileSize bytes) - 存在"
} else {
    Write-Host "   STEP文件: $stepFile - 不存在!"
    Write-Host "     请确保文件存在"
}

Write-Host

# 检查共享内存
Write-Host "4. 检查共享内存..."
$shmName = "GeomIPC_v1"
$shmExists = $false

try {
    $shm = [System.Threading.Mutex]::OpenExisting($shmName)
    Write-Host "   共享内存 '$shmName' 已存在"
    $shmExists = $true
    $shm.Dispose()
} catch {
    Write-Host "   共享内存 '$shmName' 未找到 - 应用程序启动后会创建"
}

Write-Host

# 总结和操作指南
Write-Host "=== 手动操作指南 ==="
Write-Host
Write-Host "如果应用程序已运行，请按以下步骤操作："
Write-Host "1. 切换到 SimulationTool 窗口"
Write-Host "2. 点击菜单: 文件 → 打开STEP文件"
Write-Host "3. 选择文件: $stepFile"
Write-Host "4. 点击菜单: 几何处理 → 发送到 GeomProcessor (Ctrl+G)"
Write-Host "5. 切换到 GeomProcessor 窗口"
Write-Host "6. 执行缝合/删除面/修复等操作"
Write-Host "7. 点击菜单: 操作 → 发送结果到 SimulationTool"
Write-Host "8. 切换回 SimulationTool 窗口查看处理后的几何体"
Write-Host
Write-Host "临时文件位置："
Write-Host "  发送到 GeomProcessor 的文件: $geomReceivedDir\<GUID>.stp"
Write-Host "  从 GeomProcessor 返回的文件: $simReturnedDir\<GUID>.stp"
Write-Host
Write-Host "注意事项："
Write-Host "  - 如果应用程序未运行，请先运行 launch.ps1 和 start_geomprocessor.ps1"
Write-Host "  - 临时文件使用GUID命名，完成后可手动删除"
Write-Host "  - 共享内存用于进程间通信，应用程序关闭后自动释放"
