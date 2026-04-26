# HorrorProject Daily Status Report Generator

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ReportDir = Join-Path $ProjectRoot "Docs\DailyReports"
$Today = Get-Date -Format "yyyy-MM-dd"
$ReportFile = Join-Path $ReportDir "$Today-status.md"

New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null

Write-Host "=== Generating Daily Status Report ===" -ForegroundColor Cyan

# Git stats
$CommitsToday = git log --since="midnight" --oneline | Measure-Object | Select-Object -ExpandProperty Count
$FilesChanged = git diff --stat HEAD~$CommitsToday HEAD | Select-String "files changed" | Out-String

# Code stats
$SourceDir = Join-Path $ProjectRoot "Source"
$CppFiles = (Get-ChildItem -Path $SourceDir -Recurse -Filter "*.cpp" | Measure-Object).Count
$HFiles = (Get-ChildItem -Path $SourceDir -Recurse -Filter "*.h" | Measure-Object).Count
$TotalLines = (Get-ChildItem -Path $SourceDir -Recurse -Include "*.cpp","*.h" | Get-Content | Measure-Object -Line).Lines

# Test stats
$TestFiles = (Get-ChildItem -Path $SourceDir -Recurse -Filter "*Tests.cpp" | Measure-Object).Count

# Content stats
$ContentDir = Join-Path $ProjectRoot "Content"
$ContentSize = (Get-ChildItem -Path $ContentDir -Recurse | Measure-Object -Property Length -Sum).Sum / 1GB

$Report = @"
# Daily Status Report - $Today

## Sprint Progress
**Day:** $(((Get-Date) - (Get-Date "2026-04-26")).Days + 1) of 21

## Git Activity
- **Commits today:** $CommitsToday
- **Files changed:** $FilesChanged

## Code Metrics
- **C++ files:** $CppFiles
- **Header files:** $HFiles
- **Total lines:** $TotalLines
- **Test files:** $TestFiles

## Content
- **Total size:** $([math]::Round($ContentSize, 2)) GB

## Recent Commits
``````
$(git log --since="midnight" --pretty=format:"%h - %s" | Out-String)
``````

## Tasks Status
$(& "$ProjectRoot\Scripts\GetTaskStatus.ps1" 2>$null)

## Next Steps
- [ ] Review and test today's changes
- [ ] Update documentation
- [ ] Plan tomorrow's work

---
*Generated automatically by DailyReport.ps1*
"@

$Report | Out-File -FilePath $ReportFile -Encoding UTF8
Write-Host "Report generated: $ReportFile" -ForegroundColor Green
