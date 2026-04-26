param([string]$OutputPath = "", [switch]$Continuous)
. (Join-Path $PSScriptRoot "MonitoringCommon.ps1")
Invoke-HorrorMonitoringTask -TaskName "RunMonitoring" -OutputPath $OutputPath -Continuous:$Continuous
