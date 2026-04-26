# CodeStyleCheck.ps1 - UE5 Code Style Compliance Checker
# Checks C++ code against Unreal Engine coding standards

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/CodeStyleReport.md"
)

$ErrorCount = 0
$WarningCount = 0
$Issues = @()

function Add-Issue {
    param($File, $Line, $Severity, $Category, $Message)
    $script:Issues += [PSCustomObject]@{
        File = $File
        Line = $Line
        Severity = $Severity
        Category = $Category
        Message = $Message
    }
    if ($Severity -eq "Error") { $script:ErrorCount++ }
    else { $script:WarningCount++ }
}

function Check-NamingConventions {
    param($FilePath, $Content)

    $lines = $Content -split "`n"
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Check class naming (should start with A, U, F, E, I, T)
        if ($line -match '^\s*class\s+(?!HORRORPROJECT_API\s+)(\w+)') {
            $className = $Matches[1]
            if ($className -notmatch '^[AUFEITS]') {
                Add-Issue $FilePath ($i+1) "Error" "Naming" "Class '$className' should start with UE prefix (A/U/F/E/I/T/S)"
            }
        }

        # Check member variable naming (should start with lowercase or m_)
        if ($line -match '^\s+(?:UPROPERTY|UFUNCTION)') {
            $nextLine = if ($i+1 -lt $lines.Count) { $lines[$i+1] } else { "" }
            if ($nextLine -match '\s+([A-Z]\w+)\s+(\w+);') {
                $varName = $Matches[2]
                if ($varName -match '^[A-Z]' -and $varName -notmatch '^b[A-Z]') {
                    Add-Issue $FilePath ($i+2) "Warning" "Naming" "Member variable '$varName' should start with lowercase or 'b' for bool"
                }
            }
        }

        # Check for Hungarian notation misuse
        if ($line -match '\s+(p|sz|n|i|f|d|b)([A-Z]\w+)\s*[=;]' -and $line -notmatch 'UPROPERTY') {
            Add-Issue $FilePath ($i+1) "Warning" "Naming" "Avoid Hungarian notation in modern C++"
        }
    }
}

function Check-HeaderGuards {
    param($FilePath, $Content)

    if ($FilePath -notmatch '\.h$') { return }

    $expectedGuard = ($FilePath -replace '[\\/]', '_' -replace '\.h$', '_H').ToUpper()

    if ($Content -notmatch '#pragma once') {
        if ($Content -notmatch "#ifndef\s+$expectedGuard") {
            Add-Issue $FilePath 1 "Error" "HeaderGuard" "Missing #pragma once or proper header guard"
        }
    }
}

function Check-IncludeOrder {
    param($FilePath, $Content)

    $lines = $Content -split "`n"
    $includes = @()
    $inIncludeBlock = $false

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i].Trim()
        if ($line -match '^#include\s+"([^"]+)"') {
            $includes += @{ Line = $i+1; Include = $Matches[1]; Type = "Local" }
            $inIncludeBlock = $true
        }
        elseif ($line -match '^#include\s+<([^>]+)>') {
            $includes += @{ Line = $i+1; Include = $Matches[1]; Type = "System" }
            $inIncludeBlock = $true
        }
        elseif ($inIncludeBlock -and $line -ne "" -and $line -notmatch '^#') {
            break
        }
    }

    # Check order: .generated.h should be last
    $generatedIndex = -1
    for ($i = 0; $i -lt $includes.Count; $i++) {
        if ($includes[$i].Include -match '\.generated\.h$') {
            $generatedIndex = $i
            break
        }
    }

    if ($generatedIndex -ge 0 -and $generatedIndex -ne ($includes.Count - 1)) {
        Add-Issue $FilePath $includes[$generatedIndex].Line "Error" "IncludeOrder" ".generated.h must be the last include"
    }
}

function Check-Comments {
    param($FilePath, $Content)

    $lines = $Content -split "`n"
    $hasFileComment = $false

    # Check for file-level comment
    if ($lines[0] -match '^//\s*\w+' -or $lines[0] -match '^/\*') {
        $hasFileComment = $true
    }

    if (-not $hasFileComment) {
        Add-Issue $FilePath 1 "Warning" "Documentation" "Missing file-level documentation comment"
    }

    # Check public API documentation
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        if ($line -match '^\s*UFUNCTION\(.*BlueprintCallable') {
            $nextLine = if ($i+1 -lt $lines.Count) { $lines[$i+1] } else { "" }
            $prevLine = if ($i-1 -ge 0) { $lines[$i-1] } else { "" }

            if ($prevLine -notmatch '^\s*//' -and $prevLine -notmatch '^\s*/\*') {
                Add-Issue $FilePath ($i+1) "Warning" "Documentation" "BlueprintCallable function missing documentation"
            }
        }
    }
}

function Check-ErrorHandling {
    param($FilePath, $Content)

    $lines = $Content -split "`n"

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Check for raw pointer dereference without null check
        if ($line -match '(\w+)->(\w+)' -and $line -notmatch 'if\s*\(' -and $line -notmatch 'check\(' -and $line -notmatch 'ensure') {
            $varName = $Matches[1]

            # Look back for null check
            $hasNullCheck = $false
            for ($j = [Math]::Max(0, $i-5); $j -lt $i; $j++) {
                if ($lines[$j] -match "if\s*\(\s*$varName\s*[!=]=\s*nullptr" -or
                    $lines[$j] -match "check\(\s*$varName" -or
                    $lines[$j] -match "ensure\(\s*$varName") {
                    $hasNullCheck = $true
                    break
                }
            }

            if (-not $hasNullCheck -and $varName -notmatch '^(this|Get|Find)') {
                Add-Issue $FilePath ($i+1) "Warning" "ErrorHandling" "Potential null pointer dereference: $varName"
            }
        }

        # Check for array access without bounds check
        if ($line -match '(\w+)\[(\w+)\]' -and $line -notmatch 'if\s*\(' -and $line -notmatch 'check\(') {
            Add-Issue $FilePath ($i+1) "Warning" "ErrorHandling" "Array access without bounds check"
        }
    }
}

# Main execution
Write-Host "Starting code style check..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h | Where-Object { $_.FullName -notmatch '\\Intermediate\\' }

foreach ($file in $files) {
    Write-Host "Checking: $($file.Name)" -ForegroundColor Gray
    $content = Get-Content $file.FullName -Raw

    Check-NamingConventions $file.FullName $content
    Check-HeaderGuards $file.FullName $content
    Check-IncludeOrder $file.FullName $content
    Check-Comments $file.FullName $content
    Check-ErrorHandling $file.FullName $content
}

# Generate report
$report = @"
# Code Style Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Summary
- Files Checked: $($files.Count)
- Errors: $ErrorCount
- Warnings: $WarningCount
- Total Issues: $($Issues.Count)

## Issues by Category
"@

$byCategory = $Issues | Group-Object Category | Sort-Object Count -Descending
foreach ($group in $byCategory) {
    $report += "`n### $($group.Name) ($($group.Count) issues)`n"
    foreach ($issue in $group.Group | Select-Object -First 10) {
        $report += "- **$($issue.Severity)** [$($issue.File):$($issue.Line)] $($issue.Message)`n"
    }
    if ($group.Count -gt 10) {
        $report += "- ... and $($group.Count - 10) more`n"
    }
}

$report += "`n## All Issues`n"
foreach ($issue in $Issues | Sort-Object Severity,File,Line) {
    $report += "- **$($issue.Severity)** [$($issue.Category)] $($issue.File):$($issue.Line) - $($issue.Message)`n"
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nCode style check complete!" -ForegroundColor Green
Write-Host "Errors: $ErrorCount, Warnings: $WarningCount" -ForegroundColor $(if($ErrorCount -gt 0){"Red"}else{"Green"})
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
