param([string]$OutputPath = "")
. (Join-Path $PSScriptRoot "ReleaseCommon.ps1")
Invoke-HorrorReleaseReport -ReportName "PressKitGenerator" -OutputPath $OutputPath
