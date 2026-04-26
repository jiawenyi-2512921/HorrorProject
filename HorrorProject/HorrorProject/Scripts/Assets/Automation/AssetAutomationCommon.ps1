# Shared safe asset automation helpers.

$ValidationCommon = Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "Validation\Common.ps1"
. $ValidationCommon

function Invoke-HorrorAssetAutomation {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Operation,

        [string]$SourcePath = "",
        [string]$DestinationPath = "",
        [string]$OutputPath = "",
        [string]$LogPath = "",
        [switch]$DryRun
    )

    $ProjectRoot = Get-HorrorProjectRoot
    $ContentRoot = Join-Path $ProjectRoot "Content"
    $SavedRoot = Join-Path $ProjectRoot "Saved\AssetAutomation"
    New-Item -ItemType Directory -Force -Path $SavedRoot | Out-Null

    if ([string]::IsNullOrWhiteSpace($OutputPath)) {
        $OutputPath = Join-Path $SavedRoot "$Operation.json"
    }

    if ([string]::IsNullOrWhiteSpace($LogPath)) {
        $LogPath = Join-Path $SavedRoot "$Operation.log"
    }

    $Assets = @(Get-ChildItem -LiteralPath $ContentRoot -File -Recurse -Include *.uasset,*.umap -ErrorAction SilentlyContinue)
    $ZeroByteAssets = @($Assets | Where-Object { $_.Length -le 0 })
    $Maps = @($Assets | Where-Object { $_.Extension -eq ".umap" })

    $Report = [ordered]@{
        Operation = $Operation
        GeneratedAt = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        SourcePath = $SourcePath
        DestinationPath = $DestinationPath
        DryRun = [bool]$DryRun
        AssetCount = $Assets.Count
        MapCount = $Maps.Count
        ZeroByteAssetCount = $ZeroByteAssets.Count
        Status = if ($ZeroByteAssets.Count -eq 0) { "Ready" } else { "NeedsAttention" }
        Note = "Safe static automation wrapper. Destructive asset edits are intentionally not performed by this script."
    }

    $Report | ConvertTo-Json -Depth 5 | Set-Content -Path $OutputPath -Encoding UTF8
    "[$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')] $Operation completed with status $($Report.Status)" | Add-Content -Path $LogPath

    Write-Host "=== $Operation ===" -ForegroundColor Cyan
    Write-Host "Assets: $($Report.AssetCount)" -ForegroundColor Gray
    Write-Host "Maps: $($Report.MapCount)" -ForegroundColor Gray
    Write-Host "Zero-byte assets: $($Report.ZeroByteAssetCount)" -ForegroundColor $(if ($Report.ZeroByteAssetCount -eq 0) { "Green" } else { "Red" })
    Write-Host "Report: $OutputPath" -ForegroundColor Green

    if ($Report.ZeroByteAssetCount -gt 0) {
        exit 1
    }
}
