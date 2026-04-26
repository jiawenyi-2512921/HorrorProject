# ProfileMemory.ps1
# Memory Usage Profiling Tool
# Analyzes memory allocation and usage patterns

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Profiling\Memory",
    [string]$Map = "MainLevel"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Memory Usage Profiler ===" -ForegroundColor Cyan
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "memory_profile_$timestamp.json"

# UE5 Editor path
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Editor = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $UE5Editor)) {
    Write-Error "UE5 Editor not found at: $UE5Editor"
    exit 1
}

Write-Host "Analyzing memory usage..." -ForegroundColor Yellow

$profileData = @{
    Timestamp = $timestamp
    Map = $Map
    MemoryStats = @{}
}

# Memory profile commands
$memoryCommands = @(
    "stat memory",
    "stat streaming",
    "stat streamingdetails",
    "memreport -full",
    "obj list"
)

Write-Host "Launching editor for memory analysis..." -ForegroundColor Green

# Build command line arguments
$args = @(
    "`"$ProjectPath\HorrorProject.uproject`"",
    "$Map",
    "-game",
    "-windowed",
    "-ResX=1920",
    "-ResY=1080",
    "-ExecCmds=`"$($memoryCommands -join ';')`"",
    "-log",
    "-unattended",
    "-nosplash"
)

# Start process
$process = Start-Process -FilePath $UE5Editor -ArgumentList $args -PassThru -NoNewWindow

Write-Host "Collecting memory data (60 seconds)..." -ForegroundColor Yellow
Start-Sleep -Seconds 60

# Stop process
if (-not $process.HasExited) {
    $process.Kill()
    $process.WaitForExit(5000)
}

Write-Host "Analyzing memory data..." -ForegroundColor Green

# Parse log file
$logPath = Join-Path $ProjectPath "Saved\Logs\HorrorProject.log"
if (Test-Path $logPath) {
    $logContent = Get-Content $logPath -Tail 10000

    # Extract total memory usage
    $totalMemoryData = $logContent | Select-String "Total Physical:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($totalMemoryData.Count -gt 0) {
        $profileData.MemoryStats.TotalPhysical = @{
            Average = ($totalMemoryData | Measure-Object -Average).Average
            Max = ($totalMemoryData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract texture memory
    $textureMemoryData = $logContent | Select-String "Texture Memory:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($textureMemoryData.Count -gt 0) {
        $profileData.MemoryStats.TextureMemory = @{
            Average = ($textureMemoryData | Measure-Object -Average).Average
            Max = ($textureMemoryData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract mesh memory
    $meshMemoryData = $logContent | Select-String "StaticMesh Memory:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($meshMemoryData.Count -gt 0) {
        $profileData.MemoryStats.MeshMemory = @{
            Average = ($meshMemoryData | Measure-Object -Average).Average
            Max = ($meshMemoryData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract audio memory
    $audioMemoryData = $logContent | Select-String "Audio Memory:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($audioMemoryData.Count -gt 0) {
        $profileData.MemoryStats.AudioMemory = @{
            Average = ($audioMemoryData | Measure-Object -Average).Average
            Max = ($audioMemoryData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract animation memory
    $animMemoryData = $logContent | Select-String "Animation Memory:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($animMemoryData.Count -gt 0) {
        $profileData.MemoryStats.AnimationMemory = @{
            Average = ($animMemoryData | Measure-Object -Average).Average
            Max = ($animMemoryData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract streaming pool size
    $streamingPoolData = $logContent | Select-String "Streaming Pool:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($streamingPoolData.Count -gt 0) {
        $profileData.MemoryStats.StreamingPool = @{
            Average = ($streamingPoolData | Measure-Object -Average).Average
            Max = ($streamingPoolData | Measure-Object -Maximum).Maximum
        }
    }

    # Count loaded assets
    $assetCounts = @{}
    $assetLines = $logContent | Select-String "Class:\s*(\w+)\s+Count:\s*(\d+)"
    foreach ($line in $assetLines) {
        $className = $line.Matches.Groups[1].Value
        $count = [int]$line.Matches.Groups[2].Value
        if (-not $assetCounts.ContainsKey($className)) {
            $assetCounts[$className] = @()
        }
        $assetCounts[$className] += $count
    }

    if ($assetCounts.Count -gt 0) {
        $profileData.MemoryStats.LoadedAssets = @{}
        foreach ($className in $assetCounts.Keys) {
            $counts = $assetCounts[$className]
            $profileData.MemoryStats.LoadedAssets[$className] = @{
                Average = ($counts | Measure-Object -Average).Average
                Max = ($counts | Measure-Object -Maximum).Maximum
            }
        }
    }
}

# Save report
$profileData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Memory Analysis Results ===" -ForegroundColor Cyan

if ($profileData.MemoryStats.TotalPhysical) {
    Write-Host "Total Physical Memory (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.MemoryStats.TotalPhysical.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.MemoryStats.TotalPhysical.Max, 2))"
    Write-Host ""
}

if ($profileData.MemoryStats.TextureMemory) {
    Write-Host "Texture Memory (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.MemoryStats.TextureMemory.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.MemoryStats.TextureMemory.Max, 2))"
    Write-Host ""
}

if ($profileData.MemoryStats.MeshMemory) {
    Write-Host "Mesh Memory (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.MemoryStats.MeshMemory.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.MemoryStats.MeshMemory.Max, 2))"
    Write-Host ""
}

if ($profileData.MemoryStats.AudioMemory) {
    Write-Host "Audio Memory (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.MemoryStats.AudioMemory.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.MemoryStats.AudioMemory.Max, 2))"
    Write-Host ""
}

if ($profileData.MemoryStats.AnimationMemory) {
    Write-Host "Animation Memory (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.MemoryStats.AnimationMemory.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.MemoryStats.AnimationMemory.Max, 2))"
    Write-Host ""
}

if ($profileData.MemoryStats.StreamingPool) {
    Write-Host "Streaming Pool (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.MemoryStats.StreamingPool.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.MemoryStats.StreamingPool.Max, 2))"
    Write-Host ""
}

if ($profileData.MemoryStats.LoadedAssets) {
    Write-Host "Top Loaded Asset Types:" -ForegroundColor Yellow
    $sortedAssets = $profileData.MemoryStats.LoadedAssets.GetEnumerator() |
        Sort-Object { $_.Value.Max } -Descending |
        Select-Object -First 5
    foreach ($asset in $sortedAssets) {
        Write-Host "  $($asset.Key): $([math]::Round($asset.Value.Max, 0))"
    }
    Write-Host ""
}

Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Memory optimization recommendations
Write-Host "=== Memory Optimization Recommendations ===" -ForegroundColor Cyan

$totalMemoryMB = if ($profileData.MemoryStats.TotalPhysical) { $profileData.MemoryStats.TotalPhysical.Max } else { 0 }

if ($totalMemoryMB -gt 8000) {
    Write-Host "- High memory usage detected ($([math]::Round($totalMemoryMB, 0)) MB)" -ForegroundColor Red
    Write-Host "  Consider implementing aggressive asset streaming" -ForegroundColor Yellow
}

if ($profileData.MemoryStats.TextureMemory -and $profileData.MemoryStats.TextureMemory.Max -gt 2000) {
    Write-Host "- Texture memory is high. Optimize texture sizes and use streaming" -ForegroundColor Yellow
}

if ($profileData.MemoryStats.MeshMemory -and $profileData.MemoryStats.MeshMemory.Max -gt 1000) {
    Write-Host "- Mesh memory is high. Implement LODs and mesh streaming" -ForegroundColor Yellow
}

if ($profileData.MemoryStats.AudioMemory -and $profileData.MemoryStats.AudioMemory.Max -gt 500) {
    Write-Host "- Audio memory is high. Use audio streaming for large files" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Memory profiling complete!" -ForegroundColor Green
