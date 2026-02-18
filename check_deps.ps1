$exePath = "d:\MyAICADCode\SimulationTool\build_cmake\Release\SimulationTool.exe"
$dir = "d:\MyAICADCode\SimulationTool\build_cmake\Release"

# Read PE imports using Get-Content and manual parsing - use dumpbin if available
$dumpbin = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\dumpbin.exe"
if (-not (Test-Path $dumpbin)) {
    # Try to find dumpbin
    $vs2022paths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC"
    )
    foreach ($vsPath in $vs2022paths) {
        if (Test-Path $vsPath) {
            $versions = Get-ChildItem $vsPath -Directory | Sort-Object Name -Descending
            foreach ($ver in $versions) {
                $candidate = "$vsPath\$($ver.Name)\bin\Hostx64\x64\dumpbin.exe"
                if (Test-Path $candidate) {
                    $dumpbin = $candidate
                    break
                }
            }
        }
        if (Test-Path $dumpbin) { break }
    }
}

if (Test-Path $dumpbin) {
    Write-Host "Using dumpbin: $dumpbin"
    & $dumpbin /DEPENDENTS $exePath 2>&1
} else {
    Write-Host "dumpbin not found, checking known DLLs..."
    # Check if key DLLs exist in the release dir or system
    $requiredDlls = @("Qt6Core.dll","Qt6Gui.dll","Qt6Widgets.dll","Qt6OpenGL.dll","vcruntime140.dll","msvcp140.dll","tbb12.dll")
    foreach ($dll in $requiredDlls) {
        $inDir = Test-Path "$dir\$dll"
        $inSys = $null -ne (Get-Command $dll -ErrorAction SilentlyContinue)
        $status = if ($inDir) { "OK (in Release dir)" } elseif ($inSys) { "OK (in PATH)" } else { "MISSING!" }
        Write-Host "$dll : $status"
    }
    # Check platforms
    $platDir = "$dir\platforms"
    if (Test-Path "$platDir\qwindows.dll") {
        Write-Host "platforms\qwindows.dll : OK"
    } else {
        Write-Host "platforms\qwindows.dll : MISSING!"
    }
    # Check MSVC runtime
    $sysDir = "$env:SystemRoot\System32"
    foreach ($dll in @("vcruntime140.dll","vcruntime140_1.dll","msvcp140.dll","concrt140.dll")) {
        $status = if (Test-Path "$sysDir\$dll") { "OK (System32)" } else { "MISSING from System32" }
        Write-Host "$dll : $status"
    }
}
