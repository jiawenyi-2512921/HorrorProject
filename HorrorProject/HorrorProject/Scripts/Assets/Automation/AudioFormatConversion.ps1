param([string]$SourcePath = "", [string]$DestinationPath = "", [string]$OutputPath = "", [string]$LogPath = "", [switch]$DryRun)
. (Join-Path $PSScriptRoot "AssetAutomationCommon.ps1")
Invoke-HorrorAssetAutomation -Operation "AudioFormatConversion" -SourcePath $SourcePath -DestinationPath $DestinationPath -OutputPath $OutputPath -LogPath $LogPath -DryRun:$DryRun
