# PowerShell script to run SimulationTool
Write-Host "Starting SimulationTool..." -ForegroundColor Green

# Set environment variables
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "platforms"

# Add Qt to PATH
$qtPath = "D:\ProgramFiles\Qt\6.7.3\msvc2022_64\bin"
$env:PATH = "$qtPath;$env:PATH"

# Change to Debug directory
Set-Location "d:\MyAICADCode\SimulationTool\build_cmake\Debug"

# Check if executable exists
if (Test-Path "SimulationTool.exe") {
    Write-Host "Found SimulationTool.exe" -ForegroundColor Green
    Write-Host "Running SimulationTool..." -ForegroundColor Yellow
    .\SimulationTool.exe
} else {
    Write-Host "Error: SimulationTool.exe not found!" -ForegroundColor Red
    Write-Host "Current directory: $(Get-Location)" -ForegroundColor Yellow
    Write-Host "Files in directory:" -ForegroundColor Yellow
    Get-ChildItem
}