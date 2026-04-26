# Validate Translations for Completeness and Quality
# Usage: .\ValidateTranslations.ps1

param(
    [string]$LocalizationPath = "D:\gptzuo\HorrorProject\HorrorProject\Content\Localization",
    [string]$ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Content\Localization\ValidationReport.html"
)

Write-Host "Validating translations in: $LocalizationPath"

$Languages = @("en", "zh-Hans", "ja", "ko", "es")
$Issues = @()
$Stats = @{}

function Test-TranslationQuality {
    param(
        [string]$Key,
        [string]$SourceText,
        [string]$Translation,
        [string]$Language
    )

    $LocalIssues = @()

    # Check for missing translation
    if ([string]::IsNullOrWhiteSpace($Translation)) {
        $LocalIssues += "Missing translation"
    }

    # Check for TODO markers
    if ($Translation -like "[TODO:*") {
        $LocalIssues += "Translation pending"
    }

    # Check for placeholder text
    if ($Translation -eq $SourceText -and $Language -ne "en") {
        $LocalIssues += "Using English text as placeholder"
    }

    # Check for format string mismatches
    $SourcePlaceholders = [regex]::Matches($SourceText, '\{(\d+)\}').Count
    $TransPlaceholders = [regex]::Matches($Translation, '\{(\d+)\}').Count
    if ($SourcePlaceholders -ne $TransPlaceholders) {
        $LocalIssues += "Format placeholder mismatch (source: $SourcePlaceholders, translation: $TransPlaceholders)"
    }

    # Check for excessive length difference (potential truncation issues)
    if ($Translation.Length -gt 0) {
        $LengthRatio = $Translation.Length / [Math]::Max($SourceText.Length, 1)
        if ($LengthRatio -gt 2.5) {
            $LocalIssues += "Translation significantly longer than source (may cause UI issues)"
        }
    }

    return $LocalIssues
}

# Load master table
$MasterTablePath = Join-Path $LocalizationPath "MasterLocalizationTable.csv"
if (-not (Test-Path $MasterTablePath)) {
    Write-Host "Master table not found. Run GenerateLocalizationFiles.ps1 first."
    exit 1
}

$MasterTable = Import-Csv -Path $MasterTablePath -Encoding UTF8

# Initialize stats
foreach ($Lang in $Languages) {
    $Stats[$Lang] = @{
        Total = 0
        Complete = 0
        Pending = 0
        Missing = 0
        Issues = 0
    }
}

# Validate each entry
foreach ($Entry in $MasterTable) {
    $Key = $Entry.Key
    $SourceText = $Entry.English

    foreach ($Lang in $Languages) {
        $Stats[$Lang].Total++

        $Translation = switch ($Lang) {
            "en" { $Entry.English }
            "zh-Hans" { $Entry.Chinese }
            "ja" { $Entry.Japanese }
            "ko" { $Entry.Korean }
            "es" { $Entry.Spanish }
        }

        $EntryIssues = Test-TranslationQuality -Key $Key -SourceText $SourceText -Translation $Translation -Language $Lang

        if ($EntryIssues.Count -eq 0) {
            $Stats[$Lang].Complete++
        } else {
            if ([string]::IsNullOrWhiteSpace($Translation)) {
                $Stats[$Lang].Missing++
            } elseif ($Translation -like "[TODO:*") {
                $Stats[$Lang].Pending++
            } else {
                $Stats[$Lang].Issues++
            }

            foreach ($Issue in $EntryIssues) {
                $Issues += [PSCustomObject]@{
                    Language = $Lang
                    Key = $Key
                    SourceText = $SourceText
                    Translation = $Translation
                    Issue = $Issue
                    Severity = if ($Issue -like "*Missing*") { "High" } elseif ($Issue -like "*pending*") { "Medium" } else { "Low" }
                }
            }
        }
    }
}

# Generate HTML report
$HtmlReport = @"
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Localization Validation Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        h1 { color: #333; }
        .summary { background: white; padding: 20px; margin: 20px 0; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .stats { display: flex; gap: 20px; flex-wrap: wrap; }
        .stat-card { background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); min-width: 200px; }
        .stat-card h3 { margin: 0 0 10px 0; color: #555; }
        .progress-bar { background: #e0e0e0; height: 20px; border-radius: 10px; overflow: hidden; margin: 10px 0; }
        .progress-fill { background: #4caf50; height: 100%; transition: width 0.3s; }
        .issues-table { width: 100%; border-collapse: collapse; background: white; margin: 20px 0; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .issues-table th, .issues-table td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }
        .issues-table th { background: #f0f0f0; font-weight: bold; }
        .severity-high { color: #d32f2f; font-weight: bold; }
        .severity-medium { color: #f57c00; }
        .severity-low { color: #388e3c; }
        .filter-buttons { margin: 20px 0; }
        .filter-btn { padding: 8px 16px; margin-right: 10px; border: none; background: #2196f3; color: white; border-radius: 4px; cursor: pointer; }
        .filter-btn:hover { background: #1976d2; }
        .filter-btn.active { background: #0d47a1; }
    </style>
</head>
<body>
    <h1>Localization Validation Report</h1>
    <p>Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>

    <div class="summary">
        <h2>Translation Statistics</h2>
        <div class="stats">
"@

foreach ($Lang in $Languages) {
    $LangStats = $Stats[$Lang]
    $CompletionRate = if ($LangStats.Total -gt 0) { [math]::Round(($LangStats.Complete / $LangStats.Total) * 100, 1) } else { 0 }

    $HtmlReport += @"
            <div class="stat-card">
                <h3>$Lang</h3>
                <p>Total: $($LangStats.Total)</p>
                <p>Complete: $($LangStats.Complete)</p>
                <p>Pending: $($LangStats.Pending)</p>
                <p>Missing: $($LangStats.Missing)</p>
                <p>Issues: $($LangStats.Issues)</p>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: $CompletionRate%"></div>
                </div>
                <p>Completion: $CompletionRate%</p>
            </div>
"@
}

$HtmlReport += @"
        </div>
    </div>

    <h2>Issues Found ($($Issues.Count))</h2>
    <div class="filter-buttons">
        <button class="filter-btn active" onclick="filterIssues('all')">All</button>
        <button class="filter-btn" onclick="filterIssues('high')">High Severity</button>
        <button class="filter-btn" onclick="filterIssues('medium')">Medium Severity</button>
        <button class="filter-btn" onclick="filterIssues('low')">Low Severity</button>
    </div>

    <table class="issues-table" id="issuesTable">
        <thead>
            <tr>
                <th>Language</th>
                <th>Key</th>
                <th>Source Text</th>
                <th>Translation</th>
                <th>Issue</th>
                <th>Severity</th>
            </tr>
        </thead>
        <tbody>
"@

foreach ($Issue in $Issues) {
    $SeverityClass = "severity-$($Issue.Severity.ToLower())"
    $HtmlReport += @"
            <tr class="issue-row" data-severity="$($Issue.Severity.ToLower())">
                <td>$($Issue.Language)</td>
                <td>$($Issue.Key)</td>
                <td>$($Issue.SourceText)</td>
                <td>$($Issue.Translation)</td>
                <td>$($Issue.Issue)</td>
                <td class="$SeverityClass">$($Issue.Severity)</td>
            </tr>
"@
}

$HtmlReport += @"
        </tbody>
    </table>

    <script>
        function filterIssues(severity) {
            const rows = document.querySelectorAll('.issue-row');
            const buttons = document.querySelectorAll('.filter-btn');

            buttons.forEach(btn => btn.classList.remove('active'));
            event.target.classList.add('active');

            rows.forEach(row => {
                if (severity === 'all' || row.dataset.severity === severity) {
                    row.style.display = '';
                } else {
                    row.style.display = 'none';
                }
            });
        }
    </script>
</body>
</html>
"@

# Save report
$HtmlReport | Out-File -FilePath $ReportPath -Encoding UTF8
Write-Host "Validation report saved to: $ReportPath"

# Console summary
Write-Host "`n=== Validation Summary ==="
foreach ($Lang in $Languages) {
    $LangStats = $Stats[$Lang]
    $CompletionRate = if ($LangStats.Total -gt 0) { [math]::Round(($LangStats.Complete / $LangStats.Total) * 100, 1) } else { 0 }
    Write-Host "$Lang : $CompletionRate% complete ($($LangStats.Complete)/$($LangStats.Total))"
}

Write-Host "`nTotal Issues: $($Issues.Count)"
Write-Host "High Severity: $(($Issues | Where-Object { $_.Severity -eq 'High' }).Count)"
Write-Host "Medium Severity: $(($Issues | Where-Object { $_.Severity -eq 'Medium' }).Count)"
Write-Host "Low Severity: $(($Issues | Where-Object { $_.Severity -eq 'Low' }).Count)"

# Export issues to CSV
$IssuesCsvPath = Join-Path (Split-Path $ReportPath -Parent) "ValidationIssues.csv"
$Issues | Export-Csv -Path $IssuesCsvPath -NoTypeInformation -Encoding UTF8
Write-Host "`nIssues exported to: $IssuesCsvPath"
