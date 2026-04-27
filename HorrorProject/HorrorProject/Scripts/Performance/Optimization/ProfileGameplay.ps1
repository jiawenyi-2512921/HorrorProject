# ProfileGameplay.ps1
# Gameplay Performance Profiling Tool
# Analyzes runtime performance during gameplay sessions

param(
    [string]$ProjectPath = "",
    [string]$OutputDir = "",
    [int]$DurationSeconds = 300,
    [string]$Map = "MainLevel"
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = $ProjectRoot }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Profiling\Gameplay" }

Write-Host "=== Gameplay Performance Profiler ===" -ForegroundColor Cyan
Write-Host "Duration: $DurationSeconds seconds"
Write-Host "Map: $Map"
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "gameplay_profile_$timestamp.json"

# UE5 Editor path
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Editor = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $UE5Editor)) {
    Write-Error "UE5 Editor not found at: $UE5Editor"
    exit 1
}

Write-Host "Starting gameplay profiling session..." -ForegroundColor Yellow

# Profile commands
$profileCommands = @(
    "stat fps",
    "stat unit",
    "stat game",
    "stat scenerendering",
    "stat gpu",
    "stat memory",
    "stat streaming",
    "stat ai"
)

# Start profiling
$profileData = @{
    Timestamp = $timestamp
    Map = $Map
    Duration = $DurationSeconds
    Metrics = @{}
}

Write-Host "Launching game with profiling enabled..." -ForegroundColor Green

# Build command line arguments
$args = @(
    "`"$ProjectPath\HorrorProject.uproject`"",
    "$Map",
    "-game",
    "-windowed",
    "-ResX=1920",
    "-ResY=1080",
    "-ExecCmds=`"$($profileCommands -join ';')`"",
    "-log",
    "-unattended",
    "-nopause",
    "-nosplash"
)

# Start game process
$process = Start-Process -FilePath $UE5Editor -ArgumentList $args -PassThru -NoNewWindow

Write-Host "Game launched. Profiling for $DurationSeconds seconds..." -ForegroundColor Yellow
Write-Host "Process ID: $($process.Id)"

# Wait for profiling duration
$elapsed = 0
while ($elapsed -lt $DurationSeconds -and -not $process.HasExited) {
    Start-Sleep -Seconds 10
    $elapsed += 10
    $progress = [math]::Min(100, ($elapsed / $DurationSeconds) * 100)
    Write-Progress -Activity "Profiling Gameplay" -Status "$elapsed / $DurationSeconds seconds" -PercentComplete $progress
}

Write-Progress -Activity "Profiling Gameplay" -Completed

# Stop game
if (-not $process.HasExited) {
    Write-Host "Stopping game process..." -ForegroundColor Yellow
    $process.Kill()
    $process.WaitForExit(5000)
}

Write-Host "Analyzing log files..." -ForegroundColor Green

# Parse log file
$logPath = Join-Path $ProjectPath "Saved\Logs\HorrorProject.log"
if (Test-Path $logPath) {
    $logContent = Get-Content $logPath -Tail 10000

    # Extract FPS data
    $fpsData = $logContent | Select-String "FPS:\s*(\d+\.?\d*)" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($fpsData.Count -gt 0) {
        $profileData.Metrics.FPS = @{
            Average = ($fpsData | Measure-Object -Average).Average
            Min = ($fpsData | Measure-Object -Minimum).Minimum
            Max = ($fpsData | Measure-Object -Maximum).Maximum
            Samples = $fpsData.Count
        }
    }

    # Extract frame time data
    $frameTimeData = $logContent | Select-String "Frame:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($frameTimeData.Count -gt 0) {
        $profileData.Metrics.FrameTime = @{
            Average = ($frameTimeData | Measure-Object -Average).Average
            Min = ($frameTimeData | Measure-Object -Minimum).Minimum
            Max = ($frameTimeData | Measure-Object -Maximum).Maximum
            P95 = ($frameTimeData | Sort-Object)[([math]::Floor($frameTimeData.Count * 0.95))]
            P99 = ($frameTimeData | Sort-Object)[([math]::Floor($frameTimeData.Count * 0.99))]
        }
    }

    # Extract game thread time
    $gameThreadData = $logContent | Select-String "Game:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($gameThreadData.Count -gt 0) {
        $profileData.Metrics.GameThread = @{
            Average = ($gameThreadData | Measure-Object -Average).Average
            Max = ($gameThreadData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract render thread time
    $renderThreadData = $logContent | Select-String "Render:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($renderThreadData.Count -gt 0) {
        $profileData.Metrics.RenderThread = @{
            Average = ($renderThreadData | Measure-Object -Average).Average
            Max = ($renderThreadData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract GPU time
    $gpuTimeData = $logContent | Select-String "GPU:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($gpuTimeData.Count -gt 0) {
        $profileData.Metrics.GPU = @{
            Average = ($gpuTimeData | Measure-Object -Average).Average
            Max = ($gpuTimeData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract memory usage
    $memoryData = $logContent | Select-String "Memory:\s*(\d+)\s*MB" | ForEach-Object {
        [int]$_.Matches.Groups[1].Value
    }

    if ($memoryData.Count -gt 0) {
        $profileData.Metrics.Memory = @{
            Average = ($memoryData | Measure-Object -Average).Average
            Max = ($memoryData | Measure-Object -Maximum).Maximum
        }
    }
}

# Save report
$profileData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Profiling Results ===" -ForegroundColor Cyan

if ($profileData.Metrics.FPS) {
    Write-Host "FPS:" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.Metrics.FPS.Average, 2))"
    Write-Host "  Min: $([math]::Round($profileData.Metrics.FPS.Min, 2))"
    Write-Host "  Max: $([math]::Round($profileData.Metrics.FPS.Max, 2))"
    Write-Host ""
}

if ($profileData.Metrics.FrameTime) {
    Write-Host "Frame Time (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.Metrics.FrameTime.Average, 2))"
    Write-Host "  P95: $([math]::Round($profileData.Metrics.FrameTime.P95, 2))"
    Write-Host "  P99: $([math]::Round($profileData.Metrics.FrameTime.P99, 2))"
    Write-Host ""
}

if ($profileData.Metrics.GameThread) {
    Write-Host "Game Thread (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.Metrics.GameThread.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.Metrics.GameThread.Max, 2))"
    Write-Host ""
}

if ($profileData.Metrics.RenderThread) {
    Write-Host "Render Thread (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.Metrics.RenderThread.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.Metrics.RenderThread.Max, 2))"
    Write-Host ""
}

if ($profileData.Metrics.GPU) {
    Write-Host "GPU Time (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.Metrics.GPU.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.Metrics.GPU.Max, 2))"
    Write-Host ""
}

if ($profileData.Metrics.Memory) {
    Write-Host "Memory (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.Metrics.Memory.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.Metrics.Memory.Max, 2))"
    Write-Host ""
}

Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Performance assessment
$targetFPS = 60
$targetFrameTime = 16.67

if ($profileData.Metrics.FPS -and $profileData.Metrics.FPS.Average -lt $targetFPS) {
    Write-Host "WARNING: Average FPS below target ($targetFPS)" -ForegroundColor Red
}

if ($profileData.Metrics.FrameTime -and $profileData.Metrics.FrameTime.Average -gt $targetFrameTime) {
    Write-Host "WARNING: Average frame time above target ($targetFrameTime ms)" -ForegroundColor Red
}

Write-Host "Gameplay profiling complete!" -ForegroundColor Green
