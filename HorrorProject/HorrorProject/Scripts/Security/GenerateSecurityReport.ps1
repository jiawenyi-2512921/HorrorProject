# Generate Comprehensive Security Report for HorrorProject

param(
    [string]$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
)

$ErrorActionPreference = "Stop"

Write-Host "Generating Comprehensive Security Report..." -ForegroundColor Cyan
Write-Host "=" * 80 -ForegroundColor Gray

# Run all security scans
Write-Host "`n[1/3] Running Static Analysis..." -ForegroundColor Yellow
& "$PSScriptRoot\StaticAnalysis.ps1" -ProjectRoot $ProjectRoot

Write-Host "`n[2/3] Running Memory Safety Analysis..." -ForegroundColor Yellow
& "$PSScriptRoot\MemorySafety.ps1" -ProjectRoot $ProjectRoot

Write-Host "`n[3/3] Running Input Validation Analysis..." -ForegroundColor Yellow
& "$PSScriptRoot\InputValidation.ps1" -ProjectRoot $ProjectRoot

Write-Host "`n" + ("=" * 80) -ForegroundColor Gray
Write-Host "All security scans completed!" -ForegroundColor Green
Write-Host "Reports saved to: $ProjectRoot\Docs\Security\" -ForegroundColor Cyan
