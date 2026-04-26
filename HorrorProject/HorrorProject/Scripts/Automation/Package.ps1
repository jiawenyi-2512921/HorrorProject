# HorrorProject - Automated Package Script
# 21-Day Sprint Optimized

param(
    [string]$Configuration = "Shipping",
    [string]$Platform = "Win64",
    [string]$OutputDir = "",
    [switch]$Archive,
    [switch]$SkipValidation
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$ProjectFile = "$ProjectRoot\HorrorProject.uproject"
$UE5Path = "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles"
$LogDir = "$ProjectRoot\Build\Logs\Package"
$PackageStartTime = Get-Date

if (-not $OutputDir) {
    $OutputDir = "$ProjectRoot\Build\Packages\$Configuration-$Platform-$(Get-Date -Format 'yyyyMMdd_HHmmss')"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$LogFile = "$LogDir\Package_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
    Add-Content -Path $LogFile -Value $logMessage
}

function Invoke-PackageBuild {
    Write-Log "Packaging project..."
    Write-Log "Output: $OutputDir"

    $packageArgs = @(
        "BuildCookRun",
        "-project=`"$ProjectFile`"",
        "-platform=$Platform",
        "-clientconfig=$Configuration",
        "-cook",
        "-stage",
        "-package",
        "-pak",
        "-archive",
        "-archivedirectory=`"$OutputDir`"",
        "-build",
        "-compressed",
        "-prereqs",
        "-utf8output"
    )

    Write-Log "Running: RunUAT.bat $($packageArgs -join ' ')"

    $process = Start-Process -FilePath "$UE5Path\RunUAT.bat" -ArgumentList $packageArgs -NoNewWindow -Wait -PassThru

    if ($process.ExitCode -ne 0) {
        throw "Package failed with exit code: $($process.ExitCode)"
    }

    Write-Log "Package complete" "SUCCESS"
}

function Test-PackageIntegrity {
    if ($SkipValidation) {
        Write-Log "Skipping validation"
        return
    }

    Write-Log "Validating package..."

    $requiredFiles = @(
        "HorrorProject.exe",
        "HorrorProject\Content\Paks\HorrorProject-WindowsNoEditor.pak"
    )

    $allFilesExist = $true
    foreach ($file in $requiredFiles) {
        $fullPath = Join-Path $OutputDir "WindowsNoEditor\$file"
        if (-not (Test-Path $fullPath)) {
            Write-Log "Missing required file: $file" "ERROR"
            $allFilesExist = $false
        }
    }

    if ($allFilesExist) {
        Write-Log "Package validation passed" "SUCCESS"
    } else {
        throw "Package validation failed"
    }
}

function Invoke-ArchivePackage {
    if ($Archive) {
        Write-Log "Creating archive..."

        $archiveName = "HorrorProject_$Configuration-$Platform-$(Get-Date -Format 'yyyyMMdd_HHmmss').zip"
        $archivePath = "$ProjectRoot\Build\Archives\$archiveName"

        New-Item -ItemType Directory -Force -Path "$ProjectRoot\Build\Archives" | Out-Null

        Compress-Archive -Path "$OutputDir\*" -DestinationPath $archivePath -Force

        $archiveSize = (Get-Item $archivePath).Length / 1MB
        Write-Log "Archive created: $archivePath ($([math]::Round($archiveSize, 2)) MB)" "SUCCESS"
    }
}

function Write-PackageReport {
    $packageDuration = (Get-Date) - $PackageStartTime

    $packageSize = 0
    if (Test-Path $OutputDir) {
        $packageSize = (Get-ChildItem -Path $OutputDir -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB
    }

    Write-Log "========================================" "SUCCESS"
    Write-Log "PACKAGE COMPLETE" "SUCCESS"
    Write-Log "Configuration: $Configuration" "SUCCESS"
    Write-Log "Platform: $Platform" "SUCCESS"
    Write-Log "Output: $OutputDir" "SUCCESS"
    Write-Log "Size: $([math]::Round($packageSize, 2)) MB" "SUCCESS"
    Write-Log "Duration: $($packageDuration.ToString('hh\:mm\:ss'))" "SUCCESS"
    Write-Log "========================================" "SUCCESS"
}

# Main execution
try {
    Write-Log "Starting HorrorProject packaging..."

    Invoke-PackageBuild
    Test-PackageIntegrity
    Invoke-ArchivePackage
    Write-PackageReport

    exit 0
}
catch {
    Write-Log "PACKAGE FAILED: $_" "ERROR"
    exit 1
}
