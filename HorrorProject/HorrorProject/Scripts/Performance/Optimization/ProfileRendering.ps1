# ProfileRendering.ps1
# Rendering Performance Profiling Tool
# Analyzes GPU and rendering pipeline performance

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Profiling\Rendering",
    [string]$Map = "MainLevel"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Rendering Performance Profiler ===" -ForegroundColor Cyan
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "rendering_profile_$timestamp.json"

# UE5 Editor path
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Editor = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $UE5Editor)) {
    Write-Error "UE5 Editor not found at: $UE5Editor"
    exit 1
}

Write-Host "Analyzing rendering performance..." -ForegroundColor Yellow

$profileData = @{
    Timestamp = $timestamp
    Map = $Map
    RenderingStats = @{}
}

# Rendering profile commands
$renderCommands = @(
    "stat gpu",
    "stat scenerendering",
    "stat rhi",
    "stat shadowrendering",
    "stat lightrendering",
    "stat particles",
    "profilegpu",
    "r.ScreenPercentage 100"
)

Write-Host "Launching editor for rendering analysis..." -ForegroundColor Green

# Build command line arguments
$args = @(
    "`"$ProjectPath\HorrorProject.uproject`"",
    "$Map",
    "-game",
    "-windowed",
    "-ResX=1920",
    "-ResY=1080",
    "-ExecCmds=`"$($renderCommands -join ';')`"",
    "-log",
    "-unattended",
    "-nosplash"
)

# Start process
$process = Start-Process -FilePath $UE5Editor -ArgumentList $args -PassThru -NoNewWindow

Write-Host "Collecting rendering data (60 seconds)..." -ForegroundColor Yellow
Start-Sleep -Seconds 60

# Stop process
if (-not $process.HasExited) {
    $process.Kill()
    $process.WaitForExit(5000)
}

Write-Host "Analyzing rendering data..." -ForegroundColor Green

# Parse log file
$logPath = Join-Path $ProjectPath "Saved\Logs\HorrorProject.log"
if (Test-Path $logPath) {
    $logContent = Get-Content $logPath -Tail 5000

    # Extract draw calls
    $drawCallData = $logContent | Select-String "DrawCalls:\s*(\d+)" | ForEach-Object {
        [int]$_.Matches.Groups[1].Value
    }

    if ($drawCallData.Count -gt 0) {
        $profileData.RenderingStats.DrawCalls = @{
            Average = ($drawCallData | Measure-Object -Average).Average
            Max = ($drawCallData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract triangle count
    $triangleData = $logContent | Select-String "Triangles:\s*(\d+)" | ForEach-Object {
        [int]$_.Matches.Groups[1].Value
    }

    if ($triangleData.Count -gt 0) {
        $profileData.RenderingStats.Triangles = @{
            Average = ($triangleData | Measure-Object -Average).Average
            Max = ($triangleData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract shadow rendering time
    $shadowTimeData = $logContent | Select-String "ShadowDepths:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($shadowTimeData.Count -gt 0) {
        $profileData.RenderingStats.ShadowRendering = @{
            Average = ($shadowTimeData | Measure-Object -Average).Average
            Max = ($shadowTimeData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract lighting time
    $lightingTimeData = $logContent | Select-String "Lights:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($lightingTimeData.Count -gt 0) {
        $profileData.RenderingStats.Lighting = @{
            Average = ($lightingTimeData | Measure-Object -Average).Average
            Max = ($lightingTimeData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract base pass time
    $basePassData = $logContent | Select-String "BasePass:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($basePassData.Count -gt 0) {
        $profileData.RenderingStats.BasePass = @{
            Average = ($basePassData | Measure-Object -Average).Average
            Max = ($basePassData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract post-processing time
    $postProcessData = $logContent | Select-String "PostProcessing:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
        [float]$_.Matches.Groups[1].Value
    }

    if ($postProcessData.Count -gt 0) {
        $profileData.RenderingStats.PostProcessing = @{
            Average = ($postProcessData | Measure-Object -Average).Average
            Max = ($postProcessData | Measure-Object -Maximum).Maximum
        }
    }

    # Extract texture streaming stats
    $textureMemoryData = $logContent | Select-String "TextureMemory:\s*(\d+)\s*MB" | ForEach-Object {
        [int]$_.Matches.Groups[1].Value
    }

    if ($textureMemoryData.Count -gt 0) {
        $profileData.RenderingStats.TextureMemory = @{
            Average = ($textureMemoryData | Measure-Object -Average).Average
            Max = ($textureMemoryData | Measure-Object -Maximum).Maximum
        }
    }
}

# Save report
$profileData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Rendering Analysis Results ===" -ForegroundColor Cyan

if ($profileData.RenderingStats.DrawCalls) {
    Write-Host "Draw Calls:" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.RenderingStats.DrawCalls.Average, 0))"
    Write-Host "  Max: $([math]::Round($profileData.RenderingStats.DrawCalls.Max, 0))"
    Write-Host ""
}

if ($profileData.RenderingStats.Triangles) {
    Write-Host "Triangles:" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.RenderingStats.Triangles.Average, 0))"
    Write-Host "  Max: $([math]::Round($profileData.RenderingStats.Triangles.Max, 0))"
    Write-Host ""
}

if ($profileData.RenderingStats.ShadowRendering) {
    Write-Host "Shadow Rendering (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.RenderingStats.ShadowRendering.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.RenderingStats.ShadowRendering.Max, 2))"
    Write-Host ""
}

if ($profileData.RenderingStats.Lighting) {
    Write-Host "Lighting (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.RenderingStats.Lighting.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.RenderingStats.Lighting.Max, 2))"
    Write-Host ""
}

if ($profileData.RenderingStats.BasePass) {
    Write-Host "Base Pass (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.RenderingStats.BasePass.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.RenderingStats.BasePass.Max, 2))"
    Write-Host ""
}

if ($profileData.RenderingStats.PostProcessing) {
    Write-Host "Post-Processing (ms):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.RenderingStats.PostProcessing.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.RenderingStats.PostProcessing.Max, 2))"
    Write-Host ""
}

if ($profileData.RenderingStats.TextureMemory) {
    Write-Host "Texture Memory (MB):" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($profileData.RenderingStats.TextureMemory.Average, 2))"
    Write-Host "  Max: $([math]::Round($profileData.RenderingStats.TextureMemory.Max, 2))"
    Write-Host ""
}

Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Optimization recommendations
Write-Host "=== Optimization Recommendations ===" -ForegroundColor Cyan

if ($profileData.RenderingStats.DrawCalls -and $profileData.RenderingStats.DrawCalls.Average -gt 3000) {
    Write-Host "- High draw call count detected. Consider mesh instancing and LODs" -ForegroundColor Yellow
}

if ($profileData.RenderingStats.ShadowRendering -and $profileData.RenderingStats.ShadowRendering.Average -gt 5) {
    Write-Host "- Shadow rendering is expensive. Optimize shadow cascades and resolution" -ForegroundColor Yellow
}

if ($profileData.RenderingStats.TextureMemory -and $profileData.RenderingStats.TextureMemory.Max -gt 2000) {
    Write-Host "- High texture memory usage. Review texture sizes and compression" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Rendering profiling complete!" -ForegroundColor Green
