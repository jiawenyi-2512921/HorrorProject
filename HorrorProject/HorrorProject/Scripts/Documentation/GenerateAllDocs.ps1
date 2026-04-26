# Master Documentation Script
# Generates all documentation for HorrorProject

param(
    [switch]$API,
    [switch]$Diagrams,
    [switch]$Validate,
    [switch]$Publish,
    [switch]$All,
    [string]$PublishFormat = "HTML"
)

$ErrorActionPreference = "Stop"
$ScriptDir = $PSScriptRoot

Write-Host "=== HorrorProject Documentation Master Script ===" -ForegroundColor Cyan
Write-Host "Started: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

$startTime = Get-Date

# Determine what to run
$runAPI = $API -or $All
$runDiagrams = $Diagrams -or $All
$runValidate = $Validate -or $All
$runPublish = $Publish -or $All

# API Documentation
if ($runAPI) {
    Write-Host ">>> Generating API Documentation" -ForegroundColor Yellow
    & "$ScriptDir\GenerateAPIDocs.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "API documentation generation failed"
    }
    Write-Host ""
}

# Architecture Diagrams
if ($runDiagrams) {
    Write-Host ">>> Generating Architecture Diagrams" -ForegroundColor Yellow
    & "$ScriptDir\GenerateDiagrams.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Diagram generation failed"
    }
    Write-Host ""
}

# Validation
if ($runValidate) {
    Write-Host ">>> Validating Documentation" -ForegroundColor Yellow
    & "$ScriptDir\ValidateDocs.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Documentation validation found issues"
    }
    Write-Host ""
}

# Publishing
if ($runPublish) {
    Write-Host ">>> Publishing Documentation" -ForegroundColor Yellow
    & "$ScriptDir\PublishDocs.ps1" -Format $PublishFormat -Validate
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Documentation publishing failed"
    }
    Write-Host ""
}

# Summary
$endTime = Get-Date
$duration = $endTime - $startTime

Write-Host "=== Documentation Generation Complete ===" -ForegroundColor Cyan
Write-Host "Duration: $($duration.TotalSeconds) seconds" -ForegroundColor Gray
Write-Host "Completed: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray

# Generate summary report
$docsPath = "$ScriptDir\..\..\Docs"
$mdFiles = Get-ChildItem -Path $docsPath -Filter "*.md" -Recurse
$totalSize = ($mdFiles | Measure-Object -Property Length -Sum).Sum

Write-Host "`nDocumentation Statistics:" -ForegroundColor Cyan
Write-Host "  Total markdown files: $($mdFiles.Count)" -ForegroundColor Gray
Write-Host "  Total size: $([math]::Round($totalSize / 1KB, 2)) KB" -ForegroundColor Gray

# List generated files
Write-Host "`nGenerated Documentation:" -ForegroundColor Cyan
$categories = @{
    "Architecture" = "Architecture\*.md"
    "Developer" = "Developer\*.md"
    "Technical" = "Technical\*.md"
    "API" = "API\*.md"
}

foreach ($category in $categories.Keys) {
    $pattern = $categories[$category]
    $files = Get-ChildItem -Path $docsPath -Filter $pattern -Recurse -ErrorAction SilentlyContinue
    if ($files) {
        Write-Host "  $category ($($files.Count) files):" -ForegroundColor Yellow
        foreach ($file in $files) {
            Write-Host "    - $($file.Name)" -ForegroundColor Gray
        }
    }
}

Write-Host "`nNext Steps:" -ForegroundColor Cyan
Write-Host "  1. Review generated documentation in Docs/" -ForegroundColor Gray
Write-Host "  2. Open Docs/README.md for navigation" -ForegroundColor Gray
Write-Host "  3. Run validation: .\ValidateDocs.ps1" -ForegroundColor Gray
Write-Host "  4. Publish: .\PublishDocs.ps1 -Format HTML" -ForegroundColor Gray

Write-Host "`n=== All Done! ===" -ForegroundColor Green
