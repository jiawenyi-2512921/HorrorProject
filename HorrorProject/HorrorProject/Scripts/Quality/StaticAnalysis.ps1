# StaticAnalysis.ps1 - Static Code Analysis for UE5 C++
# Detects common bugs, memory issues, and code smells

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/StaticAnalysisReport.md"
)

$Issues = @()

function Add-Issue {
    param($File, $Line, $Severity, $Category, $Message, $Recommendation)
    $script:Issues += [PSCustomObject]@{
        File = $File
        Line = $Line
        Severity = $Severity
        Category = $Category
        Message = $Message
        Recommendation = $Recommendation
    }
}

function Check-MemoryManagement {
    param($FilePath, $Content)

    $lines = $Content -split "`n"

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Check for raw new/delete
        if ($line -cmatch '\bnew\s+(U\w+|A\w+)' -and $line -notmatch 'NewObject' -and $line -notmatch 'UPROPERTY') {
            Add-Issue $FilePath ($i+1) "Warning" "Memory" "Raw 'new' detected" "Use NewObject<> or TSharedPtr/TUniquePtr"
        }

        if ($line -cmatch '\bdelete\s+\w+') {
            Add-Issue $FilePath ($i+1) "Warning" "Memory" "Raw 'delete' detected" "Let UE GC manage object lifetime"
        }

        # Check for missing UPROPERTY on UObject pointers
        if ($line -match '^\s+(U\w+|A\w+)\s*\*\s*(\w+);' -and $lines[$i-1] -notmatch 'UPROPERTY') {
            $className = $Matches[1]
            $varName = $Matches[2]
            Add-Issue $FilePath ($i+1) "Error" "Memory" "UObject pointer '$varName' missing UPROPERTY()" "Add UPROPERTY() for GC tracking"
        }

        # Check for TSharedPtr on UObject
        if ($line -match 'TSharedPtr\s*<\s*(U\w+|A\w+)') {
            Add-Issue $FilePath ($i+1) "Error" "Memory" "TSharedPtr used with UObject" "Use raw pointer with UPROPERTY() instead"
        }

        # Check for missing TWeakObjectPtr
        if ($line -match 'UPROPERTY.*\n.*?(U\w+|A\w+)\s*\*\s*(\w+);' -and $line -notmatch 'TWeakObjectPtr') {
            $varName = $Matches[2]
            if ($varName -notmatch '^(Owner|Parent|World)') {
                Add-Issue $FilePath ($i+1) "Warning" "Memory" "Consider TWeakObjectPtr for '$varName'" "Prevents dangling pointers"
            }
        }
    }
}

function Check-ThreadSafety {
    param($FilePath, $Content)

    $lines = $Content -split "`n"

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Check for mutable state in const methods
        if ($line -match 'mutable\s+\w+') {
            Add-Issue $FilePath ($i+1) "Warning" "ThreadSafety" "Mutable member detected" "Ensure thread-safe access"
        }

        # Check for static variables
        if ($line -match '^\s*static\s+(?!const\b|constexpr\b|constinit\b)[^;()]+;') {
            Add-Issue $FilePath ($i+1) "Warning" "ThreadSafety" "Non-const static variable" "May cause race conditions"
        }

        # Check for shared container modification without lock
        if ($line -match '^\s*static\s+(TArray|TMap|TSet)\s*<.*>\s*(\w+)' -and $line -notmatch 'const') {
            $containerName = $Matches[2]
            # Look for Add/Remove/Empty calls
            for ($j = $i; $j -lt [Math]::Min($i+20, $lines.Count); $j++) {
                if ($lines[$j] -match "$containerName\.(Add|Remove|Empty|Reset)\(") {
                    # Check if inside critical section
                    $hasCriticalSection = $false
                    for ($k = [Math]::Max(0, $j-10); $k -lt $j; $k++) {
                        if ($lines[$k] -match 'FScopeLock|FCriticalSection') {
                            $hasCriticalSection = $true
                            break
                        }
                    }
                    if (-not $hasCriticalSection) {
                        Add-Issue $FilePath ($j+1) "Warning" "ThreadSafety" "Container modification without lock" "Use FScopeLock if accessed from multiple threads"
                    }
                    break
                }
            }
        }
    }
}

function Check-PerformanceIssues {
    param($FilePath, $Content)

    $lines = $Content -split "`n"

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Check for pass-by-value of large types
        $previousAttributes = if ($i -gt 0) { ($lines[[Math]::Max(0, $i - 2)..$i] -join " ") } else { $line }
        if ($line -cmatch '\w+\s+\w+\([^)]*\b(?!FName\b|FKey\b|FGameplayTag\b|FGuid\b|FVector\b|FRotator\b|FColor\b|FLinearColor\b)(F[A-Z]\w+|TArray|TMap|FString)\s+(\w+)\)' -and
            $previousAttributes -notmatch 'UFUNCTION\s*\(\s*Exec\s*\)') {
            $typeName = $Matches[1]
            Add-Issue $FilePath ($i+1) "Warning" "Performance" "Pass '$typeName' by const reference" "Avoid unnecessary copies"
        }

        # Check for FString in loops
        if ($line -match 'for\s*\(' -or $line -match 'while\s*\(') {
            $braceDepth = 0
            $foundLoopBody = $false
            for ($j = $i; $j -lt $lines.Count; $j++) {
                if ($lines[$j] -match 'FString\s+\w+\s*=') {
                    Add-Issue $FilePath ($j+1) "Warning" "Performance" "FString allocation in loop" "Move outside loop or use FName"
                }

                $braceDepth += ($lines[$j].ToCharArray() | Where-Object { $_ -eq '{' }).Count
                if ($braceDepth -gt 0) {
                    $foundLoopBody = $true
                }
                $braceDepth -= ($lines[$j].ToCharArray() | Where-Object { $_ -eq '}' }).Count
                if ($foundLoopBody -and $braceDepth -le 0) {
                    break
                }

                if (-not $foundLoopBody -and $j -gt $i + 1) {
                    break
                }
            }
        }

        # Check for GetWorld() in Tick
        if ($line -match 'void\s+\w+::Tick') {
            $braceDepth = 0
            $foundFunctionBody = $false
            for ($j = $i; $j -lt $lines.Count; $j++) {
                if ($lines[$j] -match 'GetWorld\(\)' -and $lines[$j] -notmatch 'UWorld\s*\*\s*World\s*=') {
                    Add-Issue $FilePath ($j+1) "Warning" "Performance" "GetWorld() called in Tick" "Cache World pointer"
                }

                $braceDepth += ($lines[$j].ToCharArray() | Where-Object { $_ -eq '{' }).Count
                if ($braceDepth -gt 0) {
                    $foundFunctionBody = $true
                }
                $braceDepth -= ($lines[$j].ToCharArray() | Where-Object { $_ -eq '}' }).Count
                if ($foundFunctionBody -and $braceDepth -le 0) {
                    break
                }
            }
        }

        # Check for FindObject in hot paths
        if ($line -match '(FindObject|FindActor|GetActorOfClass)') {
            Add-Issue $FilePath ($i+1) "Warning" "Performance" "Expensive search function" "Cache result if called frequently"
        }
    }
}

function Check-APIDesign {
    param($FilePath, $Content)

    $lines = $Content -split "`n"

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Check BlueprintCallable without Category
        if ($line -match 'UFUNCTION\(.*BlueprintCallable' -and $line -notmatch 'Category\s*=') {
            Add-Issue $FilePath ($i+1) "Warning" "API" "BlueprintCallable missing Category" "Add Category for better organization"
        }

        # Check public members without UPROPERTY
        if ($line -match '^public:') {
            for ($j = $i+1; $j -lt [Math]::Min($i+20, $lines.Count); $j++) {
                if ($lines[$j] -match '^\s+(F\w+|TArray|TMap|int|float|bool)\s+(\w+);' -and
                    $lines[$j-1] -notmatch 'UPROPERTY') {
                    Add-Issue $FilePath ($j+1) "Warning" "API" "Public member without UPROPERTY" "Encapsulate or add UPROPERTY"
                }
                if ($lines[$j] -match '^(private|protected):') { break }
            }
        }

        # Check for non-const reference parameters
        if ($line -match 'void\s+\w+\(.*?(\w+)\s*&\s*(\w+)' -and
            $line -notmatch 'const' -and
            $line -notmatch '\boverride\b|UPARAM\s*\(\s*ref\s*\)|FSubsystemCollectionBase\s*&|\bOut[A-Z]\w*') {
            Add-Issue $FilePath ($i+1) "Warning" "API" "Non-const reference parameter" "Use const& for input, pointer for output"
        }
    }
}

function Check-ErrorHandling {
    param($FilePath, $Content)

    $lines = $Content -split "`n"

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Check for immediate dereference of a cast result
        if ($line -match '\bCast<\w+>\([^\)]*\)\s*->') {
            Add-Issue $FilePath ($i+1) "Warning" "ErrorHandling" "Cast<> result dereferenced without null check" "Store the cast result and validate it before use"
        }

        # Check for missing ensure/check
        if ($line -match '(GetWorld|GetOwner|GetController)\(\)->') {
            Add-Issue $FilePath ($i+1) "Warning" "ErrorHandling" "Chained call without null check" "Use ensure() or check()"
        }

        # Check for empty catch blocks
        if ($line -match 'catch\s*\(') {
            $nextLine = if ($i+1 -lt $lines.Count) { $lines[$i+1].Trim() } else { "" }
            if ($nextLine -eq '{' -and $i+2 -lt $lines.Count -and $lines[$i+2].Trim() -eq '}') {
                Add-Issue $FilePath ($i+1) "Error" "ErrorHandling" "Empty catch block" "Log error or rethrow"
            }
        }
    }
}

function Check-CodeSmells {
    param($FilePath, $Content)

    $lines = $Content -split "`n"

    # Check function length
    $inFunction = $false
    $functionStart = 0
    $braceCount = 0

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        if ($line -match '^\w+.*::\w+\(.*\)') {
            $inFunction = $true
            $functionStart = $i
            $braceCount = 0
        }

        if ($inFunction) {
            $braceCount += ($line.ToCharArray() | Where-Object { $_ -eq '{' }).Count
            $braceCount -= ($line.ToCharArray() | Where-Object { $_ -eq '}' }).Count

            if ($braceCount -eq 0 -and $line -match '}') {
                $functionLength = $i - $functionStart
                if ($functionLength -gt 50) {
                    Add-Issue $FilePath $functionStart "Warning" "Complexity" "Function too long ($functionLength lines)" "Consider refactoring into smaller functions"
                }
                $inFunction = $false
            }
        }

        # Check for magic numbers outside strings/comments/constants.
        $lineForMagicNumberCheck = [regex]::Replace($line, '"(?:\\.|[^"\\])*"', '""')
        $lineForMagicNumberCheck = [regex]::Replace($lineForMagicNumberCheck, "'(?:\\.|[^'\\])+'", "''")
        $lineForMagicNumberCheck = [regex]::Replace($lineForMagicNumberCheck, '//.*$', '')
        if ($lineForMagicNumberCheck -match '\b(\d{2,})\b' -and $lineForMagicNumberCheck -notmatch '\bconst\b|constexpr|UPROPERTY|UMETA') {
            $number = $Matches[1]
            if ($number -ne '10' -and $number -ne '100' -and $number -ne '1000') {
                Add-Issue $FilePath ($i+1) "Warning" "Maintainability" "Magic number: $number" "Use named constant"
            }
        }

        # Check for technical debt markers
        $DebtMarkerPattern = '(TO' + 'DO|FIX' + 'ME|HA' + 'CK|X' + 'XX)'
        if ($line -match $DebtMarkerPattern) {
            Add-Issue $FilePath ($i+1) "Info" "TechnicalDebt" "Technical debt marker found" "Address before release"
        }
    }
}

# Main execution
Write-Host "Starting static analysis..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object {
        $_.FullName -notmatch '\\Intermediate\\' -and
        $_.FullName -notmatch '\\Tests?\\' -and
        $_.Name -notmatch '(^Test.*|.*Tests)\.(cpp|h)$'
    }

foreach ($file in $files) {
    Write-Host "Analyzing: $($file.Name)" -ForegroundColor Gray
    $content = Get-Content $file.FullName -Raw

    Check-MemoryManagement $file.FullName $content
    Check-ThreadSafety $file.FullName $content
    Check-PerformanceIssues $file.FullName $content
    Check-APIDesign $file.FullName $content
    Check-ErrorHandling $file.FullName $content
    Check-CodeSmells $file.FullName $content
}

# Generate report
$errorCount = ($Issues | Where-Object { $_.Severity -eq "Error" }).Count
$warningCount = ($Issues | Where-Object { $_.Severity -eq "Warning" }).Count
$infoCount = ($Issues | Where-Object { $_.Severity -eq "Info" }).Count

$report = @"
# Static Analysis Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Summary
- Files Analyzed: $($files.Count)
- Errors: $errorCount
- Warnings: $warningCount
- Info: $infoCount
- Total Issues: $($Issues.Count)

## Issues by Category
"@

$byCategory = $Issues | Group-Object Category | Sort-Object Count -Descending
foreach ($group in $byCategory) {
    $report += "`n### $($group.Name) ($($group.Count) issues)`n"
    $report += "| Severity | File | Line | Issue | Recommendation |`n"
    $report += "|----------|------|------|-------|----------------|`n"
    foreach ($issue in $group.Group | Select-Object -First 20) {
        $shortFile = Split-Path $issue.File -Leaf
        $report += "| $($issue.Severity) | $shortFile | $($issue.Line) | $($issue.Message) | $($issue.Recommendation) |`n"
    }
    if ($group.Count -gt 20) {
        $report += "| ... | ... | ... | and $($group.Count - 20) more | ... |`n"
    }
}

$report += "`n## Critical Issues (Errors)`n"
$criticalIssues = $Issues | Where-Object { $_.Severity -eq "Error" }
if ($criticalIssues.Count -gt 0) {
    foreach ($issue in $criticalIssues) {
        $report += "- **[$($issue.Category)]** $($issue.File):$($issue.Line) - $($issue.Message)`n"
        $report += "  - Recommendation: $($issue.Recommendation)`n"
    }
} else {
    $report += "No critical issues found.`n"
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nStatic analysis complete!" -ForegroundColor Green
Write-Host "Errors: $errorCount, Warnings: $warningCount, Info: $infoCount" -ForegroundColor $(if($errorCount -gt 0){"Red"}elseif($warningCount -gt 0){"Yellow"}else{"Green"})
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
