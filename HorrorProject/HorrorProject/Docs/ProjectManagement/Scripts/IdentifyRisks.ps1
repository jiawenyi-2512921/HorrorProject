param([string]$OutputPath = "")
. (Join-Path $PSScriptRoot "ProjectManagementCommon.ps1")
Invoke-HorrorProjectManagementReport -ReportName "IdentifyRisks" -OutputPath $OutputPath
