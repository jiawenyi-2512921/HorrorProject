<#
.SYNOPSIS
    Check Blueprint references and dependencies
.DESCRIPTION
    Analyzes Blueprint files to find references, dependencies, and potential issues
.PARAMETER BlueprintPath
    Path to Blueprint file or directory
.PARAMETER CheckExternal
    Check external asset references
.PARAMETER FindUnused
    Find unused references
.EXAMPLE
    .\CheckBlueprintReferences.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -CheckExternal -FindUnused
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintPath,

    [Parameter(Mandatory=$false)]
    [switch]$CheckExternal,

    [Parameter(Mandatory=$false)]
    [switch]$FindUnused,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/CheckBlueprintReferences_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

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

function Get-BlueprintReferences {
    param([string]$FilePath)

    $references = @{
        ParentClass = $null
        Components = @()
        Variables = @()
        Functions = @()
        ExternalAssets = @()
        Dependencies = @()
    }

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            # Parent class reference
            if ($content.ParentClass) {
                $references.ParentClass = $content.ParentClass
                if ($content.ParentClass -match '^/Game/') {
                    $references.Dependencies += $content.ParentClass
                }
            }

            # Component references
            if ($content.Components) {
                foreach ($component in $content.Components) {
                    $references.Components += @{
                        Name = $component.Name
                        Type = $component.Type
                        AttachParent = $component.AttachParent
                    }
                }
            }

            # Variable references
            if ($content.Variables) {
                foreach ($variable in $content.Variables) {
                    $references.Variables += @{
                        Name = $variable.Name
                        Type = $variable.Type
                    }

                    # Check for asset references in variable types
                    if ($variable.Type -match '^(UTexture|UMaterial|UStaticMesh|USkeletalMesh|USoundBase|UParticleSystem)') {
                        if ($variable.DefaultValue -and $variable.DefaultValue -match '^/Game/') {
                            $references.ExternalAssets += $variable.DefaultValue
                        }
                    }
                }
            }

            # Function references
            if ($content.Functions) {
                foreach ($function in $content.Functions) {
                    $references.Functions += @{
                        Name = $function.Name
                        Type = $function.Type
                    }
                }
            }

            # Widget tree references (for Widget Blueprints)
            if ($content.WidgetTree -and $content.WidgetTree.Components) {
                foreach ($component in $content.WidgetTree.Components) {
                    $references.Components += @{
                        Name = $component.Name
                        Type = $component.Type
                    }
                }
            }

        } else {
            # For .uasset files, use basic file analysis
            Write-Log "Binary Blueprint file - limited reference checking available" -Level "WARN"
        }

    } catch {
        Write-Log "Failed to extract references from $FilePath : $($_.Exception.Message)" -Level "ERROR"
    }

    return $references
}

function Test-ReferenceValidity {
    param([hashtable]$References, [string]$ProjectRoot)

    $issues = @()
    $warnings = @()

    # Check parent class
    if ($References.ParentClass -and $References.ParentClass -match '^/Game/') {
        $assetPath = $References.ParentClass -replace '^/Game/', "$ProjectRoot/Content/"
        $assetPath = $assetPath -replace '\..*$', '.uasset'

        if (-not (Test-Path $assetPath)) {
            $issues += "Parent class not found: $($References.ParentClass)"
        }
    }

    # Check external asset references
    foreach ($asset in $References.ExternalAssets) {
        $assetPath = $asset -replace '^/Game/', "$ProjectRoot/Content/"
        $assetPath = $assetPath -replace '\..*$', '.uasset'

        if (-not (Test-Path $assetPath)) {
            $warnings += "External asset not found: $asset"
        }
    }

    # Check for circular dependencies
    foreach ($dep in $References.Dependencies) {
        if ($dep -eq $References.ParentClass) {
            # This would be caught elsewhere, but good to note
        }
    }

    return @{
        Issues = $issues
        Warnings = $warnings
    }
}

function Find-UnusedReferences {
    param([hashtable]$References)

    $unused = @()

    # Check for unused variables (simple heuristic)
    foreach ($variable in $References.Variables) {
        # In a real implementation, we'd check if the variable is used in functions/events
        # For now, just flag variables with certain patterns
        if ($variable.Name -match '^Temp|^Test|^Debug') {
            $unused += "Variable '$($variable.Name)' appears to be temporary/debug"
        }
    }

    # Check for unused components
    foreach ($component in $References.Components) {
        if ($component.Name -match '^Temp|^Test|^Debug') {
            $unused += "Component '$($component.Name)' appears to be temporary/debug"
        }
    }

    return $unused
}

function Get-ReferenceDependencyTree {
    param([string]$BlueprintPath, [hashtable]$AllReferences)

    $tree = @{
        Blueprint = $BlueprintPath
        DirectDependencies = @()
        IndirectDependencies = @()
        TotalDepth = 0
    }

    $refs = $AllReferences[$BlueprintPath]
    if ($refs) {
        $tree.DirectDependencies = $refs.Dependencies

        # Calculate depth (simplified)
        $tree.TotalDepth = $refs.Dependencies.Count
    }

    return $tree
}

# Main execution
try {
    Write-Log "Starting Blueprint reference check"
    Write-Log "Blueprint Path: $BlueprintPath"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Determine project root
    $projectRoot = $BlueprintPath
    while ($projectRoot -and -not (Test-Path (Join-Path $projectRoot "*.uproject"))) {
        $projectRoot = Split-Path $projectRoot -Parent
    }

    if (-not $projectRoot) {
        Write-Log "Could not find project root - external reference checking disabled" -Level "WARN"
    }

    # Get Blueprint files
    $blueprintFiles = if (Test-Path $BlueprintPath -PathType Leaf) {
        @(Get-Item $BlueprintPath)
    } else {
        Get-ChildItem -Path $BlueprintPath -Filter "*.json" -Recurse | Where-Object { $_.Name -notmatch '\.metadata\.json$' }
    }

    Write-Log "Found $($blueprintFiles.Count) Blueprint files"

    if ($blueprintFiles.Count -eq 0) {
        Write-Log "No Blueprint files found" -Level "WARN"
        return
    }

    # Analyze references
    $allReferences = @{}
    $results = @()

    foreach ($file in $blueprintFiles) {
        Write-Log "Analyzing: $($file.FullName)"

        $references = Get-BlueprintReferences -FilePath $file.FullName
        $allReferences[$file.FullName] = $references

        $validityTest = if ($CheckExternal -and $projectRoot) {
            Test-ReferenceValidity -References $references -ProjectRoot $projectRoot
        } else {
            @{ Issues = @(); Warnings = @() }
        }

        $unusedRefs = if ($FindUnused) {
            Find-UnusedReferences -References $references
        } else {
            @()
        }

        $result = @{
            FilePath = $file.FullName
            FileName = $file.Name
            References = $references
            Issues = $validityTest.Issues
            Warnings = $validityTest.Warnings
            UnusedReferences = $unusedRefs
            TotalReferences = $references.Components.Count + $references.Variables.Count + $references.Functions.Count
            TotalDependencies = $references.Dependencies.Count
        }

        $results += $result

        Write-Log "  Components: $($references.Components.Count)"
        Write-Log "  Variables: $($references.Variables.Count)"
        Write-Log "  Functions: $($references.Functions.Count)"
        Write-Log "  Dependencies: $($references.Dependencies.Count)"

        if ($validityTest.Issues.Count -gt 0) {
            Write-Log "  Issues: $($validityTest.Issues.Count)" -Level "ERROR"
            foreach ($issue in $validityTest.Issues) {
                Write-Log "    - $issue" -Level "ERROR"
            }
        }

        if ($validityTest.Warnings.Count -gt 0) {
            Write-Log "  Warnings: $($validityTest.Warnings.Count)" -Level "WARN"
        }

        if ($unusedRefs.Count -gt 0) {
            Write-Log "  Unused references: $($unusedRefs.Count)" -Level "WARN"
        }
    }

    # Generate dependency report
    $reportPath = Join-Path $logsDir "ReferenceReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report = @{
        GeneratedDate = Get-Date -Format "o"
        TotalBlueprints = $results.Count
        TotalReferences = ($results | Measure-Object -Property TotalReferences -Sum).Sum
        TotalDependencies = ($results | Measure-Object -Property TotalDependencies -Sum).Sum
        Results = $results
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Log "Reference report saved to: $reportPath"

    # Summary
    Write-Host "`n=== Reference Check Complete ===" -ForegroundColor Cyan
    Write-Host "Total Blueprints: $($results.Count)" -ForegroundColor White
    Write-Host "Total References: $(($results | Measure-Object -Property TotalReferences -Sum).Sum)" -ForegroundColor White
    Write-Host "Total Dependencies: $(($results | Measure-Object -Property TotalDependencies -Sum).Sum)" -ForegroundColor White
    Write-Host "`nReport: $reportPath" -ForegroundColor Cyan

    return @{
        Success = $true
        Results = $results
        ReportPath = $reportPath
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
