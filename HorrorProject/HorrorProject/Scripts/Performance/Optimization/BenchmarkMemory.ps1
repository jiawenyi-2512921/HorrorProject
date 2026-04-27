# BenchmarkMemory.ps1
# Memory Benchmark Tool
# Measures memory usage across different scenarios

param(
    [string]$ProjectPath = "",
    [string]$OutputDir = "",
    [string]$Map = "MainLevel"
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = $ProjectRoot }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Benchmarks" }

Write-Host "=== Memory Benchmark Tool ===" -ForegroundColor Cyan
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "memory_benchmark_$timestamp.json"

# Test scenarios
$scenarios = @(
    @{ Name = "Startup"; Duration = 30; Description = "Initial load" },
    @{ Name = "Idle"; Duration = 60; Description = "Idle in level" },
    @{ Name = "Movement"; Duration = 90; Description = "Moving through level" },
    @{ Name = "Combat"; Duration = 60; Description = "Combat scenario" }
)

# UE5 Editor path
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Editor = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $UE5Editor)) {
    Write-Error "UE5 Editor not found at: $UE5Editor"
    exit 1
}

$benchmarkData = @{
    Timestamp = $timestamp
    Map = $Map
    Scenarios = @{}
}

foreach ($scenario in $scenarios) {
    Write-Host ""
    Write-Host "=== Testing Scenario: $($scenario.Name) ===" -ForegroundColor Yellow
    Write-Host "Description: $($scenario.Description)"

    # Clear log
    $logPath = Join-Path $ProjectPath "Saved\Logs\HorrorProject.log"
    if (Test-Path $logPath) {
        Remove-Item $logPath -Force
    }

    # Build command line arguments
    $args = @(
        "`"$ProjectPath\HorrorProject.uproject`"",
        "$Map",
        "-game",
        "-windowed",
        "-ResX=1920",
        "-ResY=1080",
        "-ExecCmds=`"stat memory;stat streaming;memreport`"",
        "-log",
        "-unattended",
        "-nosplash"
    )

    # Start game
    $process = Start-Process -FilePath $UE5Editor -ArgumentList $args -PassThru -NoNewWindow

    Write-Host "Running scenario for $($scenario.Duration) seconds..." -ForegroundColor Green

    # Monitor memory during scenario
    $memorySnapshots = @()
    $elapsed = 0
    $interval = 5

    while ($elapsed -lt $scenario.Duration -and -not $process.HasExited) {
        Start-Sleep -Seconds $interval
        $elapsed += $interval

        # Get process memory
        $proc = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
        if ($proc) {
            $memorySnapshots += @{
                Time = $elapsed
                WorkingSet = $proc.WorkingSet64
                PrivateMemory = $proc.PrivateMemorySize64
            }
        }

        $progress = [math]::Min(100, ($elapsed / $scenario.Duration) * 100)
        Write-Progress -Activity "Testing $($scenario.Name)" -Status "$elapsed / $($scenario.Duration) seconds" -PercentComplete $progress
    }

    Write-Progress -Activity "Testing $($scenario.Name)" -Completed

    # Stop game
    if (-not $process.HasExited) {
        $process.Kill()
        $process.WaitForExit(5000)
    }

    Write-Host "Analyzing memory data..." -ForegroundColor Cyan

    # Calculate statistics
    $scenarioResults = @{
        Name = $scenario.Name
        Description = $scenario.Description
        Duration = $scenario.Duration
        WorkingSet = @{}
        PrivateMemory = @{}
        DetailedStats = @{}
    }

    if ($memorySnapshots.Count -gt 0) {
        $workingSets = $memorySnapshots | ForEach-Object { $_.WorkingSet }
        $privateMemory = $memorySnapshots | ForEach-Object { $_.PrivateMemory }

        $scenarioResults.WorkingSet = @{
            Average = ($workingSets | Measure-Object -Average).Average / 1MB
            Min = ($workingSets | Measure-Object -Minimum).Minimum / 1MB
            Max = ($workingSets | Measure-Object -Maximum).Maximum / 1MB
            Final = $workingSets[-1] / 1MB
        }

        $scenarioResults.PrivateMemory = @{
            Average = ($privateMemory | Measure-Object -Average).Average / 1MB
            Min = ($privateMemory | Measure-Object -Minimum).Minimum / 1MB
            Max = ($privateMemory | Measure-Object -Maximum).Maximum / 1MB
            Final = $privateMemory[-1] / 1MB
        }
    }

    # Parse log for detailed memory stats
    if (Test-Path $logPath) {
        $logContent = Get-Content $logPath -Tail 2000

        # Extract texture memory
        $textureMemory = $logContent | Select-String "Texture Memory:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        } | Select-Object -Last 1

        # Extract mesh memory
        $meshMemory = $logContent | Select-String "StaticMesh Memory:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        } | Select-Object -Last 1

        # Extract audio memory
        $audioMemory = $logContent | Select-String "Audio Memory:\s*(\d+\.?\d*)\s*MB" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        } | Select-Object -Last 1

        $scenarioResults.DetailedStats = @{
            TextureMemory = if ($textureMemory) { $textureMemory } else { 0 }
            MeshMemory = if ($meshMemory) { $meshMemory } else { 0 }
            AudioMemory = if ($audioMemory) { $audioMemory } else { 0 }
        }
    }

    $benchmarkData.Scenarios[$scenario.Name] = $scenarioResults

    # Display results
    Write-Host ""
    Write-Host "Results for $($scenario.Name):" -ForegroundColor Green
    Write-Host "  Working Set (MB):"
    Write-Host "    Average: $([math]::Round($scenarioResults.WorkingSet.Average, 2))"
    Write-Host "    Max: $([math]::Round($scenarioResults.WorkingSet.Max, 2))"
    Write-Host "    Final: $([math]::Round($scenarioResults.WorkingSet.Final, 2))"
    Write-Host "  Private Memory (MB):"
    Write-Host "    Average: $([math]::Round($scenarioResults.PrivateMemory.Average, 2))"
    Write-Host "    Max: $([math]::Round($scenarioResults.PrivateMemory.Max, 2))"
}

# Save report
$benchmarkData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Memory Benchmark Summary ===" -ForegroundColor Cyan
Write-Host ""

# Compare scenarios
Write-Host "Scenario Comparison (Working Set MB):" -ForegroundColor Yellow
Write-Host ""
Write-Host "Scenario  | Average | Max     | Final" -ForegroundColor White
Write-Host "----------|---------|---------|-------" -ForegroundColor White

foreach ($scenario in $scenarios) {
    $name = $scenario.Name
    if ($benchmarkData.Scenarios.ContainsKey($name)) {
        $result = $benchmarkData.Scenarios[$name]
        $avg = [math]::Round($result.WorkingSet.Average, 1)
        $max = [math]::Round($result.WorkingSet.Max, 1)
        $final = [math]::Round($result.WorkingSet.Final, 1)

        $color = "White"
        if ($max -lt 4000) { $color = "Green" }
        elseif ($max -lt 6000) { $color = "Yellow" }
        else { $color = "Red" }

        Write-Host ("{0,-9} | {1,7} | {2,7} | {3,6}" -f $name, $avg, $max, $final) -ForegroundColor $color
    }
}

Write-Host ""
Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Memory leak detection
Write-Host "=== Memory Leak Analysis ===" -ForegroundColor Cyan

$startupFinal = $benchmarkData.Scenarios["Startup"].WorkingSet.Final
$idleFinal = $benchmarkData.Scenarios["Idle"].WorkingSet.Final

if ($idleFinal -gt ($startupFinal * 1.2)) {
    Write-Host "WARNING: Potential memory leak detected during idle" -ForegroundColor Red
    Write-Host "  Startup final: $([math]::Round($startupFinal, 2)) MB" -ForegroundColor Yellow
    Write-Host "  Idle final: $([math]::Round($idleFinal, 2)) MB" -ForegroundColor Yellow
    Write-Host "  Increase: $([math]::Round((($idleFinal - $startupFinal) / $startupFinal) * 100, 1))%" -ForegroundColor Yellow
} else {
    Write-Host "No significant memory leaks detected" -ForegroundColor Green
}

Write-Host ""
Write-Host "Memory benchmark complete!" -ForegroundColor Green
