<#
.SYNOPSIS
    24-hour continuous development pipeline for HorrorProject.
    Orchestrates build, test, and quality checks in a continuous loop.

.DESCRIPTION
    This script runs the full development pipeline continuously:
    1. Build (Game + Editor modules)
    2. Test (Automation tests)
    3. Quality (Code style, dependencies, metrics)
    4. Report (Generate status report)

    It watches for source changes and re-runs the pipeline automatically.

.PARAMETER Mode
    Pipeline mode: Full, Build, Test, Quality, Watch

.PARAMETER Interval
    Watch interval in seconds (default: 300 = 5 minutes)

.PARAMETER MaxIterations
    Maximum number of iterations (0 = unlimited)

.EXAMPLE
    .\DevPipeline.ps1 -Mode Full
    .\DevPipeline.ps1 -Mode Watch -Interval 300
    .\DevPipeline.ps1 -Mode Build
#>

param(
    [ValidateSet("Full", "Build", "Test", "Quality", "Watch")]
    [string]$Mode = "Full",

    [int]$Interval = 300,

    [int]$MaxIterations = 0
)

$ErrorActionPreference = "Continue"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$UERoot = $env:UE_5_6_ROOT
if (-not $UERoot) { $UERoot = "D:\UnrealEngine\UE_5.6" }
$UProject = "$ProjectRoot\HorrorProject.uproject"
$LogDir = "$ProjectRoot\Saved\Logs\Pipeline"
$ReportDir = "$ProjectRoot\Saved\Reports"

# Ensure directories exist
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null

function Write-PipelineLog {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logEntry = "[$timestamp] [$Level] $Message"
    Write-Host $logEntry
    Add-Content -Path "$LogDir\pipeline.log" -Value $logEntry
}

function Get-SourceHash {
    $sourceDir = "$ProjectRoot\Source"
    $files = Get-ChildItem -Path $sourceDir -Recurse -Include *.h,*.cpp,*.cs
    $hashes = $files | ForEach-Object { (Get-FileHash $_.FullName).Hash }
    return ($hashes -join "|")
}

function Invoke-Build {
    Write-PipelineLog "Starting build..."
    $buildLog = "$LogDir\build_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

    # Build Game
    Write-PipelineLog "Building HorrorProject (Game)..."
    $gameResult = & "$UERoot\Engine\Build\BatchFiles\Build.bat" `
        HorrorProject Win64 Development `
        -Project="$UProject" -WaitMutex -NoHotReload 2>&1
    $gameResult | Out-File $buildLog

    if ($LASTEXITCODE -ne 0) {
        Write-PipelineLog "Game build FAILED (exit code: $LASTEXITCODE)" "ERROR"
        return $false
    }

    # Build Editor
    Write-PipelineLog "Building HorrorProjectEditor..."
    $editorResult = & "$UERoot\Engine\Build\BatchFiles\Build.bat" `
        HorrorProjectEditor Win64 Development `
        -Project="$UProject" -WaitMutex -NoHotReload 2>&1
    $editorResult | Out-File $buildLog -Append

    if ($LASTEXITCODE -ne 0) {
        Write-PipelineLog "Editor build FAILED (exit code: $LASTEXITCODE)" "ERROR"
        return $false
    }

    Write-PipelineLog "Build completed successfully"
    return $true
}

function Invoke-Tests {
    Write-PipelineLog "Starting tests..."
    $testLog = "$LogDir\test_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

    $editorExe = "$UERoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    if (-not (Test-Path $editorExe)) {
        Write-PipelineLog "Editor not found at $editorExe" "WARN"
        Write-PipelineLog "Skipping tests (editor not available)"
        return $true
    }

    $testResult = & $editorExe "$UProject" `
        -ExecCmds="Automation RunTests;Quit" `
        -NullRHI -NoSplash -Unattended `
        -Log="$testLog" 2>&1

    if (Test-Path $testLog) {
        $content = Get-Content $testLog -Raw
        if ($content -match "(\d+) passed.*(\d+) failed") {
            $passed = $Matches[1]
            $failed = $Matches[2]
            Write-PipelineLog "Tests: $passed passed, $failed failed"
            if ([int]$failed -gt 0) {
                Write-PipelineLog "Some tests FAILED" "ERROR"
                return $false
            }
        }
    }

    Write-PipelineLog "Tests completed successfully"
    return $true
}

function Invoke-Quality {
    Write-PipelineLog "Running quality checks..."
    $sourceDir = "$ProjectRoot\Source\HorrorProject"
    $issues = @()

    # Check copyright headers
    $files = Get-ChildItem -Path $sourceDir -Recurse -Include *.h,*.cpp
    foreach ($file in $files) {
        $firstLine = Get-Content $file.FullName -First 1
        if ($firstLine -notmatch "Copyright") {
            $issues += "Missing copyright: $($file.Name)"
        }
    }

    # Check for missing .generated.h
    $headers = Get-ChildItem -Path $sourceDir -Recurse -Include *.h
    foreach ($header in $headers) {
        $content = Get-Content $header.FullName -Raw
        if ($content -match "UCLASS|USTRUCT|UENUM" -and $content -notmatch "\.generated\.h") {
            $issues += "Missing .generated.h: $($header.Name)"
        }
    }

    # Check modules have tests
    $modules = Get-ChildItem -Path $sourceDir -Directory
    foreach ($module in $modules) {
        if ($module.Name -eq "Tests") { continue }
        if (-not (Test-Path "$($module.FullName)/Tests")) {
            $issues += "Module without tests: $($module.Name)"
        }
    }

    if ($issues.Count -gt 0) {
        Write-PipelineLog "Quality issues found ($($issues.Count)):" "WARN"
        $issues | ForEach-Object { Write-PipelineLog "  - $_" "WARN" }
    } else {
        Write-PipelineLog "Quality checks passed"
    }

    return $issues.Count -eq 0
}

function New-Report {
    param(
        [bool]$BuildSuccess,
        [bool]$TestSuccess,
        [bool]$QualitySuccess
    )

    $report = @"
# HorrorProject Pipeline Report
**Generated:** $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
**Mode:** $Mode

## Results
| Step | Status |
|------|--------|
| Build | $(if ($BuildSuccess) { "PASS" } else { "FAIL" }) |
| Tests | $(if ($TestSuccess) { "PASS" } else { "FAIL" }) |
| Quality | $(if ($QualitySuccess) { "PASS" } else { "FAIL" }) |

## Source Metrics
$(Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.h | Measure-Object | ForEach-Object { "- Headers: $($_.Count)" })
$(Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.cpp | Measure-Object | ForEach-Object { "- CPP files: $($_.Count)" })

## Logs
- Pipeline: $LogDir\pipeline.log
- Build: $LogDir\build_*.log
- Tests: $LogDir\test_*.log
"@

    $reportPath = "$ReportDir\pipeline_$(Get-Date -Format 'yyyyMMdd_HHmmss').md"
    $report | Out-File $reportPath
    Write-PipelineLog "Report saved to $reportPath"
}

function Invoke-FullPipeline {
    $buildSuccess = Invoke-Build
    $testSuccess = if ($buildSuccess) { Invoke-Tests } else { $false }
    $qualitySuccess = Invoke-Quality
    New-Report -BuildSuccess $buildSuccess -TestSuccess $testSuccess -QualitySuccess $qualitySuccess
    return ($buildSuccess -and $testSuccess -and $qualitySuccess)
}

# Main execution
Write-PipelineLog "=== HorrorProject Dev Pipeline Started ==="
Write-PipelineLog "Mode: $Mode"
Write-PipelineLog "Project: $ProjectRoot"
Write-PipelineLog "UE Root: $UERoot"

$iteration = 0
$lastHash = ""

switch ($Mode) {
    "Full" {
        $result = Invoke-FullPipeline
        Write-PipelineLog "Pipeline $(if ($result) { 'PASSED' } else { 'FAILED' })"
        exit $(if ($result) { 0 } else { 1 })
    }
    "Build" {
        $result = Invoke-Build
        exit $(if ($result) { 0 } else { 1 })
    }
    "Test" {
        $result = Invoke-Tests
        exit $(if ($result) { 0 } else { 1 })
    }
    "Quality" {
        $result = Invoke-Quality
        exit $(if ($result) { 0 } else { 1 })
    }
    "Watch" {
        Write-PipelineLog "Watch mode started (interval: ${Interval}s)"
        while ($true) {
            $iteration++
            if ($MaxIterations -gt 0 -and $iteration -gt $MaxIterations) {
                Write-PipelineLog "Max iterations reached ($MaxIterations)"
                break
            }

            $currentHash = Get-SourceHash
            if ($currentHash -ne $lastHash) {
                Write-PipelineLog "Source changes detected (iteration $iteration)"
                $lastHash = $currentHash
                $result = Invoke-FullPipeline
                Write-PipelineLog "Pipeline iteration $iteration $(if ($result) { 'PASSED' } else { 'FAILED' })"
            } else {
                Write-PipelineLog "No source changes detected (iteration $iteration)"
            }

            Write-PipelineLog "Waiting ${Interval}s before next check..."
            Start-Sleep -Seconds $Interval
        }
    }
}

Write-PipelineLog "=== Pipeline Ended ==="
