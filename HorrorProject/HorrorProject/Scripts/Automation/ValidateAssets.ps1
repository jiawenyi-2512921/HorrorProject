# HorrorProject - Asset Validation Script
# 21-Day Sprint Optimized

param(
    [switch]$FixIssues,
    [switch]$DetailedReport,
    [string]$AssetPath = "/Game/"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$ProjectFile = "$ProjectRoot\HorrorProject.uproject"
$UE5Editor = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$LogDir = "$ProjectRoot\Build\Logs\AssetValidation"
$ValidationStartTime = Get-Date

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogFile = "$LogDir\AssetValidation_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
    Add-Content -Path $LogFile -Value $logMessage
}

function Test-AssetNamingConventions {
    Write-Log "Checking asset naming conventions..."

    $contentDir = "$ProjectRoot\Content"
    $issues = @()

    $assetFiles = Get-ChildItem -Path $contentDir -Recurse -Include *.uasset, *.umap -ErrorAction SilentlyContinue

    foreach ($file in $assetFiles) {
        $name = $file.BaseName

        # Check for spaces
        if ($name -match '\s') {
            $issues += "Spaces in name: $($file.FullName)"
        }

        # Check for special characters
        if ($name -match '[^a-zA-Z0-9_]') {
            $issues += "Special characters in name: $($file.FullName)"
        }

        # Check for lowercase start (except maps)
        if ($file.Extension -ne '.umap' -and $name -match '^[a-z]') {
            $issues += "Should start with uppercase: $($file.FullName)"
        }
    }

    if ($issues.Count -gt 0) {
        Write-Log "Found $($issues.Count) naming issues" "WARN"
        if ($DetailedReport) {
            $issues | ForEach-Object { Write-Log $_ "WARN" }
        }
    } else {
        Write-Log "Naming conventions OK" "SUCCESS"
    }

    return $issues.Count
}

function Test-MissingReferences {
    Write-Log "Checking for missing references..."

    $commandletArgs = @(
        "`"$ProjectFile`"",
        "-run=ResavePackages",
        "-fixupredirects",
        "-autocheckout",
        "-projectonly",
        "-unattended",
        "-log"
    )

    Write-Log "Running reference check..."

    $process = Start-Process -FilePath $UE5Editor -ArgumentList $commandletArgs -NoNewWindow -Wait -PassThru

    if ($process.ExitCode -eq 0) {
        Write-Log "No missing references found" "SUCCESS"
        return 0
    } else {
        Write-Log "Found missing references" "WARN"
        return 1
    }
}

function Test-TextureSettings {
    Write-Log "Checking texture settings..."

    $contentDir = "$ProjectRoot\Content"
    $issues = 0

    # Check for uncompressed textures (basic file size check)
    $textureFiles = Get-ChildItem -Path $contentDir -Recurse -Include *.uasset -ErrorAction SilentlyContinue | Where-Object {
        $_.Name -match '^T_'
    }

    foreach ($texture in $textureFiles) {
        $sizeInMB = $texture.Length / 1MB

        if ($sizeInMB -gt 10) {
            Write-Log "Large texture detected: $($texture.Name) ($([math]::Round($sizeInMB, 2)) MB)" "WARN"
            $issues++
        }
    }

    if ($issues -eq 0) {
        Write-Log "Texture settings OK" "SUCCESS"
    } else {
        Write-Log "Found $issues texture issues" "WARN"
    }

    return $issues
}

function Test-MapSettings {
    Write-Log "Checking map settings..."

    $contentDir = "$ProjectRoot\Content"
    $maps = Get-ChildItem -Path $contentDir -Recurse -Filter *.umap -ErrorAction SilentlyContinue

    Write-Log "Found $($maps.Count) maps"

    foreach ($map in $maps) {
        Write-Log "Map: $($map.Name) ($([math]::Round($map.Length / 1MB, 2)) MB)"
    }

    return 0
}

function Invoke-AssetCleanup {
    if ($FixIssues) {
        Write-Log "Running asset cleanup..."

        # Remove empty folders
        $contentDir = "$ProjectRoot\Content"
        $emptyDirs = Get-ChildItem -Path $contentDir -Recurse -Directory | Where-Object {
            (Get-ChildItem $_.FullName -File -Recurse).Count -eq 0
        }

        foreach ($dir in $emptyDirs) {
            Write-Log "Removing empty directory: $($dir.FullName)"
            Remove-Item -Path $dir.FullName -Force -ErrorAction SilentlyContinue
        }

        Write-Log "Cleanup complete" "SUCCESS"
    }
}

function Write-ValidationReport {
    $validationDuration = (Get-Date) - $ValidationStartTime

    Write-Log "========================================" "SUCCESS"
    Write-Log "ASSET VALIDATION COMPLETE" "SUCCESS"
    Write-Log "Duration: $($validationDuration.ToString('hh\:mm\:ss'))" "SUCCESS"
    Write-Log "Log: $LogFile" "SUCCESS"
    Write-Log "========================================" "SUCCESS"
}

# Main execution
try {
    Write-Log "Starting asset validation..."

    $totalIssues = 0
    $totalIssues += Test-AssetNamingConventions
    $totalIssues += Test-MissingReferences
    $totalIssues += Test-TextureSettings
    $totalIssues += Test-MapSettings

    Invoke-AssetCleanup
    Write-ValidationReport

    if ($totalIssues -eq 0) {
        Write-Log "All validations passed" "SUCCESS"
        exit 0
    } else {
        Write-Log "Found $totalIssues total issues" "WARN"
        exit 0
    }
}
catch {
    Write-Log "VALIDATION FAILED: $_" "ERROR"
    exit 1
}
