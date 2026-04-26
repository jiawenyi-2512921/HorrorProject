param([string]$OutputPath = "", [switch]$Continuous)
. (Join-Path $PSScriptRoot "MonitoringCommon.ps1")
Invoke-HorrorMonitoringTask -TaskName "StartMonitoring" -OutputPath $OutputPath -Continuous:$Continuous
