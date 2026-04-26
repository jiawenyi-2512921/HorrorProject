# Runs the project's fast health checks and optional compilation checks.

param(
    [switch]$SkipBuild,
    [switch]$EditorOnly
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot

function Invoke-HealthStep {
    param(
        [string]$Name,
        [scriptblock]$Action
    )

    Write-Host "`n=== $Name ===" -ForegroundColor Cyan
    & $Action
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAILED] $Name" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Write-Host "[OK] $Name" -ForegroundColor Green
}

Write-Host "=== HorrorProject Health Validation ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectRoot" -ForegroundColor Yellow

Invoke-HealthStep -Name "PowerShell syntax" -Action {
    & (Join-Path $PSScriptRoot "ValidatePowerShellSyntax.ps1")
}

Invoke-HealthStep -Name "Final integration quality gate" -Action {
    & (Join-Path $ProjectRoot "Scripts\FinalIntegration\QualityGateValidator.ps1") -StrictMode
}

Invoke-HealthStep -Name "Asset integrity" -Action {
    & (Join-Path $ProjectRoot "Scripts\FinalIntegration\CheckAssetIntegrity.ps1")
}

Invoke-HealthStep -Name "Build configuration" -Action {
    & (Join-Path $ProjectRoot "Scripts\FinalIntegration\VerifyBuildConfigs.ps1")
}

if (-not $SkipBuild) {
    if ($EditorOnly) {
        Invoke-HealthStep -Name "Editor compilation" -Action {
            & (Join-Path $PSScriptRoot "ValidateCompilation.ps1") -EditorOnly
        }
    } else {
        Invoke-HealthStep -Name "Game compilation" -Action {
            & (Join-Path $PSScriptRoot "ValidateCompilation.ps1")
        }
        Invoke-HealthStep -Name "Editor compilation" -Action {
            & (Join-Path $PSScriptRoot "ValidateCompilation.ps1") -EditorOnly
        }
    }
}

Invoke-HealthStep -Name "Compilation report" -Action {
    & (Join-Path $PSScriptRoot "GenerateCompilationReport.ps1")
}

Write-Host "`n[SUCCESS] HorrorProject health validation completed" -ForegroundColor Green
