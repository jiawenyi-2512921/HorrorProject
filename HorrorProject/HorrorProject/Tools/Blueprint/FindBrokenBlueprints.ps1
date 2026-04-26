<#
.SYNOPSIS
    Find broken or corrupted Blueprint files
.DESCRIPTION
    Scans for Blueprint files with corruption, missing data, or structural issues
.PARAMETER BlueprintPath
    Path to Blueprint file or directory
.PARAMETER AutoFix
    Attempt to automatically fix simple issues
.PARAMETER Quarantine
    Move broken Blueprints to quarantine folder
.EXAMPLE
    .\FindBrokenBlueprints.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -AutoFix -Quarantine
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintPath,

    [Parameter(Mandatory=$false)]
    [switch]$AutoFix,

    [Parameter(Mandatory=$false)]
    [switch]$Quarantine,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/FindBrokenBlueprints_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $logMessage
    if ($Verbose -or $Level -eq "ERROR" -or $Level -eq "WARN") {
        $color = switch ($Level) {
            "ERROR" { "Red" }
            "WARN" { "Yellow" }
            "SUCCESS" { "Green" }
            default { "White" }
        }
        Write-Host $logMessage -ForegroundColor $color
    }
}

function Test-BlueprintCorruption {
    param([string]$FilePath)

    $issues = @()
    $severity = "None"

    try {
        $fileInfo = Get-Item $FilePath

        # Check 1: Empty file
        if ($fileInfo.Length -eq 0) {
            $issues += "File is empty (0 bytes)"
            $severity = "Critical"
            return @{ Issues = $issues; Severity = $severity; CanFix = $false }
        }

        # Check 2: File size anomalies
        if ($FilePath -match '\.uasset$' -and $fileInfo.Length -lt 512) {
            $issues += "File size suspiciously small for .uasset (< 512 bytes)"
            $severity = "High"
        }

        # Check 3: JSON parsing (for template files)
        if ($FilePath -match '\.json$') {
            try {
                $content = Get-Content $FilePath -Raw | ConvertFrom-Json

                # Check for required Blueprint fields
                if (-not $content.Type) {
                    $issues += "Missing 'Type' field"
                    $severity = "High"
                }

                if (-not $content.Name) {
                    $issues += "Missing 'Name' field"
                    $severity = "High"
                }

                if (-not $content.GUID) {
                    $issues += "Missing 'GUID' field"
                    $severity = "Medium"
                }

                if (-not $content.ParentClass) {
                    $issues += "Missing 'ParentClass' field"
                    $severity = "High"
                }

                # Check for malformed GUID
                if ($content.GUID -and $content.GUID -notmatch '^[A-F0-9]{32}$') {
                    $issues += "Malformed GUID: $($content.GUID)"
                    $severity = "Medium"
                }

                # Check for circular inheritance
                if ($content.ParentClass -eq "/Game/$($content.Name)") {
                    $issues += "Circular inheritance detected"
                    $severity = "Critical"
                }

                # Check for null/undefined critical fields
                if ($content.ParentClass -eq $null -or $content.ParentClass -eq "") {
                    $issues += "ParentClass is null or empty"
                    $severity = "Critical"
                }

                # Check component integrity
                if ($content.Components) {
                    $componentNames = @{}
                    foreach ($component in $content.Components) {
                        if (-not $component.Name) {
                            $issues += "Component missing name"
                            $severity = "High"
                        } elseif ($componentNames.ContainsKey($component.Name)) {
                            $issues += "Duplicate component name: $($component.Name)"
                            $severity = "High"
                        } else {
                            $componentNames[$component.Name] = $true
                        }

                        if (-not $component.Type) {
                            $issues += "Component '$($component.Name)' missing type"
                            $severity = "High"
                        }
                    }
                }

                # Check variable integrity
                if ($content.Variables) {
                    $variableNames = @{}
                    foreach ($variable in $content.Variables) {
                        if (-not $variable.Name) {
                            $issues += "Variable missing name"
                            $severity = "Medium"
                        } elseif ($variableNames.ContainsKey($variable.Name)) {
                            $issues += "Duplicate variable name: $($variable.Name)"
                            $severity = "Medium"
                        } else {
                            $variableNames[$variable.Name] = $true
                        }

                        if (-not $variable.Type) {
                            $issues += "Variable '$($variable.Name)' missing type"
                            $severity = "Medium"
                        }
                    }
                }

            } catch {
                $issues += "JSON parsing failed: $($_.Exception.Message)"
                $severity = "Critical"
            }
        }

        # Check 4: File accessibility
        try {
            $stream = [System.IO.File]::OpenRead($FilePath)
            $stream.Close()
        } catch {
            $issues += "File cannot be opened: $($_.Exception.Message)"
            $severity = "Critical"
        }

        # Check 5: File name validity
        if ($fileInfo.Name -match '[<>:"|?*]') {
            $issues += "File name contains invalid characters"
            $severity = "High"
        }

    } catch {
        $issues += "Unexpected error during corruption check: $($_.Exception.Message)"
        $severity = "Critical"
    }

    $canFix = ($severity -ne "Critical" -and $issues.Count -gt 0)

    return @{
        Issues = $issues
        Severity = $severity
        CanFix = $canFix
    }
}

function Repair-Blueprint {
    param([string]$FilePath, [array]$Issues)

    $fixed = @()
    $failed = @()

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json
            $modified = $false

            # Fix missing GUID
            if (-not $content.GUID -or $content.GUID -notmatch '^[A-F0-9]{32}$') {
                $content.GUID = [guid]::NewGuid().ToString("N").ToUpper()
                $fixed += "Generated new GUID"
                $modified = $true
            }

            # Fix missing metadata
            if (-not $content.Metadata) {
                $content.Metadata = @{
                    Category = "Repaired"
                    Description = "Auto-repaired Blueprint"
                    Keywords = "Repaired"
                }
                $fixed += "Added missing metadata"
                $modified = $true
            }

            # Fix missing GeneratedDate
            if (-not $content.GeneratedDate) {
                $content.GeneratedDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
                $fixed += "Added GeneratedDate"
                $modified = $true
            }

            # Fix empty arrays
            if ($content.Components -eq $null) {
                $content.Components = @()
                $fixed += "Initialized Components array"
                $modified = $true
            }

            if ($content.Variables -eq $null) {
                $content.Variables = @()
                $fixed += "Initialized Variables array"
                $modified = $true
            }

            if ($content.Functions -eq $null) {
                $content.Functions = @()
                $fixed += "Initialized Functions array"
                $modified = $true
            }

            # Save if modified
            if ($modified) {
                $backupPath = "$FilePath.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
                Copy-Item $FilePath $backupPath
                Write-Log "Created backup: $backupPath"

                $content | ConvertTo-Json -Depth 10 | Out-File -FilePath $FilePath -Encoding UTF8
                Write-Log "Repaired Blueprint: $FilePath" -Level "SUCCESS"
            }
        }

    } catch {
        $failed += "Repair failed: $($_.Exception.Message)"
    }

    return @{
        Fixed = $fixed
        Failed = $failed
    }
}

function Move-ToQuarantine {
    param([string]$FilePath, [string]$QuarantinePath)

    try {
        if (-not (Test-Path $QuarantinePath)) {
            New-Item -ItemType Directory -Path $QuarantinePath -Force | Out-Null
        }

        $fileName = Split-Path $FilePath -Leaf
        $destination = Join-Path $QuarantinePath $fileName

        # Handle duplicate names
        $counter = 1
        while (Test-Path $destination) {
            $baseName = [System.IO.Path]::GetFileNameWithoutExtension($fileName)
            $extension = [System.IO.Path]::GetExtension($fileName)
            $destination = Join-Path $QuarantinePath "$baseName`_$counter$extension"
            $counter++
        }

        Move-Item $FilePath $destination -Force
        Write-Log "Moved to quarantine: $destination" -Level "WARN"

        return $destination

    } catch {
        Write-Log "Failed to quarantine file: $($_.Exception.Message)" -Level "ERROR"
        return $null
    }
}

# Main execution
try {
    Write-Log "Starting broken Blueprint scan"
    Write-Log "Blueprint Path: $BlueprintPath"
    Write-Log "Auto-fix: $AutoFix"
    Write-Log "Quarantine: $Quarantine"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Create quarantine directory
    $quarantinePath = Join-Path $logsDir "Quarantine"

    # Get Blueprint files
    $blueprintFiles = if (Test-Path $BlueprintPath -PathType Leaf) {
        @(Get-Item $BlueprintPath)
    } else {
        Get-ChildItem -Path $BlueprintPath -Include "*.uasset","*.json" -Recurse |
            Where-Object { $_.Name -notmatch '\.metadata\.json$|\.backup_' }
    }

    Write-Log "Found $($blueprintFiles.Count) Blueprint files to scan"

    if ($blueprintFiles.Count -eq 0) {
        Write-Log "No Blueprint files found" -Level "WARN"
        return
    }

    # Scan for broken Blueprints
    $results = @()
    $brokenCount = 0
    $fixedCount = 0
    $quarantinedCount = 0

    foreach ($file in $blueprintFiles) {
        Write-Log "Scanning: $($file.FullName)"

        $corruptionTest = Test-BlueprintCorruption -FilePath $file.FullName

        if ($corruptionTest.Issues.Count -gt 0) {
            $brokenCount++

            Write-Log "  BROKEN - Severity: $($corruptionTest.Severity)" -Level "ERROR"
            foreach ($issue in $corruptionTest.Issues) {
                Write-Log "    - $issue" -Level "ERROR"
            }

            $repairResult = $null
            if ($AutoFix -and $corruptionTest.CanFix) {
                Write-Log "  Attempting auto-fix..." -Level "WARN"
                $repairResult = Repair-Blueprint -FilePath $file.FullName -Issues $corruptionTest.Issues

                if ($repairResult.Fixed.Count -gt 0) {
                    $fixedCount++
                    Write-Log "  Fixed $($repairResult.Fixed.Count) issues" -Level "SUCCESS"
                    foreach ($fix in $repairResult.Fixed) {
                        Write-Log "    + $fix" -Level "SUCCESS"
                    }
                }

                if ($repairResult.Failed.Count -gt 0) {
                    foreach ($failure in $repairResult.Failed) {
                        Write-Log "    ! $failure" -Level "ERROR"
                    }
                }
            }

            $quarantinedPath = $null
            if ($Quarantine -and ($corruptionTest.Severity -eq "Critical" -or ($AutoFix -and $repairResult -and $repairResult.Failed.Count -gt 0))) {
                $quarantinedPath = Move-ToQuarantine -FilePath $file.FullName -QuarantinePath $quarantinePath
                if ($quarantinedPath) {
                    $quarantinedCount++
                }
            }

            $results += @{
                FilePath = $file.FullName
                FileName = $file.Name
                Issues = $corruptionTest.Issues
                Severity = $corruptionTest.Severity
                CanFix = $corruptionTest.CanFix
                RepairResult = $repairResult
                QuarantinedPath = $quarantinedPath
            }
        }
    }

    # Generate report
    $reportPath = Join-Path $logsDir "BrokenBlueprintsReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report = @{
        GeneratedDate = Get-Date -Format "o"
        TotalScanned = $blueprintFiles.Count
        BrokenFound = $brokenCount
        Fixed = $fixedCount
        Quarantined = $quarantinedCount
        Results = $results
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Log "Report saved to: $reportPath"

    # Summary
    Write-Host "`n=== Broken Blueprint Scan Complete ===" -ForegroundColor Cyan
    Write-Host "Total Scanned: $($blueprintFiles.Count)" -ForegroundColor White
    Write-Host "Broken Found: $brokenCount" -ForegroundColor $(if($brokenCount -eq 0){"Green"}else{"Red"})
    Write-Host "Fixed: $fixedCount" -ForegroundColor $(if($fixedCount -gt 0){"Green"}else{"White"})
    Write-Host "Quarantined: $quarantinedCount" -ForegroundColor $(if($quarantinedCount -gt 0){"Yellow"}else{"White"})
    Write-Host "`nReport: $reportPath" -ForegroundColor Cyan

    if ($quarantinedCount -gt 0) {
        Write-Host "Quarantine folder: $quarantinePath" -ForegroundColor Yellow
    }

    return @{
        Success = $true
        TotalScanned = $blueprintFiles.Count
        BrokenFound = $brokenCount
        Fixed = $fixedCount
        Quarantined = $quarantinedCount
        ReportPath = $reportPath
        Results = $results
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
