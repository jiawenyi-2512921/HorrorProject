param([string]$OutputPath = "")
. (Join-Path $PSScriptRoot "ProjectManagementCommon.ps1")
Invoke-HorrorProjectManagementReport -ReportName "CalculateVelocity" -OutputPath $OutputPath
