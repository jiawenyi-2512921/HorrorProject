# Shared release documentation report helper.

function Get-HorrorProjectRootFromReleaseDocs {
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

function Invoke-HorrorReleaseReport {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ReportName,

        [string]$OutputPath = ""
    )

    $ProjectRoot = Get-HorrorProjectRootFromReleaseDocs
    $ReleaseRoot = Join-Path $ProjectRoot "Saved\Release"
    New-Item -ItemType Directory -Force -Path $ReleaseRoot | Out-Null

    if ([string]::IsNullOrWhiteSpace($OutputPath)) {
        $OutputPath = Join-Path $ReleaseRoot "$ReportName.md"
    }

    $Maps = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Content") -File -Recurse -Include *.umap -ErrorAction SilentlyContinue)
    $Assets = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Content") -File -Recurse -Include *.uasset,*.umap -ErrorAction SilentlyContinue)

    $Content = @"
# $ReportName

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Project Snapshot

- Maps: $($Maps.Count)
- Assets: $($Assets.Count)
- Project root: $ProjectRoot

## Release Notes

- Build validation should be run with `Scripts\Validation\ValidateCompilation.ps1`.
- Editor validation should be run with `Scripts\Validation\ValidateCompilation.ps1 -EditorOnly`.
- Final integration checks should be run with `Scripts\FinalIntegration\QualityGateValidator.ps1`.
"@

    Set-Content -Path $OutputPath -Value $Content -Encoding UTF8
    Write-Host "Generated $ReportName report: $OutputPath" -ForegroundColor Green
}
