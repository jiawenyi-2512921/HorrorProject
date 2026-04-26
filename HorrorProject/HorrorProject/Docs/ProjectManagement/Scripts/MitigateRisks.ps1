param([string]$OutputPath = "")
. (Join-Path $PSScriptRoot "ProjectManagementCommon.ps1")
Invoke-HorrorProjectManagementReport -ReportName "MitigateRisks" -OutputPath $OutputPath
