# Performance Benchmark Script for HorrorProject
# Runs automated performance tests and generates reports

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$UEPath = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
    [string]$MapName = "/Game/Maps/DeepWaterStation_Main",
    [int]$DurationSeconds = 300,
    [string]$OutputDir = "Saved\Profiling"
)

Write-Host "=== HorrorProject Performance Benchmark ===" -ForegroundColor Cyan
Write-Host ""

# Validate paths
if (-not (Test-Path $ProjectPath)) {
    Write-Error "Project path not found: $ProjectPath"
    exit 1
}

if (-not (Test-Path $UEPath)) {
    Write-Error "Unreal Editor not found: $UEPath"
    exit 1
}

$ProjectFile = Join-Path $ProjectPath "HorrorProject.uproject"
if (-not (Test-Path $ProjectFile)) {
    Write-Error "Project file not found: $ProjectFile"
    exit 1
}

# Create output directory
$FullOutputDir = Join-Path $ProjectPath $OutputDir
if (-not (Test-Path $FullOutputDir)) {
    New-Item -ItemType Directory -Path $FullOutputDir -Force | Out-Null
}

$Timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$BenchmarkDir = Join-Path $FullOutputDir "Benchmark_$Timestamp"
New-Item -ItemType Directory -Path $BenchmarkDir -Force | Out-Null

Write-Host "Output directory: $BenchmarkDir" -ForegroundColor Green
Write-Host ""

# Benchmark configuration
$BenchmarkConfig = @"
[Benchmark]
Duration=$DurationSeconds
Map=$MapName
EnableProfiling=true
EnableStats=true
CaptureFrames=true
"@

$ConfigFile = Join-Path $BenchmarkDir "BenchmarkConfig.ini"
$BenchmarkConfig | Out-File -FilePath $ConfigFile -Encoding UTF8

Write-Host "Starting benchmark..." -ForegroundColor Yellow
Write-Host "Duration: $DurationSeconds seconds" -ForegroundColor Gray
Write-Host "Map: $MapName" -ForegroundColor Gray
Write-Host ""

# Build command line arguments
$Args = @(
    "`"$ProjectFile`"",
    "$MapName",
    "-game",
    "-windowed",
    "-ResX=1920",
    "-ResY=1080",
    "-ExecCmds=`"stat fps,stat unit,stat gpu`"",
    "-benchmark",
    "-fps=60",
    "-deterministic",
    "-seconds=$DurationSeconds",
    "-unattended",
    "-noscreenmessages",
    "-log"
)

$LogFile = Join-Path $BenchmarkDir "Benchmark.log"

Write-Host "Launching Unreal Editor..." -ForegroundColor Yellow

# Start the benchmark
$Process = Start-Process -FilePath $UEPath -ArgumentList $Args -PassThru -RedirectStandardOutput $LogFile -NoNewWindow

Write-Host "Benchmark running (PID: $($Process.Id))..." -ForegroundColor Green
Write-Host "Waiting for completion..." -ForegroundColor Gray

# Wait for completion
$Process.WaitForExit()

Write-Host ""
Write-Host "Benchmark completed!" -ForegroundColor Green
Write-Host ""

# Parse results
Write-Host "Parsing results..." -ForegroundColor Yellow

$StatsFile = Join-Path $ProjectPath "Saved\Profiling\*.csv"
$LatestStats = Get-ChildItem $StatsFile -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1

if ($LatestStats) {
    Copy-Item $LatestStats.FullName -Destination (Join-Path $BenchmarkDir "PerformanceStats.csv")
    Write-Host "Stats copied: PerformanceStats.csv" -ForegroundColor Green
}

# Generate summary report
$ReportFile = Join-Path $BenchmarkDir "BenchmarkReport.txt"

$Report = @"
HorrorProject Performance Benchmark Report
==========================================

Timestamp: $Timestamp
Duration: $DurationSeconds seconds
Map: $MapName

Configuration:
- Resolution: 1920x1080
- Target FPS: 60
- Quality: Epic

Results:
- See PerformanceStats.csv for detailed metrics
- See Benchmark.log for full output

Next Steps:
1. Analyze PerformanceStats.csv
2. Identify bottlenecks
3. Compare with performance budget
4. Implement optimizations

"@

$Report | Out-File -FilePath $ReportFile -Encoding UTF8

Write-Host ""
Write-Host "Report generated: $ReportFile" -ForegroundColor Green
Write-Host ""
Write-Host "=== Benchmark Complete ===" -ForegroundColor Cyan
Write-Host "Results saved to: $BenchmarkDir" -ForegroundColor Green
