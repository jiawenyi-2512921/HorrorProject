param([string]$OutputPath = "", [switch]$Continuous)
$Common = Join-Path (Split-Path -Parent $PSScriptRoot) "MonitoringCommon.ps1"
. $Common
Invoke-HorrorMonitoringTask -TaskName "RealtimeMonitor" -OutputPath $OutputPath -Continuous:$Continuous
