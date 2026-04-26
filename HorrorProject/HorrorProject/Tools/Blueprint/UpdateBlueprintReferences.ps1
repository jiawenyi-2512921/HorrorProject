<#
.SYNOPSIS
    Update Blueprint references after asset moves or renames
.DESCRIPTION
    Scans and updates Blueprint references to reflect moved or renamed assets
.PARAMETER OldPath
    Old asset path (UE format: /Game/...)
.PARAMETER NewPath
    New asset path (UE format: /Game/...)
.PARAMETER SearchPath
    Directory to search for Blueprints to update
.EXAMPLE
    .\UpdateBlueprintReferences.ps1 -OldPath "/Game/Blueprints/BP_OldName" -NewPath "/Game/Blueprints/BP_NewName" -SearchPath "D:/gptzuo/HorrorProject/HorrorProject/Content"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$OldPath,

    [Parameter(Mandatory=$true)]
    [string]$NewPath,

    [Parameter(Mandatory=$true)]
    [string]$SearchPath,

    [Parameter(Mandatory=$false)]
    [switch]$DryRun,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/UpdateBlueprintReferences_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

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

function Find-BlueprintsWithReference {
    param([string]$AssetPath, [string]$SearchDir)

    $blueprintsWithRef = @()

    try {
        $blueprints = Get-ChildItem -Path $SearchDir -Filter "*.json" -Recurse |
            Where-Object { $_.Name -notmatch '\.metadata\.json$' }

        foreach ($blueprint in $blueprints) {
            $content = Get-Content $blueprint.FullName -Raw

            if ($content -match [regex]::Escape($AssetPath)) {
                $blueprintsWithRef += @{
                    FilePath = $blueprint.FullName
                    FileName = $blueprint.Name
                    Content = $content
                }
            }
        }

    } catch {
        Write-Log "Error searching for references: $($_.Exception.Message)" -Level "ERROR"
    }

    return $blueprintsWithRef
}

function Update-BlueprintReference {
    param([string]$FilePath, [string]$OldRef, [string]$NewRef, [bool]$DryRun)

    try {
        $content = Get-Content $FilePath -Raw
        $originalContent = $content

        # Count occurrences
        $matches = [regex]::Matches($content, [regex]::Escape($OldRef))
        $occurrenceCount = $matches.Count

        if ($occurrenceCount -eq 0) {
            return @{
                Success = $false
                Updated = $false
                Occurrences = 0
                Message = "No references found"
            }
        }

        # Replace references
        $content = $content -replace [regex]::Escape($OldRef), $NewRef

        if (-not $DryRun) {
            # Create backup
            $backupPath = "$FilePath.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
            Copy-Item $FilePath $backupPath
            Write-Log "Created backup: $backupPath"

            # Save updated content
            $content | Out-File -FilePath $FilePath -Encoding UTF8
            Write-Log "Updated $occurrenceCount reference(s) in: $FilePath" -Level "SUCCESS"
        } else {
            Write-Log "[DRY RUN] Would update $occurrenceCount reference(s) in: $FilePath" -Level "WARN"
        }

        return @{
            Success = $true
            Updated = $true
            Occurrences = $occurrenceCount
            Message = "Updated $occurrenceCount reference(s)"
        }

    } catch {
        Write-Log "Failed to update references in $FilePath : $($_.Exception.Message)" -Level "ERROR"
        return @{
            Success = $false
            Updated = $false
            Occurrences = 0
            Message = "Error: $($_.Exception.Message)"
        }
    }
}

function Get-ReferenceTypes {
    param([string]$Content, [string]$AssetPath)

    $types = @()

    # Check different reference contexts
    if ($Content -match "`"ParentClass`":\s*`"$([regex]::Escape($AssetPath))`"") {
        $types += "ParentClass"
    }

    if ($Content -match "`"Type`":\s*`"$([regex]::Escape($AssetPath))`"") {
        $types += "VariableType"
    }

    if ($Content -match "`"DefaultValue`":\s*`"$([regex]::Escape($AssetPath))`"") {
        $types += "DefaultValue"
    }

    if ($Content -match "`"AttachParent`":\s*`"$([regex]::Escape($AssetPath))`"") {
        $types += "ComponentParent"
    }

    if ($types.Count -eq 0) {
        $types += "Unknown"
    }

    return $types
}

# Main execution
try {
    Write-Log "Starting Blueprint reference update"
    Write-Log "Old Path: $OldPath"
    Write-Log "New Path: $NewPath"
    Write-Log "Search Path: $SearchPath"
    Write-Log "Dry Run: $DryRun"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Validate search path
    if (-not (Test-Path $SearchPath)) {
        throw "Search path does not exist: $SearchPath"
    }

    # Find Blueprints with references
    Write-Log "Searching for Blueprints with references to: $OldPath"
    $blueprintsWithRef = Find-BlueprintsWithReference -AssetPath $OldPath -SearchDir $SearchPath

    Write-Log "Found $($blueprintsWithRef.Count) Blueprint(s) with references"

    if ($blueprintsWithRef.Count -eq 0) {
        Write-Log "No Blueprints found with references to: $OldPath" -Level "WARN"
        return @{
            Success = $true
            BlueprintsFound = 0
            BlueprintsUpdated = 0
            TotalOccurrences = 0
        }
    }

    # Analyze reference types
    Write-Log "`nAnalyzing reference types..."
    foreach ($bp in $blueprintsWithRef) {
        $refTypes = Get-ReferenceTypes -Content $bp.Content -AssetPath $OldPath
        Write-Log "  $($bp.FileName): $($refTypes -join ', ')"
    }

    # Update references
    Write-Log "`nUpdating references..."
    $results = @()
    $totalOccurrences = 0
    $updatedCount = 0

    foreach ($bp in $blueprintsWithRef) {
        Write-Log "Processing: $($bp.FileName)"

        $result = Update-BlueprintReference -FilePath $bp.FilePath -OldRef $OldPath -NewRef $NewPath -DryRun $DryRun

        $results += @{
            FilePath = $bp.FilePath
            FileName = $bp.FileName
            Success = $result.Success
            Updated = $result.Updated
            Occurrences = $result.Occurrences
            Message = $result.Message
        }

        if ($result.Updated) {
            $updatedCount++
            $totalOccurrences += $result.Occurrences
        }
    }

    # Generate report
    $reportPath = Join-Path $logsDir "ReferenceUpdateReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report = @{
        GeneratedDate = Get-Date -Format "o"
        OldPath = $OldPath
        NewPath = $NewPath
        SearchPath = $SearchPath
        DryRun = $DryRun.IsPresent
        BlueprintsFound = $blueprintsWithRef.Count
        BlueprintsUpdated = $updatedCount
        TotalOccurrences = $totalOccurrences
        Results = $results
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Log "Report saved to: $reportPath"

    # Summary
    Write-Host "`n=== Reference Update Complete ===" -ForegroundColor Cyan
    Write-Host "Blueprints Found: $($blueprintsWithRef.Count)" -ForegroundColor White
    Write-Host "Blueprints Updated: $updatedCount" -ForegroundColor $(if($updatedCount -gt 0){"Green"}else{"Yellow"})
    Write-Host "Total Occurrences: $totalOccurrences" -ForegroundColor White

    if ($DryRun) {
        Write-Host "`n[DRY RUN MODE - No files were actually modified]" -ForegroundColor Yellow
    }

    Write-Host "`nReport: $reportPath" -ForegroundColor Cyan

    return @{
        Success = $true
        BlueprintsFound = $blueprintsWithRef.Count
        BlueprintsUpdated = $updatedCount
        TotalOccurrences = $totalOccurrences
        ReportPath = $reportPath
        Results = $results
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
