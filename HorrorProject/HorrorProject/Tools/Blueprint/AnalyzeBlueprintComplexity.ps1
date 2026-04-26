<#
.SYNOPSIS
    Analyze Blueprint complexity metrics
.DESCRIPTION
    Calculates complexity metrics for Blueprints including node count, depth, and maintainability scores
.PARAMETER BlueprintPath
    Path to Blueprint file or directory
.PARAMETER GenerateReport
    Generate detailed complexity report with recommendations
.PARAMETER Threshold
    Complexity threshold for warnings (default: 100)
.EXAMPLE
    .\AnalyzeBlueprintComplexity.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -GenerateReport -Threshold 80
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$BlueprintPath,

    [Parameter(Mandatory=$false)]
    [switch]$GenerateReport,

    [Parameter(Mandatory=$false)]
    [int]$Threshold = 100,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$LogFile = Join-Path $PSScriptRoot "Logs/AnalyzeBlueprintComplexity_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

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

function Get-ComplexityMetrics {
    param([string]$FilePath)

    $metrics = @{
        ComponentCount = 0
        VariableCount = 0
        FunctionCount = 0
        EventCount = 0
        NodeCount = 0
        ConnectionCount = 0
        MaxDepth = 0
        CyclomaticComplexity = 0
        MaintainabilityIndex = 100
        LinesOfCode = 0
        CommentRatio = 0
    }

    try {
        if ($FilePath -match '\.json$') {
            $content = Get-Content $FilePath -Raw | ConvertFrom-Json

            # Component count
            if ($content.Components) {
                $metrics.ComponentCount = $content.Components.Count
            }
            if ($content.WidgetTree -and $content.WidgetTree.Components) {
                $metrics.ComponentCount += $content.WidgetTree.Components.Count
            }

            # Variable count
            if ($content.Variables) {
                $metrics.VariableCount = $content.Variables.Count
            }

            # Function and event count
            if ($content.Functions) {
                $metrics.FunctionCount = ($content.Functions | Where-Object { $_.Type -ne "Event" }).Count
                $metrics.EventCount = ($content.Functions | Where-Object { $_.Type -eq "Event" }).Count
            }

            # Node and connection count
            if ($content.EventGraph) {
                if ($content.EventGraph.Nodes) {
                    $metrics.NodeCount = $content.EventGraph.Nodes.Count
                }
                if ($content.EventGraph.Connections) {
                    $metrics.ConnectionCount = $content.EventGraph.Connections.Count
                }
            }

            # Calculate cyclomatic complexity (simplified)
            # CC = E - N + 2P (edges - nodes + 2*connected components)
            if ($metrics.NodeCount -gt 0) {
                $metrics.CyclomaticComplexity = [Math]::Max(1, $metrics.ConnectionCount - $metrics.NodeCount + 2)
            }

            # Estimate lines of code
            $metrics.LinesOfCode = $metrics.NodeCount * 3 + $metrics.FunctionCount * 10

            # Calculate maintainability index (simplified)
            # MI = 171 - 5.2 * ln(HV) - 0.23 * CC - 16.2 * ln(LOC)
            if ($metrics.LinesOfCode -gt 0) {
                $halsteadVolume = [Math]::Max(1, $metrics.NodeCount * [Math]::Log($metrics.VariableCount + $metrics.FunctionCount + 1))
                $mi = 171 - 5.2 * [Math]::Log($halsteadVolume) - 0.23 * $metrics.CyclomaticComplexity - 16.2 * [Math]::Log($metrics.LinesOfCode)
                $metrics.MaintainabilityIndex = [Math]::Max(0, [Math]::Min(100, $mi))
            }

        } else {
            # For .uasset files, use file size as rough estimate
            $fileInfo = Get-Item $FilePath
            $sizeKB = $fileInfo.Length / 1KB

            # Rough estimates based on file size
            $metrics.NodeCount = [int]($sizeKB * 2)
            $metrics.LinesOfCode = [int]($sizeKB * 5)
            $metrics.MaintainabilityIndex = [Math]::Max(0, 100 - ($sizeKB / 10))
        }

    } catch {
        Write-Log "Failed to calculate metrics for $FilePath : $($_.Exception.Message)" -Level "ERROR"
    }

    return $metrics
}

function Get-ComplexityScore {
    param([hashtable]$Metrics)

    # Weighted complexity score
    $score = 0
    $score += $Metrics.ComponentCount * 2
    $score += $Metrics.VariableCount * 1
    $score += $Metrics.FunctionCount * 3
    $score += $Metrics.EventCount * 2
    $score += $Metrics.NodeCount * 0.5
    $score += $Metrics.CyclomaticComplexity * 5

    return [Math]::Round($score, 2)
}

function Get-ComplexityRating {
    param([double]$Score)

    if ($Score -lt 20) { return "Low" }
    elseif ($Score -lt 50) { return "Moderate" }
    elseif ($Score -lt 100) { return "High" }
    else { return "Very High" }
}

function Get-ComplexityRecommendations {
    param([hashtable]$Metrics, [double]$Score)

    $recommendations = @()

    if ($Metrics.ComponentCount -gt 20) {
        $recommendations += "Consider splitting into multiple Blueprints - high component count ($($Metrics.ComponentCount))"
    }

    if ($Metrics.VariableCount -gt 50) {
        $recommendations += "Use structs to group related variables - high variable count ($($Metrics.VariableCount))"
    }

    if ($Metrics.FunctionCount -gt 30) {
        $recommendations += "Extract functionality into component Blueprints - high function count ($($Metrics.FunctionCount))"
    }

    if ($Metrics.NodeCount -gt 200) {
        $recommendations += "Refactor large event graphs into smaller functions - high node count ($($Metrics.NodeCount))"
    }

    if ($Metrics.CyclomaticComplexity -gt 20) {
        $recommendations += "Reduce branching logic complexity - high cyclomatic complexity ($($Metrics.CyclomaticComplexity))"
    }

    if ($Metrics.MaintainabilityIndex -lt 50) {
        $recommendations += "Blueprint maintainability is low - consider refactoring (MI: $([Math]::Round($Metrics.MaintainabilityIndex, 2)))"
    }

    if ($Score -gt 150) {
        $recommendations += "Blueprint is extremely complex - strongly recommend breaking into smaller pieces"
    }

    if ($recommendations.Count -eq 0) {
        $recommendations += "Blueprint complexity is within acceptable limits"
    }

    return $recommendations
}

function New-ComplexityReport {
    param([array]$Results, [string]$OutputPath, [int]$Threshold)

    $reportPath = Join-Path $OutputPath "ComplexityReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"

    $highComplexity = $Results | Where-Object { $_.ComplexityScore -gt $Threshold } | Sort-Object -Property ComplexityScore -Descending
    $avgScore = ($Results | Measure-Object -Property ComplexityScore -Average).Average
    $maxScore = ($Results | Measure-Object -Property ComplexityScore -Maximum).Maximum

    $html = @"
<!DOCTYPE html>
<html>
<head>
    <title>Blueprint Complexity Report</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; border-bottom: 3px solid #0078d4; padding-bottom: 10px; }
        h2 { color: #555; margin-top: 30px; }
        .summary { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin: 20px 0; }
        .metric-card { background: #f8f9fa; padding: 20px; border-radius: 6px; border-left: 4px solid #0078d4; }
        .metric-value { font-size: 32px; font-weight: bold; color: #0078d4; }
        .metric-label { color: #666; font-size: 14px; margin-top: 5px; }
        table { width: 100%; border-collapse: collapse; margin: 20px 0; }
        th { background: #0078d4; color: white; padding: 12px; text-align: left; }
        td { padding: 10px; border-bottom: 1px solid #ddd; }
        tr:hover { background: #f8f9fa; }
        .rating-low { color: #28a745; font-weight: bold; }
        .rating-moderate { color: #ffc107; font-weight: bold; }
        .rating-high { color: #fd7e14; font-weight: bold; }
        .rating-veryhigh { color: #dc3545; font-weight: bold; }
        .recommendations { background: #fff3cd; border-left: 4px solid #ffc107; padding: 15px; margin: 10px 0; border-radius: 4px; }
        .recommendations ul { margin: 10px 0; padding-left: 20px; }
        .footer { margin-top: 30px; padding-top: 20px; border-top: 1px solid #ddd; color: #666; font-size: 12px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Blueprint Complexity Analysis Report</h1>
        <p>Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>

        <div class="summary">
            <div class="metric-card">
                <div class="metric-value">$($Results.Count)</div>
                <div class="metric-label">Total Blueprints</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$([Math]::Round($avgScore, 1))</div>
                <div class="metric-label">Average Complexity</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$([Math]::Round($maxScore, 1))</div>
                <div class="metric-label">Max Complexity</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($highComplexity.Count)</div>
                <div class="metric-label">Above Threshold ($Threshold)</div>
            </div>
        </div>

        <h2>High Complexity Blueprints</h2>
        <table>
            <tr>
                <th>Blueprint</th>
                <th>Score</th>
                <th>Rating</th>
                <th>Nodes</th>
                <th>Functions</th>
                <th>Variables</th>
                <th>MI</th>
            </tr>
"@

    foreach ($result in $highComplexity) {
        $ratingClass = switch ($result.ComplexityRating) {
            "Low" { "rating-low" }
            "Moderate" { "rating-moderate" }
            "High" { "rating-high" }
            "Very High" { "rating-veryhigh" }
        }

        $html += @"
            <tr>
                <td>$($result.FileName)</td>
                <td>$([Math]::Round($result.ComplexityScore, 1))</td>
                <td class="$ratingClass">$($result.ComplexityRating)</td>
                <td>$($result.Metrics.NodeCount)</td>
                <td>$($result.Metrics.FunctionCount)</td>
                <td>$($result.Metrics.VariableCount)</td>
                <td>$([Math]::Round($result.Metrics.MaintainabilityIndex, 1))</td>
            </tr>
"@
    }

    $html += @"
        </table>

        <h2>Recommendations</h2>
"@

    foreach ($result in ($highComplexity | Select-Object -First 10)) {
        $html += @"
        <div class="recommendations">
            <strong>$($result.FileName)</strong> (Score: $([Math]::Round($result.ComplexityScore, 1)))
            <ul>
"@
        foreach ($rec in $result.Recommendations) {
            $html += "                <li>$rec</li>`n"
        }
        $html += @"
            </ul>
        </div>
"@
    }

    $html += @"
        <div class="footer">
            <p>Complexity Score = (Components × 2) + (Variables × 1) + (Functions × 3) + (Events × 2) + (Nodes × 0.5) + (Cyclomatic Complexity × 5)</p>
            <p>Maintainability Index (MI) ranges from 0-100, where higher is better. MI &lt; 50 indicates low maintainability.</p>
        </div>
    </div>
</body>
</html>
"@

    $html | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Log "HTML report saved to: $reportPath"

    return $reportPath
}

# Main execution
try {
    Write-Log "Starting Blueprint complexity analysis"
    Write-Log "Blueprint Path: $BlueprintPath"
    Write-Log "Complexity Threshold: $Threshold"

    # Create logs directory
    $logsDir = Join-Path $PSScriptRoot "Logs"
    if (-not (Test-Path $logsDir)) {
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
    }

    # Get Blueprint files
    $blueprintFiles = if (Test-Path $BlueprintPath -PathType Leaf) {
        @(Get-Item $BlueprintPath)
    } else {
        Get-ChildItem -Path $BlueprintPath -Include "*.uasset","*.json" -Recurse |
            Where-Object { $_.Name -notmatch '\.metadata\.json$' }
    }

    Write-Log "Found $($blueprintFiles.Count) Blueprint files"

    if ($blueprintFiles.Count -eq 0) {
        Write-Log "No Blueprint files found" -Level "WARN"
        return
    }

    # Analyze complexity
    $results = @()

    foreach ($file in $blueprintFiles) {
        Write-Log "Analyzing: $($file.FullName)"

        $metrics = Get-ComplexityMetrics -FilePath $file.FullName
        $score = Get-ComplexityScore -Metrics $metrics
        $rating = Get-ComplexityRating -Score $score
        $recommendations = Get-ComplexityRecommendations -Metrics $metrics -Score $score

        $result = @{
            FilePath = $file.FullName
            FileName = $file.Name
            Metrics = $metrics
            ComplexityScore = $score
            ComplexityRating = $rating
            Recommendations = $recommendations
        }

        $results += $result

        Write-Log "  Score: $score ($rating)"
        Write-Log "  Nodes: $($metrics.NodeCount), Functions: $($metrics.FunctionCount), Variables: $($metrics.VariableCount)"
        Write-Log "  Maintainability Index: $([Math]::Round($metrics.MaintainabilityIndex, 2))"

        if ($score -gt $Threshold) {
            Write-Log "  WARNING: Complexity exceeds threshold!" -Level "WARN"
        }
    }

    # Sort by complexity
    $results = $results | Sort-Object -Property ComplexityScore -Descending

    # Generate reports
    $jsonReportPath = Join-Path $logsDir "ComplexityReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    @{
        GeneratedDate = Get-Date -Format "o"
        Threshold = $Threshold
        TotalBlueprints = $results.Count
        AverageComplexity = ($results | Measure-Object -Property ComplexityScore -Average).Average
        MaxComplexity = ($results | Measure-Object -Property ComplexityScore -Maximum).Maximum
        AboveThreshold = ($results | Where-Object { $_.ComplexityScore -gt $Threshold }).Count
        Results = $results
    } | ConvertTo-Json -Depth 10 | Out-File -FilePath $jsonReportPath -Encoding UTF8

    Write-Log "JSON report saved to: $jsonReportPath"

    $htmlReportPath = $null
    if ($GenerateReport) {
        $htmlReportPath = New-ComplexityReport -Results $results -OutputPath $logsDir -Threshold $Threshold
    }

    # Summary
    $avgScore = ($results | Measure-Object -Property ComplexityScore -Average).Average
    $maxScore = ($results | Measure-Object -Property ComplexityScore -Maximum).Maximum
    $aboveThreshold = ($results | Where-Object { $_.ComplexityScore -gt $Threshold }).Count

    Write-Host "`n=== Complexity Analysis Complete ===" -ForegroundColor Cyan
    Write-Host "Total Blueprints: $($results.Count)" -ForegroundColor White
    Write-Host "Average Complexity: $([Math]::Round($avgScore, 2))" -ForegroundColor White
    Write-Host "Max Complexity: $([Math]::Round($maxScore, 2))" -ForegroundColor White
    Write-Host "Above Threshold ($Threshold): $aboveThreshold" -ForegroundColor $(if($aboveThreshold -eq 0){"Green"}else{"Yellow"})
    Write-Host "`nJSON Report: $jsonReportPath" -ForegroundColor Cyan
    if ($htmlReportPath) {
        Write-Host "HTML Report: $htmlReportPath" -ForegroundColor Cyan
    }

    return @{
        Success = $true
        TotalBlueprints = $results.Count
        AverageComplexity = $avgScore
        MaxComplexity = $maxScore
        AboveThreshold = $aboveThreshold
        JsonReportPath = $jsonReportPath
        HtmlReportPath = $htmlReportPath
        Results = $results
    }

} catch {
    Write-Log "Error: $($_.Exception.Message)" -Level "ERROR"
    Write-Log "Stack trace: $($_.ScriptStackTrace)" -Level "ERROR"
    throw
}
