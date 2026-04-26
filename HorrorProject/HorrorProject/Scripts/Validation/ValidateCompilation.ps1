# Validate project compilation
# Attempts to build the project and reports errors

param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64",
    [switch]$EditorOnly = $false
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$UE5Root = Get-HorrorUERoot
$UE5Path = Join-Path $UE5Root "Engine\Build\BatchFiles"
$UBT = Get-HorrorBuildScript -UERoot $UE5Root -ScriptName "Build.bat"

Write-Host "=== Compilation Validation ===" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Platform: $Platform" -ForegroundColor Yellow
Write-Host "Project: $ProjectFile" -ForegroundColor Yellow

Write-Host "`n=== Running Dependency Check ===" -ForegroundColor Cyan
& (Join-Path $PSScriptRoot "CheckDependencies.ps1")
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Dependency check failed" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== Running Include Check ===" -ForegroundColor Cyan
& (Join-Path $PSScriptRoot "CheckIncludes.ps1")

Write-Host "`n=== Building Project ===" -ForegroundColor Cyan

$LogFile = Join-Path $ProjectRoot "Saved\Logs\CompilationValidation_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"
$LogDir = Split-Path $LogFile -Parent
if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
}

try {
    if ($EditorOnly) {
        Write-Host "Building Editor target..." -ForegroundColor Yellow
        $Target = "HorrorProjectEditor"
    } else {
        Write-Host "Building Game target..." -ForegroundColor Yellow
        $Target = "HorrorProject"
    }

    $BuildArgs = @(
        $Target,
        $Platform,
        $Configuration,
        "`"$ProjectFile`"",
        "-NoHotReloadFromIDE",
        "-Progress"
    )

    Write-Host "Command: $UBT $($BuildArgs -join ' ')" -ForegroundColor Gray

    $Process = Start-Process -FilePath $UBT -ArgumentList $BuildArgs -NoNewWindow -Wait -PassThru -RedirectStandardOutput $LogFile

    if ($Process.ExitCode -eq 0) {
        Write-Host "`n[SUCCESS] Compilation completed successfully" -ForegroundColor Green

        # Check for warnings
        $LogContent = Get-Content $LogFile -Raw
        $WarningCount = ([regex]::Matches($LogContent, "warning")).Count

        if ($WarningCount -gt 0) {
            Write-Host "[INFO] Build completed with $WarningCount warnings" -ForegroundColor Yellow
            Write-Host "Check log file: $LogFile" -ForegroundColor Gray
        } else {
            Write-Host "[INFO] Build completed with zero warnings" -ForegroundColor Green
        }

        exit 0
    } else {
        Write-Host "`n[ERROR] Compilation failed with exit code $($Process.ExitCode)" -ForegroundColor Red
        Write-Host "Check log file: $LogFile" -ForegroundColor Yellow

        # Extract common compiler, linker, and UnrealHeaderTool errors from the log.
        $LogContent = Get-Content $LogFile -Raw
        $Errors = @(
            [regex]::Matches($LogContent, "(?im)^.*error [A-Z][A-Z0-9]+:.*$")
            [regex]::Matches($LogContent, "(?im)^.*fatal error [A-Z][A-Z0-9]+:.*$")
            [regex]::Matches($LogContent, "(?im)^.*\bError:\s+.*$")
        ) | ForEach-Object { $_ } | Select-Object -First 10

        if ($Errors.Count -gt 0) {
            Write-Host "`nFirst 10 errors:" -ForegroundColor Red
            $Errors | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
        }

        exit 1
    }
} catch {
    Write-Host "[ERROR] Build process failed: $_" -ForegroundColor Red
    exit 1
}
