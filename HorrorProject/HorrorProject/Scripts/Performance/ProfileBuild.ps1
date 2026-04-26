# Profile Build Script for HorrorProject
# Builds the project with profiling enabled and runs performance analysis

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$UEPath = "C:\Program Files\Epic Games\UE_5.6",
    [string]$Configuration = "Development",
    [string]$Platform = "Win64",
    [switch]$SkipBuild = $false
)

Write-Host "=== HorrorProject Profile Build ===" -ForegroundColor Cyan
Write-Host ""

$ProjectFile = Join-Path $ProjectPath "HorrorProject.uproject"
$UATPath = Join-Path $UEPath "Engine\Build\BatchFiles\RunUAT.bat"

if (-not (Test-Path $ProjectFile)) {
    Write-Error "Project file not found: $ProjectFile"
    exit 1
}

if (-not (Test-Path $UATPath)) {
    Write-Error "UAT not found: $UATPath"
    exit 1
}

# Create profiling output directory
$OutputDir = Join-Path $ProjectPath "Saved\Profiling"
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

$Timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$ProfileDir = Join-Path $OutputDir "Profile_$Timestamp"
New-Item -ItemType Directory -Path $ProfileDir -Force | Out-Null

Write-Host "Profile directory: $ProfileDir" -ForegroundColor Green
Write-Host ""

if (-not $SkipBuild) {
    Write-Host "Building project with profiling enabled..." -ForegroundColor Yellow
    Write-Host "Configuration: $Configuration" -ForegroundColor Gray
    Write-Host "Platform: $Platform" -ForegroundColor Gray
    Write-Host ""

    # Build arguments
    $BuildArgs = @(
        "BuildCookRun",
        "-project=`"$ProjectFile`"",
        "-platform=$Platform",
        "-clientconfig=$Configuration",
        "-build",
        "-cook",
        "-stage",
        "-pak",
        "-archive",
        "-archivedirectory=`"$ProfileDir\Build`"",
        "-noP4",
        "-utf8output"
    )

    Write-Host "Starting build..." -ForegroundColor Yellow
    $BuildProcess = Start-Process -FilePath $UATPath -ArgumentList $BuildArgs -Wait -PassThru -NoNewWindow

    if ($BuildProcess.ExitCode -ne 0) {
        Write-Error "Build failed with exit code: $($BuildProcess.ExitCode)"
        exit 1
    }

    Write-Host ""
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host ""
} else {
    Write-Host "Skipping build (using existing build)" -ForegroundColor Yellow
    Write-Host ""
}

# Generate profiling configuration
Write-Host "Generating profiling configuration..." -ForegroundColor Yellow

$ProfilingConfig = @"
; Profiling Configuration for HorrorProject
; Generated: $Timestamp

[/Script/Engine.Engine]
bEnableOnScreenDebugMessages=True
bEnableOnScreenDebugMessagesDisplay=True

[/Script/Engine.GameEngine]
bSmoothFrameRate=False

[Core.System]
Paths=../../../Engine/Content
Paths=%GAMEDIR%Content
Paths=../../../Engine/Plugins/Runtime/Synthesis/Content

[/Script/Engine.RendererSettings]
; Enable profiling stats
r.ProfileGPU=1
r.ProfileGPU.ShowLeafEvents=1

[ConsoleVariables]
; Performance stats
stat.fps=1
stat.unit=1
stat.gpu=1
stat.scenerendering=1
stat.memory=1

; Profiling
r.ProfileGPU=1
r.ProfileGPU.ShowLeafEvents=1
r.Streaming.PoolSize=3000
r.Streaming.UseFixedPoolSize=1

; Lumen profiling
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.Reflections.Allow=1

; VSM profiling
r.Shadow.Virtual.Enable=1
"@

$ConfigFile = Join-Path $ProfileDir "ProfilingConfig.ini"
$ProfilingConfig | Out-File -FilePath $ConfigFile -Encoding UTF8

Write-Host "Configuration saved: $ConfigFile" -ForegroundColor Green
Write-Host ""

# Generate profiling script
$ProfilingScript = @"
# Profiling Commands for HorrorProject
# Run these commands in the Unreal Editor console

# Enable stats
stat fps
stat unit
stat gpu
stat scenerendering
stat memory
stat streaming

# Profile GPU
profilegpu

# Lumen stats
r.Lumen.Stats 1

# VSM stats
r.Shadow.Virtual.ShowStats 1

# Nanite stats
r.Nanite.ShowStats 1

# Export profiling data
stat startfile
# ... play for 5 minutes ...
stat stopfile

# Capture GPU trace
profilegpu.trace.start
# ... play for 30 seconds ...
profilegpu.trace.stop
"@

$ScriptFile = Join-Path $ProfileDir "ProfilingCommands.txt"
$ProfilingScript | Out-File -FilePath $ScriptFile -Encoding UTF8

Write-Host "Profiling commands saved: $ScriptFile" -ForegroundColor Green
Write-Host ""

# Generate analysis script
$AnalysisScript = @"
# Performance Analysis Checklist

## 1. Frame Time Analysis
- [ ] Check stat unit for bottlenecks
- [ ] Identify if Game, Render, or GPU bound
- [ ] Look for frame time spikes

## 2. GPU Profiling
- [ ] Run profilegpu command
- [ ] Identify expensive passes
- [ ] Check Lumen cost (target: <8ms)
- [ ] Check VSM cost (target: <3ms)
- [ ] Check post-process cost (target: <1ms)

## 3. Memory Analysis
- [ ] Check stat memory
- [ ] Verify texture streaming
- [ ] Check for memory leaks
- [ ] Validate memory budget (<4GB)

## 4. Draw Call Analysis
- [ ] Check stat scenerendering
- [ ] Count draw calls (target: <3000)
- [ ] Identify redundant draws
- [ ] Check instancing effectiveness

## 5. Lumen Analysis
- [ ] Check r.Lumen.Stats
- [ ] Verify probe count
- [ ] Check surface cache usage
- [ ] Validate radiance cache

## 6. VSM Analysis
- [ ] Check r.Shadow.Virtual.ShowStats
- [ ] Verify page allocation
- [ ] Check cache hit rate
- [ ] Validate clipmap levels

## 7. Optimization Priorities
Based on profiling results:
1. [ ] Highest cost system: ___________
2. [ ] Second highest: ___________
3. [ ] Third highest: ___________

## 8. Action Items
- [ ] Document bottlenecks
- [ ] Create optimization plan
- [ ] Implement fixes
- [ ] Re-profile and validate
"@

$ChecklistFile = Join-Path $ProfileDir "AnalysisChecklist.md"
$AnalysisScript | Out-File -FilePath $ChecklistFile -Encoding UTF8

Write-Host "Analysis checklist saved: $ChecklistFile" -ForegroundColor Green
Write-Host ""

# Generate summary report
$Report = @"
HorrorProject Profile Build Report
===================================

Timestamp: $Timestamp
Configuration: $Configuration
Platform: $Platform

Output Directory: $ProfileDir

Files Generated:
1. ProfilingConfig.ini - Configuration for profiling
2. ProfilingCommands.txt - Console commands to run
3. AnalysisChecklist.md - Analysis workflow

Next Steps:
1. Launch the game with profiling enabled
2. Run commands from ProfilingCommands.txt
3. Play through test scenarios (5-10 minutes)
4. Collect profiling data
5. Follow AnalysisChecklist.md
6. Document findings
7. Create optimization plan

Profiling Workflow:
1. Enable stats (stat fps, stat unit, stat gpu)
2. Play through each major area
3. Capture GPU traces (profilegpu.trace.start/stop)
4. Export stats (stat startfile/stopfile)
5. Analyze results with Unreal Insights
6. Compare against performance budget

Performance Targets:
- Frame Time: <16.67ms (60 FPS)
- Game Thread: <10ms
- Render Thread: <12ms
- GPU: <14ms
- Draw Calls: <3000
- Memory: <4GB

"@

$ReportFile = Join-Path $ProfileDir "ProfileBuildReport.txt"
$Report | Out-File -FilePath $ReportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Profile Build Complete ===" -ForegroundColor Cyan
Write-Host "Results saved to: $ProfileDir" -ForegroundColor Green
Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "1. Launch the game" -ForegroundColor Gray
Write-Host "2. Run profiling commands from: $ScriptFile" -ForegroundColor Gray
Write-Host "3. Follow analysis checklist: $ChecklistFile" -ForegroundColor Gray
