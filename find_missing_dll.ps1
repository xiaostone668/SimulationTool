$exePath = "d:\MyAICADCode\SimulationTool\build_cmake\Release\SimulationTool.exe"
$releaseDir = "d:\MyAICADCode\SimulationTool\build_cmake\Release"

# Parse PE Import Directory to find required DLLs
$bytes = [System.IO.File]::ReadAllBytes($exePath)

# Read PE offset
$peOffset = [BitConverter]::ToInt32($bytes, 0x3C)

# Check PE signature
$sig = [System.Text.Encoding]::ASCII.GetString($bytes, $peOffset, 4)
if ($sig -ne "PE`0`0") {
    Write-Host "Not a PE file!"
    exit 1
}

# Read machine/optional header
$machineOffset = $peOffset + 4
$machine = [BitConverter]::ToUInt16($bytes, $machineOffset)
$is64 = ($machine -eq 0x8664)
Write-Host "Architecture: $(if($is64){'x64'}else{'x86'})"

# Optional header offset
$optHeaderOffset = $peOffset + 24
$magicOffset = $optHeaderOffset
$magic = [BitConverter]::ToUInt16($bytes, $magicOffset)

if ($magic -eq 0x20B) {
    # PE32+
    $importDirOffset = $optHeaderOffset + 0x68 + (1 * 8)  # DataDirectory[1] = import table
    $importRva = [BitConverter]::ToUInt32($bytes, $optHeaderOffset + 0x68 + 8)
    $importSize = [BitConverter]::ToUInt32($bytes, $optHeaderOffset + 0x68 + 12)
} else {
    # PE32
    $importRva = [BitConverter]::ToUInt32($bytes, $optHeaderOffset + 0x68 + 8)
    $importSize = [BitConverter]::ToUInt32($bytes, $optHeaderOffset + 0x68 + 12)
}

# Find section containing the import RVA
$numSections = [BitConverter]::ToUInt16($bytes, $peOffset + 6)
$sectionTableOffset = $optHeaderOffset + [BitConverter]::ToUInt16($bytes, $peOffset + 20)

$importFileOffset = 0
for ($i = 0; $i -lt $numSections; $i++) {
    $secOffset = $sectionTableOffset + $i * 40
    $secVa = [BitConverter]::ToUInt32($bytes, $secOffset + 12)
    $secSize = [BitConverter]::ToUInt32($bytes, $secOffset + 16)
    $secRaw = [BitConverter]::ToUInt32($bytes, $secOffset + 20)
    if ($importRva -ge $secVa -and $importRva -lt ($secVa + $secSize)) {
        $importFileOffset = $secRaw + ($importRva - $secVa)
        break
    }
}

# Read import descriptors
$importedDlls = @()
$offset = $importFileOffset
while ($true) {
    # Import Directory Table entry is 20 bytes
    $nameRva = [BitConverter]::ToUInt32($bytes, $offset + 12)
    if ($nameRva -eq 0) { break }
    
    # Convert name RVA to file offset
    $nameFileOffset = 0
    for ($i = 0; $i -lt $numSections; $i++) {
        $secOffset = $sectionTableOffset + $i * 40
        $secVa = [BitConverter]::ToUInt32($bytes, $secOffset + 12)
        $secSize = [BitConverter]::ToUInt32($bytes, $secOffset + 16)
        $secRaw = [BitConverter]::ToUInt32($bytes, $secOffset + 20)
        if ($nameRva -ge $secVa -and $nameRva -lt ($secVa + $secSize)) {
            $nameFileOffset = $secRaw + ($nameRva - $secVa)
            break
        }
    }
    
    if ($nameFileOffset -gt 0) {
        $nameBytes = @()
        $j = $nameFileOffset
        while ($bytes[$j] -ne 0) {
            $nameBytes += $bytes[$j]
            $j++
        }
        $dllName = [System.Text.Encoding]::ASCII.GetString($nameBytes)
        $importedDlls += $dllName
    }
    $offset += 20
}

Write-Host "`nImported DLLs by SimulationTool.exe:"
Write-Host "======================================"
$systemDirs = @("$env:SystemRoot\System32", "$env:SystemRoot\SysWOW64", "$env:SystemRoot")
$searchPaths = @($releaseDir) + $systemDirs

foreach ($dll in ($importedDlls | Sort-Object)) {
    $found = $false
    $foundPath = ""
    foreach ($dir in $searchPaths) {
        if (Test-Path "$dir\$dll") {
            $found = $true
            $foundPath = $dir
            break
        }
    }
    if ($found) {
        Write-Host "  [OK] $dll  ($foundPath)"
    } else {
        Write-Host "  [MISSING] $dll  <-- NOT FOUND!"
    }
}
