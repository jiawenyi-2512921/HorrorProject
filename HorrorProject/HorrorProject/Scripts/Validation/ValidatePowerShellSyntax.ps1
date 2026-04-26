# Validates PowerShell scripts with the PowerShell parser.

param(
    [string[]]$ScanPath = @("Scripts", "Source", "Docs"),
    [string[]]$ExcludePath = @("Binaries", "DerivedDataCache", "Intermediate", "Saved", ".git", "Docs\Production"),
    [int]$MaxIssues = 100,
    [switch]$NoExitCode
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Issues = New-Object System.Collections.Generic.List[object]

function Test-IsExcludedPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    foreach ($Excluded in $ExcludePath) {
        $NormalizedExcluded = $Excluded.TrimEnd("\", "/")
        if ($RelativePath -eq $NormalizedExcluded -or $RelativePath.StartsWith("$NormalizedExcluded\")) {
            return $true
        }
    }

    return $false
}

Write-Host "=== PowerShell Syntax Validation ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectRoot" -ForegroundColor Yellow

$Files = foreach ($Path in $ScanPath) {
    $AbsolutePath = if ([System.IO.Path]::IsPathRooted($Path)) { $Path } else { Join-Path $ProjectRoot $Path }
    if (Test-Path -LiteralPath $AbsolutePath) {
        Get-ChildItem -LiteralPath $AbsolutePath -Filter "*.ps1" -File -Recurse
    }
}

$Files = @($Files | Sort-Object FullName | Where-Object {
    $RelativePath = $_.FullName.Substring($ProjectRoot.Length + 1)
    -not (Test-IsExcludedPath -RelativePath $RelativePath)
})

foreach ($File in $Files) {
    $Tokens = $null
    $ParseErrors = $null
    [System.Management.Automation.Language.Parser]::ParseFile($File.FullName, [ref]$Tokens, [ref]$ParseErrors) | Out-Null

    foreach ($ParseError in $ParseErrors) {
        $RelativePath = $File.FullName.Substring($ProjectRoot.Length + 1)
        $Issues.Add([PSCustomObject]@{
            Path = $RelativePath
            Line = $ParseError.Extent.StartLineNumber
            Column = $ParseError.Extent.StartColumnNumber
            Message = $ParseError.Message
            Text = ($ParseError.Extent.Text -replace "`r|`n", " ")
        }) | Out-Null
    }
}

Write-Host "Scanned scripts: $($Files.Count)" -ForegroundColor Gray

if ($Issues.Count -eq 0) {
    Write-Host "[OK] No PowerShell syntax errors detected" -ForegroundColor Green
    exit 0
}

Write-Host "[ERROR] Found $($Issues.Count) PowerShell syntax errors" -ForegroundColor Red
Write-Host ""

$Issues |
    Select-Object -First $MaxIssues |
    Format-Table Path, Line, Column, Message -AutoSize -Wrap

if ($Issues.Count -gt $MaxIssues) {
    Write-Host "Showing first $MaxIssues issues. Remaining: $($Issues.Count - $MaxIssues)" -ForegroundColor Yellow
}

$ReportDir = Join-Path $ProjectRoot "Saved\Validation"
New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null
$ReportPath = Join-Path $ReportDir "PowerShellSyntaxIssues.json"
$Issues | ConvertTo-Json -Depth 4 | Set-Content -Path $ReportPath -Encoding UTF8
Write-Host "Report: $ReportPath" -ForegroundColor Yellow

if ($NoExitCode) {
    exit 0
}

exit 1
