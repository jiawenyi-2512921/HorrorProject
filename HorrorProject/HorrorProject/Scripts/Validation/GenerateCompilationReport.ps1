# Generates a current compilation and engineering-health report.

param(
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$ReportDir = if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    Join-Path $ProjectRoot "Docs\Compilation"
} else {
    $OutputDir
}

Write-Host "=== Generating Compilation Report ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectFile" -ForegroundColor Yellow

New-Item -ItemType Directory -Path $ReportDir -Force | Out-Null

function Get-FileCount {
    param(
        [string]$RelativePath,
        [string]$Filter = "*.cpp"
    )

    $Path = Join-Path $ProjectRoot $RelativePath
    if (-not (Test-Path -LiteralPath $Path)) {
        return 0
    }

    return @(Get-ChildItem -LiteralPath $Path -Filter $Filter -File -Recurse).Count
}

function Get-BuildModules {
    param(
        [string]$BuildFile
    )

    if (-not (Test-Path -LiteralPath $BuildFile)) {
        return @()
    }

    $Content = Get-Content -LiteralPath $BuildFile -Raw
    return @([regex]::Matches($Content, '"([^"]+)"') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
}

function Get-PowerShellSyntaxSummary {
    $ScanRoots = @("Scripts", "Source", "Docs")
    $ExcludedRoots = @("Binaries", "DerivedDataCache", "Intermediate", "Saved", ".git", "Docs\Production")
    $Files = foreach ($ScanRoot in $ScanRoots) {
        $AbsolutePath = Join-Path $ProjectRoot $ScanRoot
        if (Test-Path -LiteralPath $AbsolutePath) {
            Get-ChildItem -LiteralPath $AbsolutePath -Filter "*.ps1" -File -Recurse
        }
    }

    $Files = @($Files | Where-Object {
        $RelativePath = $_.FullName.Substring($ProjectRoot.Length + 1)
        $IsExcluded = $false
        foreach ($ExcludedRoot in $ExcludedRoots) {
            $NormalizedExcluded = $ExcludedRoot.TrimEnd("\", "/")
            if ($RelativePath -eq $NormalizedExcluded -or $RelativePath.StartsWith("$NormalizedExcluded\")) {
                $IsExcluded = $true
                break
            }
        }
        -not $IsExcluded
    })

    $ErrorCount = 0
    $FilesWithErrors = 0
    foreach ($File in $Files) {
        $Tokens = $null
        $Errors = $null
        [System.Management.Automation.Language.Parser]::ParseFile($File.FullName, [ref]$Tokens, [ref]$Errors) | Out-Null
        if ($Errors.Count -gt 0) {
            $FilesWithErrors++
            $ErrorCount += $Errors.Count
        }
    }

    return [PSCustomObject]@{
        Files = $Files.Count
        FilesWithErrors = $FilesWithErrors
        Errors = $ErrorCount
    }
}

$RuntimeBuildFile = Join-Path $ProjectRoot "Source\HorrorProject\HorrorProject.Build.cs"
$EditorBuildFile = Join-Path $ProjectRoot "Source\HorrorProjectEditor\HorrorProjectEditor.Build.cs"
$RuntimeModules = Get-BuildModules -BuildFile $RuntimeBuildFile
$EditorModules = Get-BuildModules -BuildFile $EditorBuildFile
$PowerShellSummary = Get-PowerShellSyntaxSummary

$RuntimeModuleText = ($RuntimeModules | ForEach-Object { "- $_" }) -join "`n"
$EditorModuleText = ($EditorModules | ForEach-Object { "- $_" }) -join "`n"

$ReportFile = Join-Path $ReportDir "CompilationReport.md"
$Report = @"
# Compilation Report

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Project

- Project file: $ProjectFile
- Engine association: 5.6
- Platform target: Win64
- Runtime target: HorrorProject
- Editor target: HorrorProjectEditor

## Source Inventory

- VFX cpp files: $(Get-FileCount -RelativePath "Source\HorrorProject\VFX")
- Audio cpp files: $(Get-FileCount -RelativePath "Source\HorrorProject\Audio")
- UI cpp files: $(Get-FileCount -RelativePath "Source\HorrorProject\UI")
- Runtime test cpp files: $(Get-FileCount -RelativePath "Source\HorrorProject" -Filter "*Tests.cpp")
- Editor tool cpp files: $(Get-FileCount -RelativePath "Source\HorrorProjectEditor\Tools")

## Runtime Module Dependencies

$RuntimeModuleText

## Editor Module Dependencies

$EditorModuleText

## Validation Entry Points

- Game build: `Scripts\Validation\ValidateCompilation.ps1`
- Editor build: `Scripts\Validation\ValidateCompilation.ps1 -EditorOnly`
- Dependency scan: `Scripts\Validation\CheckDependencies.ps1`
- Include scan: `Scripts\Validation\CheckIncludes.ps1`
- PowerShell syntax scan: `Scripts\Validation\ValidatePowerShellSyntax.ps1`

## Current Engineering Notes

- Niagara, UMG, networking, rendering, navigation, JSON, and editor dependencies are declared in Build.cs.
- Legacy automation tests are intentionally quarantined by `HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0` until they are migrated to current APIs.
- PowerShell syntax scan currently covers $($PowerShellSummary.Files) scripts.
- PowerShell files with syntax errors: $($PowerShellSummary.FilesWithErrors)
- PowerShell syntax error count: $($PowerShellSummary.Errors)

## Recommended Next Work

1. Keep Game and Editor builds at zero warnings.
2. Migrate legacy automation tests module by module before re-enabling them.
3. Repair or retire corrupted PowerShell tools, starting with validation, build, package, and final-integration scripts.
4. Add Cook, Package, and Editor smoke tests after script syntax debt is reduced.
"@

Set-Content -Path $ReportFile -Value $Report -Encoding UTF8
Write-Host "[OK] Generated: $ReportFile" -ForegroundColor Green

$IssuesFile = Join-Path $ReportDir "CompilationIssues.md"
$Issues = @"
# Compilation Issues

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Blocking Build Issues

- None reported by this static report generator. Run `Scripts\Validation\ValidateCompilation.ps1` and `Scripts\Validation\ValidateCompilation.ps1 -EditorOnly` for fresh build proof.

## Known Engineering Debt

- Legacy automation tests are disabled behind `HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0`.
- PowerShell syntax scan reports $($PowerShellSummary.Errors) syntax errors across $($PowerShellSummary.FilesWithErrors) files.
- Several older utility scripts still contain machine-specific paths and should be normalized to shared validation helpers or project-root-relative defaults.

## Quality Gate Direction

- Compilation must remain zero-warning for Game and Editor targets.
- Script syntax issues should trend to zero before these tools are used as release gates.
- Re-enable automation tests only after each suite is migrated and verified.
"@

Set-Content -Path $IssuesFile -Value $Issues -Encoding UTF8
Write-Host "[OK] Generated: $IssuesFile" -ForegroundColor Green

Write-Host "Reports saved to: $ReportDir" -ForegroundColor Green
