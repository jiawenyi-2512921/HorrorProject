# HorrorProject - Automated Build Script
# 21-Day Sprint Optimized

param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64",
    [switch]$Clean,
    [switch]$SkipCook,
    [switch]$FastBuild
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$ProjectFile = "$ProjectRoot\HorrorProject.uproject"
$UE5Path = "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles"
$LogDir = "$ProjectRoot\Build\Logs"
$BuildStartTime = Get-Date

# Ensure log directory exists
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogFile = "$LogDir\Build_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
    Add-Content -Path $LogFile -Value $logMessage
}

function Test-Prerequisites {
    Write-Log "Checking prerequisites..."

    if (-not (Test-Path $ProjectFile)) {
        throw "Project file not found: $ProjectFile"
    }

    if (-not (Test-Path "$UE5Path\Build.bat")) {
        throw "UE5 Build.bat not found at: $UE5Path"
    }

    Write-Log "Prerequisites OK" "SUCCESS"
}

function Invoke-CleanBuild {
    if ($Clean) {
        Write-Log "Cleaning build artifacts..."

        $cleanDirs = @(
            "$ProjectRoot\Binaries",
            "$ProjectRoot\Intermediate",
            "$ProjectRoot\DerivedDataCache",
            "$ProjectRoot\Saved\Cooked"
        )

        foreach ($dir in $cleanDirs) {
            if (Test-Path $dir) {
                Write-Log "Removing: $dir"
                Remove-Item -Path $dir -Recurse -Force -ErrorAction SilentlyContinue
            }
        }

        Write-Log "Clean complete" "SUCCESS"
    }
}

function Invoke-BuildEditor {
    Write-Log "Building editor..."

    $buildArgs = @(
        "HorrorProjectEditor",
        $Platform,
        $Configuration,
        "`"$ProjectFile`"",
        "-waitmutex"
    )

    if ($FastBuild) {
        $buildArgs += "-NoEngineChanges"
    }

    Write-Log "Running: Build.bat $($buildArgs -join ' ')"

    $process = Start-Process -FilePath "$UE5Path\Build.bat" -ArgumentList $buildArgs -NoNewWindow -Wait -PassThru

    if ($process.ExitCode -ne 0) {
        throw "Build failed with exit code: $($process.ExitCode)"
    }

    Write-Log "Editor build complete" "SUCCESS"
}

function Invoke-CookContent {
    if (-not $SkipCook) {
        Write-Log "Cooking content..."

        $cookArgs = @(
            "`"$ProjectFile`"",
            "-run=cook",
            "-targetplatform=$Platform",
            "-iterate",
            "-unversioned"
        )

        if ($FastBuild) {
            $cookArgs += "-iterativecooking"
        }

        Write-Log "Running: RunUAT.bat $($cookArgs -join ' ')"

        $process = Start-Process -FilePath "$UE5Path\RunUAT.bat" -ArgumentList $cookArgs -NoNewWindow -Wait -PassThru

        if ($process.ExitCode -ne 0) {
            Write-Log "Cook completed with warnings (exit code: $($process.ExitCode))" "WARN"
        } else {
            Write-Log "Cook complete" "SUCCESS"
        }
    }
}

function Write-BuildReport {
    $buildDuration = (Get-Date) - $BuildStartTime

    Write-Log "========================================" "SUCCESS"
    Write-Log "BUILD COMPLETE" "SUCCESS"
    Write-Log "Configuration: $Configuration" "SUCCESS"
    Write-Log "Platform: $Platform" "SUCCESS"
    Write-Log "Duration: $($buildDuration.ToString('hh\:mm\:ss'))" "SUCCESS"
    Write-Log "Log: $LogFile" "SUCCESS"
    Write-Log "========================================" "SUCCESS"
}

# Main execution
try {
    Write-Log "Starting HorrorProject build..."
    Write-Log "Configuration: $Configuration, Platform: $Platform"

    Test-Prerequisites
    Invoke-CleanBuild
    Invoke-BuildEditor
    Invoke-CookContent
    Write-BuildReport

    exit 0
}
catch {
    Write-Log "BUILD FAILED: $_" "ERROR"
    Write-Log "Check log: $LogFile" "ERROR"
    exit 1
}
