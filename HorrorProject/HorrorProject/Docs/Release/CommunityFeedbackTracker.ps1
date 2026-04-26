param([string]$OutputPath = "")
. (Join-Path $PSScriptRoot "ReleaseCommon.ps1")
Invoke-HorrorReleaseReport -ReportName "CommunityFeedbackTracker" -OutputPath $OutputPath
