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

$ValidationCommon = Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1"
. $ValidationCommon

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$ProjectName = [System.IO.Path]::GetFileNameWithoutExtension($ProjectFile)
$UE5Root = Get-HorrorUERoot
$RunUATPath = Get-HorrorBuildScript -UERoot $UE5Root -ScriptName "RunUAT.bat"
$LogDir = Join-Path $ProjectRoot "Build\Logs\Package"
$PackageStartTime = Get-Date

if (-not $OutputDir) {
    $OutputDir = Join-Path $ProjectRoot "Build\Packages\$Configuration-$Platform-$(Get-Date -Format 'yyyyMMdd_HHmmss')"
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

    $process = Start-Process -FilePath $RunUATPath -ArgumentList $packageArgs -NoNewWindow -Wait -PassThru

    if ($process.ExitCode -ne 0) {
        throw "Package failed with exit code: $($process.ExitCode)"
    }

    Write-Log "Package complete" "SUCCESS"
}

function Get-LatestAutomationToolLog {
    $automationLogRoot = Join-Path $env:APPDATA "Unreal Engine\AutomationTool\Logs"
    if (-not (Test-Path -LiteralPath $automationLogRoot)) {
        throw "AutomationTool log root not found: $automationLogRoot"
    }

    $candidateLog = Get-ChildItem -LiteralPath $automationLogRoot -Recurse -Filter "Log.txt" -File -ErrorAction SilentlyContinue |
        Where-Object { $_.LastWriteTime -ge $PackageStartTime.AddMinutes(-5) } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    if (-not $candidateLog) {
        throw "No recent AutomationTool Log.txt found under $automationLogRoot"
    }

    return $candidateLog.FullName
}

function Test-PackageLogQuality {
    Write-Log "Checking AutomationTool log quality..."

    $automationLog = Get-LatestAutomationToolLog
    $capturedLog = Join-Path $LogDir "AutomationTool_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"
    Copy-Item -LiteralPath $automationLog -Destination $capturedLog -Force
    Write-Log "Captured AutomationTool log: $capturedLog"

    $problemMatches = Select-String -LiteralPath $automationLog -Pattern @(
        ":\s*Warning:",
        ":\s*Error:",
        "Fatal error",
        "Warning/Error Summary",
        "AutomationTool exiting with ExitCode=[1-9]"
    ) -CaseSensitive:$false

    if ($problemMatches) {
        foreach ($match in $problemMatches) {
            Write-Log "AutomationTool log issue at line $($match.LineNumber): $($match.Line.Trim())" "ERROR"
        }
        throw "Package log quality gate failed"
    }

    $cleanSummary = Select-String -LiteralPath $automationLog -Pattern "Success - 0 error\(s\), 0 warning\(s\)" -CaseSensitive:$false
    if (-not $cleanSummary) {
        throw "Package log quality gate failed: missing clean Unreal summary ('Success - 0 error(s), 0 warning(s)')"
    }

    $advisoryMatches = Select-String -LiteralPath $automationLog -Pattern @(
        "Display:.*should be fixed",
        "Display:.*silently failing"
    ) -CaseSensitive:$false

    if ($advisoryMatches) {
        foreach ($match in $advisoryMatches) {
            Write-Log "AutomationTool advisory at line $($match.LineNumber): $($match.Line.Trim())" "INFO"
        }
    }

    Write-Log "AutomationTool log quality gate passed" "SUCCESS"
}

function Test-PackageIntegrity {
    if ($SkipValidation) {
        Write-Log "Skipping validation"
        return
    }

    Write-Log "Validating package..."

    $stageRoots = @(
        (Join-Path $OutputDir "Windows"),
        (Join-Path $OutputDir "WindowsNoEditor"),
        $OutputDir
    ) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -Unique

    if (-not $stageRoots) {
        throw "Package validation failed: no staged output directory found under $OutputDir"
    }

    $validationPassed = $false
    $validationErrors = New-Object System.Collections.Generic.List[string]

    foreach ($stageRoot in $stageRoots) {
        $rootExe = Join-Path $stageRoot "$ProjectName.exe"
        $binaryExe = Join-Path $stageRoot "$ProjectName\Binaries\$Platform\$ProjectName.exe"
        $pakDir = Join-Path $stageRoot "$ProjectName\Content\Paks"

        $errors = New-Object System.Collections.Generic.List[string]

        if (-not ((Test-Path -LiteralPath $rootExe) -and ((Get-Item -LiteralPath $rootExe).Length -gt 0))) {
            $errors.Add("missing launcher executable: $rootExe")
        }

        if (-not ((Test-Path -LiteralPath $binaryExe) -and ((Get-Item -LiteralPath $binaryExe).Length -gt 0))) {
            $errors.Add("missing game executable: $binaryExe")
        }

        if (-not (Test-Path -LiteralPath $pakDir)) {
            $errors.Add("missing pak directory: $pakDir")
        } else {
            $pakFiles = Get-ChildItem -LiteralPath $pakDir -Filter "$ProjectName-*.pak" -File -ErrorAction SilentlyContinue
            $ucasFiles = Get-ChildItem -LiteralPath $pakDir -Filter "$ProjectName-*.ucas" -File -ErrorAction SilentlyContinue
            $utocFiles = Get-ChildItem -LiteralPath $pakDir -Filter "$ProjectName-*.utoc" -File -ErrorAction SilentlyContinue

            if (-not ($pakFiles | Where-Object { $_.Length -gt 0 })) {
                $errors.Add("missing non-empty project pak: $pakDir\$ProjectName-*.pak")
            }

            if (-not ($ucasFiles | Where-Object { $_.Length -gt 0 })) {
                $errors.Add("missing non-empty project IoStore ucas: $pakDir\$ProjectName-*.ucas")
            }

            if (-not ($utocFiles | Where-Object { $_.Length -gt 0 })) {
                $errors.Add("missing non-empty project IoStore utoc: $pakDir\$ProjectName-*.utoc")
            }
        }

        if ($errors.Count -eq 0) {
            Write-Log "Validated launcher: $rootExe" "SUCCESS"
            Write-Log "Validated game executable: $binaryExe" "SUCCESS"
            Write-Log "Validated pak directory: $pakDir" "SUCCESS"
            Write-Log "Package validation passed" "SUCCESS"
            $validationPassed = $true
            break
        }

        foreach ($errorItem in $errors) {
            $validationErrors.Add("[$stageRoot] $errorItem")
        }
    }

    if (-not $validationPassed) {
        foreach ($errorItem in $validationErrors) {
            Write-Log $errorItem "ERROR"
        }
        throw "Package validation failed"
    }
}

function Invoke-ArchivePackage {
    if ($Archive) {
        Write-Log "Creating archive..."

        $archiveName = "HorrorProject_$Configuration-$Platform-$(Get-Date -Format 'yyyyMMdd_HHmmss').zip"
        $archivePath = Join-Path $ProjectRoot "Build\Archives\$archiveName"

        New-Item -ItemType Directory -Force -Path (Join-Path $ProjectRoot "Build\Archives") | Out-Null

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
    Test-PackageLogQuality
    Test-PackageIntegrity
    Invoke-ArchivePackage
    Write-PackageReport

    exit 0
}
catch {
    Write-Log "PACKAGE FAILED: $_" "ERROR"
    exit 1
}
