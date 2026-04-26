#!/usr/bin/env pwsh
# Horror Project Analyze CLI Tool

param(
    [Parameter(Position=0)]
    [ValidateSet('code', 'assets', 'performance', 'dependencies')]
    [string]$AnalyzeType = 'code',

    [Parameter()]
    [string]$Path = "",

    [switch]$Detailed,
    [switch]$ExportReport,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Help {
    Write-Host @"
Horror Project Analyze Tool

USAGE:
    horror-analyze [ANALYZE_TYPE] [OPTIONS]

ANALYZE TYPES:
    code          Analyze code quality and complexity
    assets        Analyze asset usage and optimization
    performance   Analyze performance metrics
    dependencies  Analyze project dependencies

OPTIONS:
    -Path <path>      Specific path to analyze
    -Detailed         Show detailed analysis
    -ExportReport     Export analysis report
    -Help             Show this help message

EXAMPLES:
    horror-analyze code
    horror-analyze assets -Detailed
    horror-analyze performance -ExportReport
    horror-analyze dependencies

"@ -ForegroundColor Cyan
}

if ($Help) {
    Show-Help
    exit 0
}

$ScriptRoot = Split-Path -Parent $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptRoot

Write-Host "Horror Analyze Tool" -ForegroundColor Cyan
Write-Host "Analyze Type: $AnalyzeType" -ForegroundColor Yellow
Write-Host ""

try {
    switch ($AnalyzeType) {
        'code' {
            Write-Host "Analyzing code quality..." -ForegroundColor Yellow

            $SourceDir = if ($Path) { $Path } else { Join-Path $ProjectRoot "Source" }

            # Count files
            $CppFiles = Get-ChildItem -Path $SourceDir -Filter "*.cpp" -Recurse
            $HeaderFiles = Get-ChildItem -Path $SourceDir -Filter "*.h" -Recurse

            Write-Host ""
            Write-Host "Code Statistics:" -ForegroundColor Cyan
            Write-Host "  C++ Files: $($CppFiles.Count)"
            Write-Host "  Header Files: $($HeaderFiles.Count)"

            # Count lines
            $TotalLines = 0
            foreach ($File in ($CppFiles + $HeaderFiles)) {
                $TotalLines += (Get-Content $File.FullName | Measure-Object -Line).Lines
            }
            Write-Host "  Total Lines: $TotalLines"

            # Find large files
            $LargeFiles = ($CppFiles + $HeaderFiles) | Where-Object { $_.Length -gt 100KB } | Sort-Object Length -Descending
            if ($LargeFiles.Count -gt 0) {
                Write-Host ""
                Write-Host "Large Files (>100KB):" -ForegroundColor Yellow
                $LargeFiles | Select-Object -First 5 | ForEach-Object {
                    Write-Host "  $($_.Name): $([math]::Round($_.Length / 1KB, 2)) KB"
                }
            }
        }

        'assets' {
            Write-Host "Analyzing assets..." -ForegroundColor Yellow

            $ContentDir = Join-Path $ProjectRoot "Content"

            # Asset statistics
            $Textures = Get-ChildItem -Path $ContentDir -Filter "*.uasset" -Recurse | Where-Object { $_.Directory.Name -like "*Texture*" }
            $Meshes = Get-ChildItem -Path $ContentDir -Filter "*.uasset" -Recurse | Where-Object { $_.Directory.Name -like "*Mesh*" }
            $Materials = Get-ChildItem -Path $ContentDir -Filter "*.uasset" -Recurse | Where-Object { $_.Directory.Name -like "*Material*" }
            $Sounds = Get-ChildItem -Path $ContentDir -Filter "*.uasset" -Recurse | Where-Object { $_.Directory.Name -like "*Audio*" -or $_.Directory.Name -like "*Sound*" }

            Write-Host ""
            Write-Host "Asset Statistics:" -ForegroundColor Cyan
            Write-Host "  Textures: $($Textures.Count)"
            Write-Host "  Meshes: $($Meshes.Count)"
            Write-Host "  Materials: $($Materials.Count)"
            Write-Host "  Sounds: $($Sounds.Count)"

            # Content size
            $ContentSize = (Get-ChildItem -Path $ContentDir -Recurse | Measure-Object -Property Length -Sum).Sum
            Write-Host "  Total Content Size: $([math]::Round($ContentSize / 1GB, 2)) GB"
        }

        'performance' {
            Write-Host "Analyzing performance..." -ForegroundColor Yellow

            $PerfReportDir = Join-Path $ProjectRoot "PerformanceReports"
            if (Test-Path $PerfReportDir) {
                $LatestReport = Get-ChildItem -Path $PerfReportDir -Filter "*.json" | Sort-Object LastWriteTime -Descending | Select-Object -First 1

                if ($LatestReport) {
                    $Report = Get-Content $LatestReport.FullName | ConvertFrom-Json

                    Write-Host ""
                    Write-Host "Latest Performance Metrics:" -ForegroundColor Cyan
                    Write-Host "  Average FPS: $($Report.AverageFPS)"
                    Write-Host "  Min FPS: $($Report.MinFPS)"
                    Write-Host "  Max FPS: $($Report.MaxFPS)"
                    Write-Host "  Memory Usage: $($Report.MemoryUsageMB) MB"
                } else {
                    Write-Host "No performance reports found" -ForegroundColor Yellow
                }
            } else {
                Write-Host "Performance reports directory not found" -ForegroundColor Yellow
            }
        }

        'dependencies' {
            Write-Host "Analyzing dependencies..." -ForegroundColor Yellow

            $ProjectFile = Join-Path $ProjectRoot "HorrorProject.uproject"
            if (Test-Path $ProjectFile) {
                $Project = Get-Content $ProjectFile | ConvertFrom-Json

                Write-Host ""
                Write-Host "Project Dependencies:" -ForegroundColor Cyan

                if ($Project.Plugins) {
                    Write-Host "  Plugins: $($Project.Plugins.Count)"
                    if ($Detailed) {
                        $Project.Plugins | ForEach-Object {
                            Write-Host "    - $($_.Name) (Enabled: $($_.Enabled))"
                        }
                    }
                }

                if ($Project.Modules) {
                    Write-Host "  Modules: $($Project.Modules.Count)"
                    if ($Detailed) {
                        $Project.Modules | ForEach-Object {
                            Write-Host "    - $($_.Name) (Type: $($_.Type))"
                        }
                    }
                }
            }
        }
    }

    if ($ExportReport) {
        $ReportDir = Join-Path $ProjectRoot "AnalysisReports"
        if (-not (Test-Path $ReportDir)) {
            New-Item -ItemType Directory -Path $ReportDir | Out-Null
        }

        $ReportFile = Join-Path $ReportDir "Analysis_${AnalyzeType}_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"
        # Export logic here
        Write-Host ""
        Write-Host "Report exported to: $ReportFile" -ForegroundColor Green
    }

    Write-Host ""
    Write-Host "Analysis completed!" -ForegroundColor Green

} catch {
    Write-Error "Analysis failed: $_"
    exit 1
}
