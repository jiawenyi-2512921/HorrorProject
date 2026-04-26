<#
.SYNOPSIS
    Validate Blueprint integrity and completeness
.DESCRIPTION
    Checks Blueprint files for common issues, missing references, and structural problems
.PARAMETER BlueprintPath
    Path to Blueprint file or directory
.PARAMETER Recursive
    Search recursively for Blueprints
.PARAMETER OutputReport
    Generate detailed validation report
.EXAMPLE
    .\ValidateBlueprints.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -Recursive -OutputReport
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintPath,

    [Parameter(Mandatory=$false)]
    [switch]$Recursive,

    [Parameter(Mandatory=$false)]
    [switch]$OutputReport,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/ValidateBlueprints_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

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

function Get-BlueprintFiles {
    param([string]$Path, [bool]$Recurse)

    if (Test-Path $Path -PathType Leaf) {
        return @($Path)
    }

    $searchParams = @{
        Path = $Path
        Filter = "*.uasset"
    }

    if ($Recurse) {
        $searchParams.Recurse = $true
    }

    $files = Get-ChildItem @searchParams -ErrorAction SilentlyContinue

    # Also check for JSON templates
    $jsonFiles = Get-ChildItem -Path $Path -Filter "*.json" -Recurse:$Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -notmatch '\.metadata\.json$' }

    return ($files + $jsonFiles)
}

function Test-BlueprintStructure {
    param([string]$FilePath)

    $issues = @()
    $warnings = @()

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            # Check required fields
            $requiredFields = @("Type", "Name", "ParentClass", "GUID")
            foreach ($field in $requiredFields) {
                if (-not $content.$field) {
                    $issues += "Missing required field: $field"
                }
            }

            # Check GUID format
            if ($content.GUID -and $content.GUID -notmatch '^[A-F0-9]{32}$') {
                $warnings += "Invalid GUID format"
            }

            # Check naming conventions
            $name = $content.Name
            if ($content.Type -eq "WidgetBlueprint" -and $name -notmatch '^(WBP_|W_)') {
                $warnings += "Widget Blueprint should start with WBP_ or W_"
            }
            if ($content.Type -eq "ActorBlueprint" -and $name -notmatch '^BP_') {
                $warnings += "Actor Blueprint should start with BP_"
            }

            # Check for empty components
            if ($content.Components -and $content.Components.Count -eq 0) {
                $warnings += "Blueprint has no components"
            }

            # Check for circular dependencies
            if ($content.ParentClass -and $content.ParentClass -eq $content.Name) {
                $issues += "Circular dependency: Blueprint inherits from itself"
            }

        } else {
            # For .uasset files, check basic file properties
            $fileInfo = Get-Item $FilePath
            if ($fileInfo.Length -eq 0) {
                $issues += "Blueprint file is empty"
            }
            if ($fileInfo.Length -lt 1KB) {
                $warnings += "Blueprint file is suspiciously small (< 1KB)"
            }
        }

    } catch {
        $issues += "Failed to parse Blueprint: $($_.Exception.Message)"
    }

    return @{
        Issues = $issues
        Warnings = $warnings
    }
}

function Test-BlueprintReferences {
    param([string]$FilePath)

    $issues = @()
    $warnings = @()

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            # Check parent class reference
            if ($content.ParentClass) {
                $parentPath = $content.ParentClass
                if ($parentPath -notmatch '^/Script/' -and $parentPath -notmatch '^/Game/') {
                    $warnings += "Unusual parent class path: $parentPath"
                }
            }

            # Check component references
            if ($content.Components) {
                foreach ($component in $content.Components) {
                    if ($component.AttachParent -and $component.AttachParent -ne $null) {
                        $parentExists = $content.Components | Where-Object { $_.Name -eq $component.AttachParent }
                        if (-not $parentExists) {
                            $issues += "Component '$($component.Name)' references non-existent parent '$($component.AttachParent)'"
                        }
                    }
                }
            }

            # Check function references
            if ($content.Functions) {
                foreach ($function in $content.Functions) {
                    if (-not $function.Name) {
                        $issues += "Function missing name"
                    }
                }
            }

            # Check variable types
            if ($content.Variables) {
                foreach ($variable in $content.Variables) {
                    if (-not $variable.Type) {
                        $issues += "Variable '$($variable.Name)' missing type"
                    }
                }
            }
        }

    } catch {
        $issues += "Failed to validate references: $($_.Exception.Message)"
    }

    return @{
        Issues = $issues
        Warnings = $warnings
    }
}

function Test-BlueprintComplexity {
    param([string]$FilePath)

    $metrics = @{
        ComponentCount = 0
        VariableCount = 0
        FunctionCount = 0
        NodeCount = 0
        ComplexityScore = 0
    }

    $warnings = @()

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            $metrics.ComponentCount = if ($content.Components) { $content.Components.Count } else { 0 }
            $metrics.VariableCount = if ($content.Variables) { $content.Variables.Count } else { 0 }
            $metrics.FunctionCount = if ($content.Functions) { $content.Functions.Count } else { 0 }

            if ($content.EventGraph -and $content.EventGraph.Nodes) {
                $metrics.NodeCount = $content.EventGraph.Nodes.Count
            }

            # Calculate complexity score
            $metrics.ComplexityScore = ($metrics.ComponentCount * 2) +
                                      ($metrics.VariableCount * 1) +
                                      ($metrics.FunctionCount * 3) +
                                      ($metrics.NodeCount * 0.5)

            # Complexity warnings
            if ($metrics.ComponentCount -gt 20) {
                $warnings += "High component count ($($metrics.ComponentCount)) - consider refactoring"
            }
            if ($metrics.VariableCount -gt 50) {
                $warnings += "High variable count ($($metrics.VariableCount)) - consider using structs"
            }
            if ($metrics.FunctionCount -gt 30) {
                $warnings += "High function count ($($metrics.FunctionCount)) - consider splitting into components"
            }
            if ($metrics.ComplexityScore -gt 100) {
                $warnings += "High complexity score ($($metrics.ComplexityScore)) - Blueprint may be difficult to maintain"
            }
        }

    } catch {
        $warnings += "Failed to calculate complexity: $($_.Exception.Message)"
    }

    return @{
        Metrics = $metrics
        Warnings = $warnings
    }
}

function New-ValidationReport {
    param([array]$Results, [string]$OutputPath)

    $reportPath = Join-Path $OutputPath "ValidationReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"

    $summary = @{
        TotalBlueprints = $Results.Count
        ValidBlueprints = ($Results | Where-Object { $_.TotalIssues -eq 0 }).Count
        BlueprintsWithIssues = ($Results | Where-Object { $_.TotalIssues -gt 0 }).Count
        BlueprintsWithWarnings = ($Results | Where-Object { $_.TotalWarnings -gt 0 }).Count
        TotalIssues = ($Results | Measure-Object -Property TotalIssues -Sum).Sum
        TotalWarnings = ($Results | Measure-Object -Property TotalWarnings -Sum).Sum
        GeneratedDate = Get-Date -Format "o"
    }

    $report = @{
        Summary = $summary
        Results = $Results
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Log "Validation report saved to: $reportPath"

    return $reportPath
}

# Main execution
try {
    Write-Log "Starting Blueprint validation"
    Write-Log "Blueprint Path: $BlueprintPath"
    Write-Log "Recursive: $Recursive"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Get Blueprint files
    $blueprintFiles = Get-BlueprintFiles -Path $BlueprintPath -Recurse $Recursive
    Write-Log "Found $($blueprintFiles.Count) Blueprint files"

    if ($blueprintFiles.Count -eq 0) {
        Write-Log "No Blueprint files found" -Level "WARN"
        return
    }

    # Validate each Blueprint
    $results = @()
    $progressCount = 0

    foreach ($file in $blueprintFiles) {
        $progressCount++
        Write-Progress -Activity "Validating Blueprints" -Status "Processing $($file.Name)" -PercentComplete (($progressCount / $blueprintFiles.Count) * 100)

        Write-Log "Validating: $($file.FullName)"

        $structureTest = Test-BlueprintStructure -FilePath $file.FullName
        $referenceTest = Test-BlueprintReferences -FilePath $file.FullName
        $complexityTest = Test-BlueprintComplexity -FilePath $file.FullName

        $allIssues = $structureTest.Issues + $referenceTest.Issues
        $allWarnings = $structureTest.Warnings + $referenceTest.Warnings + $complexityTest.Warnings

        $result = @{
            FilePath = $file.FullName
            FileName = $file.Name
            TotalIssues = $allIssues.Count
            TotalWarnings = $allWarnings.Count
            Issues = $allIssues
            Warnings = $allWarnings
            Metrics = $complexityTest.Metrics
            Status = if ($allIssues.Count -eq 0) { "Valid" } else { "Invalid" }
        }

        $results += $result

        if ($allIssues.Count -gt 0) {
            Write-Log "  Issues found: $($allIssues.Count)" -Level "ERROR"
            foreach ($issue in $allIssues) {
                Write-Log "    - $issue" -Level "ERROR"
            }
        }

        if ($allWarnings.Count -gt 0) {
            Write-Log "  Warnings: $($allWarnings.Count)" -Level "WARN"
            if ($Verbose) {
                foreach ($warning in $allWarnings) {
                    Write-Log "    - $warning" -Level "WARN"
                }
            }
        }

        if ($allIssues.Count -eq 0 -and $allWarnings.Count -eq 0) {
            Write-Log "  Status: Valid" -Level "SUCCESS"
        }
    }

    Write-Progress -Activity "Validating Blueprints" -Completed

    # Generate report
    $reportPath = $null
    if ($OutputReport) {
        $reportPath = New-ValidationReport -Results $results -OutputPath $logsDir
    }

    # Summary
    $validCount = ($results | Where-Object { $_.Status -eq "Valid" }).Count
    $invalidCount = ($results | Where-Object { $_.Status -eq "Invalid" }).Count
    $totalIssues = ($results | Measure-Object -Property TotalIssues -Sum).Sum
    $totalWarnings = ($results | Measure-Object -Property TotalWarnings -Sum).Sum

    Write-Log "`nValidation Summary:" -Level "INFO"
    Write-Log "  Total Blueprints: $($results.Count)" -Level "INFO"
    Write-Log "  Valid: $validCount" -Level "SUCCESS"
    Write-Log "  Invalid: $invalidCount" -Level $(if($invalidCount -gt 0){"ERROR"}else{"INFO"})
    Write-Log "  Total Issues: $totalIssues" -Level $(if($totalIssues -gt 0){"ERROR"}else{"INFO"})
    Write-Log "  Total Warnings: $totalWarnings" -Level $(if($totalWarnings -gt 0){"WARN"}else{"INFO"})

    Write-Host "`n=== Validation Complete ===" -ForegroundColor Cyan
    Write-Host "Valid Blueprints: $validCount / $($results.Count)" -ForegroundColor $(if($validCount -eq $results.Count){"Green"}else{"Yellow"})
    Write-Host "Total Issues: $totalIssues" -ForegroundColor $(if($totalIssues -eq 0){"Green"}else{"Red"})
    Write-Host "Total Warnings: $totalWarnings" -ForegroundColor $(if($totalWarnings -eq 0){"Green"}else{"Yellow"})

    if ($reportPath) {
        Write-Host "`nReport: $reportPath" -ForegroundColor Cyan
    }

    return @{
        Success = $true
        TotalBlueprints = $results.Count
        ValidBlueprints = $validCount
        InvalidBlueprints = $invalidCount
        TotalIssues = $totalIssues
        TotalWarnings = $totalWarnings
        ReportPath = $reportPath
        Results = $results
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
