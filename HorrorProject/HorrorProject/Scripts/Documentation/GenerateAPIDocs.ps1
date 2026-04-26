# GenerateAPIDocs.ps1 - API Documentation Generator
# Generates Doxygen-style API documentation for HorrorProject

param(
    [string]$OutputPath = "$PSScriptRoot\..\..\Docs\API",
    [string]$SourcePath = "$PSScriptRoot\..\..\Source",
    [switch]$OpenBrowser,
    [switch]$Force
)

$ErrorActionPreference = "Stop"

Write-Host "=== HorrorProject API Documentation Generator ===" -ForegroundColor Cyan

# Check if Doxygen is installed
$doxygenPath = Get-Command doxygen -ErrorAction SilentlyContinue
if (-not $doxygenPath) {
    Write-Warning "Doxygen not found. Generating markdown documentation instead."
    $useDoxygen = $false
} else {
    $useDoxygen = $true
    Write-Host "Found Doxygen: $($doxygenPath.Source)" -ForegroundColor Green
}

# Create output directory
if (Test-Path $OutputPath) {
    if ($Force) {
        Write-Host "Cleaning existing documentation..." -ForegroundColor Yellow
        Remove-Item -Path $OutputPath -Recurse -Force
    }
}
New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null

# Generate Doxyfile if using Doxygen
if ($useDoxygen) {
    $doxyfilePath = "$PSScriptRoot\Doxyfile"

    $doxyfileContent = @"
# Doxyfile for HorrorProject
PROJECT_NAME           = "HorrorProject"
PROJECT_BRIEF          = "Unreal Engine 5 Horror Game"
OUTPUT_DIRECTORY       = "$OutputPath"
INPUT                  = "$SourcePath"
RECURSIVE              = YES
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = YES
EXTRACT_STATIC         = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
HTML_OUTPUT            = html
FILE_PATTERNS          = *.h *.cpp
EXCLUDE_PATTERNS       = */Intermediate/* */Binaries/* */DerivedDataCache/* */.vs/*
HAVE_DOT               = YES
CALL_GRAPH             = YES
CALLER_GRAPH           = YES
CLASS_DIAGRAMS         = YES
UML_LOOK               = YES
TEMPLATE_RELATIONS     = YES
INCLUDE_GRAPH          = YES
INCLUDED_BY_GRAPH      = YES
COLLABORATION_GRAPH    = YES
GRAPHICAL_HIERARCHY    = YES
DIRECTORY_GRAPH        = YES
DOT_IMAGE_FORMAT       = svg
INTERACTIVE_SVG        = YES
"@

    Set-Content -Path $doxyfilePath -Value $doxyfileContent
    Write-Host "Generated Doxyfile at: $doxyfilePath" -ForegroundColor Green

    # Run Doxygen
    Write-Host "Running Doxygen..." -ForegroundColor Cyan
    & doxygen $doxyfilePath

    if ($LASTEXITCODE -eq 0) {
        Write-Host "API documentation generated successfully!" -ForegroundColor Green
        $indexPath = Join-Path $OutputPath "html\index.html"
        Write-Host "Documentation available at: $indexPath" -ForegroundColor Green

        if ($OpenBrowser -and (Test-Path $indexPath)) {
            Start-Process $indexPath
        }
    } else {
        Write-Error "Doxygen failed with exit code $LASTEXITCODE"
    }
} else {
    # Fallback: Generate simple markdown documentation
    Write-Host "Generating markdown API documentation..." -ForegroundColor Cyan

    $modules = @("AI", "Audio", "Evidence", "Game", "Interaction", "Performance", "Player", "Save", "UI", "VFX", "Variant_Horror")

    foreach ($module in $modules) {
        $modulePath = Join-Path $SourcePath "HorrorProject\$module"
        if (-not (Test-Path $modulePath)) { continue }

        $moduleDocPath = Join-Path $OutputPath "$module.md"
        $content = "# $module Module API`n`n"

        $headers = Get-ChildItem -Path $modulePath -Filter "*.h" -Recurse | Where-Object { $_.FullName -notmatch "Intermediate" }

        foreach ($header in $headers) {
            $content += "## $($header.BaseName)`n`n"
            $content += "**File:** ``$($header.Name)```n`n"

            $fileContent = Get-Content $header.FullName -Raw

            # Extract classes
            if ($fileContent -match "UCLASS\([^\)]*\)[\s\S]*?class\s+\w+\s+(\w+)\s*:") {
                $className = $Matches[1]
                $content += "### Class: $className`n`n"
            }

            # Extract functions
            $functions = [regex]::Matches($fileContent, "UFUNCTION\([^\)]*\)[\s\S]*?(\w+\s+\w+\([^\)]*\))")
            foreach ($func in $functions) {
                $content += "- ``$($func.Groups[1].Value)```n"
            }

            $content += "`n"
        }

        Set-Content -Path $moduleDocPath -Value $content
        Write-Host "Generated: $moduleDocPath" -ForegroundColor Green
    }

    # Generate index
    $indexContent = @"
# HorrorProject API Documentation

## Modules

$(foreach ($module in $modules) { "- [$module]($module.md)" }) -join "`n"

## Quick Links

- [System Architecture](../Architecture/SystemOverview.md)
- [Developer Guide](../Developer/GettingStarted.md)
- [Event Bus System](../Architecture/EventBus.md)

---
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
"@

    Set-Content -Path (Join-Path $OutputPath "index.md") -Value $indexContent
    Write-Host "Generated API documentation index" -ForegroundColor Green
}

Write-Host "`n=== Documentation Generation Complete ===" -ForegroundColor Cyan
