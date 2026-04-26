<#
.SYNOPSIS
    Migrate Blueprint assets between directories or projects
.DESCRIPTION
    Safely migrates Blueprint files with dependency tracking and reference updating
.PARAMETER SourcePath
    Source Blueprint path
.PARAMETER DestinationPath
    Destination path
.PARAMETER IncludeDependencies
    Migrate dependencies automatically
.PARAMETER UpdateReferences
    Update references in other Blueprints
.EXAMPLE
    .\MigrateBlueprintAssets.ps1 -SourcePath "D:/Old/BP_Actor.uasset" -DestinationPath "D:/New/" -IncludeDependencies -UpdateReferences
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath,

    [Parameter(Mandatory=$true)]
    [string]$DestinationPath,

    [Parameter(Mandatory=$false)]
    [switch]$IncludeDependencies,

    [Parameter(Mandatory=$false)]
    [switch]$UpdateReferences,

    [Parameter(Mandatory=$false)]
    [switch]$DryRun,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/MigrateBlueprintAssets_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

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

function Get-BlueprintDependencies {
    param([string]$FilePath)

    $dependencies = @()

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            # Parent class dependency
            if ($content.ParentClass -and $content.ParentClass -match '^/Game/') {
                $dependencies += $content.ParentClass
            }

            # Variable type dependencies
            if ($content.Variables) {
                foreach ($variable in $content.Variables) {
                    if ($variable.Type -match '^/Game/') {
                        $dependencies += $variable.Type
                    }
                    if ($variable.DefaultValue -and $variable.DefaultValue -match '^/Game/') {
                        $dependencies += $variable.DefaultValue
                    }
                }
            }

            # Component dependencies
            if ($content.Components) {
                foreach ($component in $content.Components) {
                    if ($component.Type -match '^/Game/') {
                        $dependencies += $component.Type
                    }
                }
            }
        }

    } catch {
        Write-Log "Failed to extract dependencies from $FilePath : $($_.Exception.Message)" -Level "ERROR"
    }

    return $dependencies | Select-Object -Unique
}

function Copy-BlueprintWithMetadata {
    param([string]$Source, [string]$Destination)

    try {
        # Create destination directory
        $destDir = Split-Path $Destination -Parent
        if (-not (Test-Path $destDir)) {
            New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            Write-Log "Created directory: $destDir"
        }

        # Copy main file
        Copy-Item $Source $Destination -Force
        Write-Log "Copied: $Source -> $Destination" -Level "SUCCESS"

        # Copy metadata file if exists
        $metadataSource = "$Source.metadata.json"
        if (Test-Path $metadataSource) {
            $metadataDest = "$Destination.metadata.json"
            Copy-Item $metadataSource $metadataDest -Force
            Write-Log "Copied metadata: $metadataDest"
        }

        # Copy .uexp file if exists (for .uasset files)
        if ($Source -match '\.uasset$') {
            $uexpSource = $Source -replace '\.uasset$', '.uexp'
            if (Test-Path $uexpSource) {
                $uexpDest = $Destination -replace '\.uasset$', '.uexp'
                Copy-Item $uexpSource $uexpDest -Force
                Write-Log "Copied .uexp: $uexpDest"
            }
        }

        return $true

    } catch {
        Write-Log "Failed to copy Blueprint: $($_.Exception.Message)" -Level "ERROR"
        return $false
    }
}

function Update-BlueprintPath {
    param([string]$OldPath, [string]$NewPath, [string]$ContentRoot)

    # Convert file system paths to UE asset paths
    $oldAssetPath = $OldPath -replace [regex]::Escape($ContentRoot), '/Game'
    $oldAssetPath = $oldAssetPath -replace '\\', '/'
    $oldAssetPath = $oldAssetPath -replace '\.(uasset|json)$', ''

    $newAssetPath = $NewPath -replace [regex]::Escape($ContentRoot), '/Game'
    $newAssetPath = $newAssetPath -replace '\\', '/'
    $newAssetPath = $newAssetPath -replace '\.(uasset|json)$', ''

    return @{
        OldPath = $oldAssetPath
        NewPath = $newAssetPath
    }
}

function Update-ReferencesInBlueprints {
    param([string]$OldAssetPath, [string]$NewAssetPath, [string]$SearchPath)

    $updatedFiles = @()

    try {
        # Find all Blueprint JSON files
        $blueprints = Get-ChildItem -Path $SearchPath -Filter "*.json" -Recurse |
            Where-Object { $_.Name -notmatch '\.metadata\.json$' }

        foreach ($blueprint in $blueprints) {
            $modified = $false
            $content = Get-Content $blueprint.FullName -Raw

            # Check if this Blueprint references the old path
            if ($content -match [regex]::Escape($OldAssetPath)) {
                Write-Log "Updating references in: $($blueprint.Name)"

                # Replace old path with new path
                $content = $content -replace [regex]::Escape($OldAssetPath), $NewAssetPath

                # Save updated content
                $content | Out-File -FilePath $blueprint.FullName -Encoding UTF8
                $updatedFiles += $blueprint.FullName
                $modified = $true

                Write-Log "  Updated references in $($blueprint.Name)" -Level "SUCCESS"
            }
        }

    } catch {
        Write-Log "Failed to update references: $($_.Exception.Message)" -Level "ERROR"
    }

    return $updatedFiles
}

function New-MigrationReport {
    param([hashtable]$MigrationData, [string]$OutputPath)

    $reportPath = Join-Path $OutputPath "MigrationReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"

    $report = @{
        GeneratedDate = Get-Date -Format "o"
        SourcePath = $MigrationData.SourcePath
        DestinationPath = $MigrationData.DestinationPath
        MigratedFiles = $MigrationData.MigratedFiles
        Dependencies = $MigrationData.Dependencies
        UpdatedReferences = $MigrationData.UpdatedReferences
        Success = $MigrationData.Success
        Errors = $MigrationData.Errors
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Log "Migration report saved to: $reportPath"

    return $reportPath
}

# Main execution
try {
    Write-Log "Starting Blueprint migration"
    Write-Log "Source: $SourcePath"
    Write-Log "Destination: $DestinationPath"
    Write-Log "Dry Run: $DryRun"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Validate source
    if (-not (Test-Path $SourcePath)) {
        throw "Source path does not exist: $SourcePath"
    }

    # Get files to migrate
    $filesToMigrate = if (Test-Path $SourcePath -PathType Leaf) {
        @(Get-Item $SourcePath)
    } else {
        Get-ChildItem -Path $SourcePath -Include "*.uasset","*.json" -Recurse |
            Where-Object { $_.Name -notmatch '\.metadata\.json$' }
    }

    Write-Log "Found $($filesToMigrate.Count) files to migrate"

    if ($filesToMigrate.Count -eq 0) {
        Write-Log "No files to migrate" -Level "WARN"
        return
    }

    # Track migration
    $migratedFiles = @()
    $allDependencies = @()
    $updatedReferences = @()
    $errors = @()

    # Find content root
    $contentRoot = $SourcePath
    while ($contentRoot -and -not (Test-Path (Join-Path $contentRoot "Content"))) {
        $parent = Split-Path $contentRoot -Parent
        if ($parent -eq $contentRoot) { break }
        $contentRoot = $parent
    }
    if ($contentRoot) {
        $contentRoot = Join-Path $contentRoot "Content"
    }

    # Migrate each file
    foreach ($file in $filesToMigrate) {
        Write-Log "Processing: $($file.Name)"

        # Get dependencies
        if ($IncludeDependencies) {
            $deps = Get-BlueprintDependencies -FilePath $file.FullName
            if ($deps.Count -gt 0) {
                Write-Log "  Found $($deps.Count) dependencies"
                $allDependencies += $deps
            }
        }

        # Determine destination file path
        $relativePath = if (Test-Path $SourcePath -PathType Leaf) {
            $file.Name
        } else {
            $file.FullName.Substring($SourcePath.Length).TrimStart('\', '/')
        }

        $destFile = Join-Path $DestinationPath $relativePath

        # Perform migration
        if (-not $DryRun) {
            $success = Copy-BlueprintWithMetadata -Source $file.FullName -Destination $destFile
            if ($success) {
                $migratedFiles += @{
                    Source = $file.FullName
                    Destination = $destFile
                }

                # Update references if requested
                if ($UpdateReferences -and $contentRoot) {
                    $paths = Update-BlueprintPath -OldPath $file.FullName -NewPath $destFile -ContentRoot $contentRoot
                    $updated = Update-ReferencesInBlueprints -OldAssetPath $paths.OldPath -NewAssetPath $paths.NewPath -SearchPath $contentRoot
                    $updatedReferences += $updated
                }
            } else {
                $errors += "Failed to migrate: $($file.FullName)"
            }
        } else {
            Write-Log "  [DRY RUN] Would copy to: $destFile" -Level "WARN"
            $migratedFiles += @{
                Source = $file.FullName
                Destination = $destFile
            }
        }
    }

    # Generate report
    $migrationData = @{
        SourcePath = $SourcePath
        DestinationPath = $DestinationPath
        MigratedFiles = $migratedFiles
        Dependencies = $allDependencies | Select-Object -Unique
        UpdatedReferences = $updatedReferences
        Success = ($errors.Count -eq 0)
        Errors = $errors
    }

    $reportPath = New-MigrationReport -MigrationData $migrationData -OutputPath $logsDir

    # Summary
    Write-Host "`n=== Migration Complete ===" -ForegroundColor Cyan
    Write-Host "Files Migrated: $($migratedFiles.Count)" -ForegroundColor $(if($migratedFiles.Count -gt 0){"Green"}else{"Yellow"})
    Write-Host "Dependencies Found: $($allDependencies.Count)" -ForegroundColor White
    Write-Host "References Updated: $($updatedReferences.Count)" -ForegroundColor White
    Write-Host "Errors: $($errors.Count)" -ForegroundColor $(if($errors.Count -eq 0){"Green"}else{"Red"})

    if ($DryRun) {
        Write-Host "`n[DRY RUN MODE - No files were actually migrated]" -ForegroundColor Yellow
    }

    Write-Host "`nReport: $reportPath" -ForegroundColor Cyan

    return @{
        Success = ($errors.Count -eq 0)
        MigratedFiles = $migratedFiles.Count
        Dependencies = $allDependencies.Count
        UpdatedReferences = $updatedReferences.Count
        Errors = $errors.Count
        ReportPath = $reportPath
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
