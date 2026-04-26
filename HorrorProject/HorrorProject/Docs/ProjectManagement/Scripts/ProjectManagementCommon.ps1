# Shared project-management report helper.

function Get-HorrorProjectRootFromDocs {
    $Current = $PSScriptRoot
    while ($Current) {
        if (Test-Path -LiteralPath (Join-Path $Current "HorrorProject.uproject")) {
            return $Current
        }
        $Parent = Split-Path -Parent $Current
        if ($Parent -eq $Current) {
            break
        }
        $Current = $Parent
    }
    throw "Unable to locate HorrorProject.uproject."
}

function Invoke-HorrorProjectManagementReport {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ReportName,

        [string]$OutputPath = ""
    )

    $ProjectRoot = Get-HorrorProjectRootFromDocs
    $ReportRoot = Join-Path $ProjectRoot "Saved\ProjectManagement"
    New-Item -ItemType Directory -Force -Path $ReportRoot | Out-Null

    if ([string]::IsNullOrWhiteSpace($OutputPath)) {
        $OutputPath = Join-Path $ReportRoot "$ReportName.json"
    }

    $SourceFiles = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Source") -File -Recurse -Include *.h,*.cpp,*.cs -ErrorAction SilentlyContinue)
    $ScriptFiles = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Scripts") -File -Recurse -Include *.ps1 -ErrorAction SilentlyContinue)
    $DocsFiles = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Docs") -File -Recurse -ErrorAction SilentlyContinue)

    $GitStatus = ""
    try {
        $GitStatus = (git -C $ProjectRoot status --short) -join "`n"
    } catch {
        $GitStatus = "git status unavailable"
    }

    $Payload = [ordered]@{
        Report = $ReportName
        GeneratedAt = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        SourceFileCount = $SourceFiles.Count
        ScriptFileCount = $ScriptFiles.Count
        DocsFileCount = $DocsFiles.Count
        GitStatusLineCount = @($GitStatus -split "`n" | Where-Object { $_ }).Count
        Recommendation = "Use this generated snapshot as an input for planning; do not treat it as a shipped-release approval."
    }

    $Payload | ConvertTo-Json -Depth 4 | Set-Content -Path $OutputPath -Encoding UTF8
    Write-Host "Generated $ReportName report: $OutputPath" -ForegroundColor Green
}
