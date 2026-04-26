# HorrorProject Monitoring System

Complete real-time monitoring and alerting system for the HorrorProject.

## Features

### Real-time Monitoring
- **Build Status Monitoring** - Track build success/failure, duration, errors, and warnings
- **Test Status Monitoring** - Monitor test results, coverage, and failure rates
- **Performance Monitoring** - Track CPU, memory, disk, and network usage
- **Asset Status Monitoring** - Monitor asset health and validation status

### Alert System
- **Build Failure Alerts** - Immediate notification on build failures
- **Test Failure Alerts** - Alert on test failures and coverage drops
- **Performance Alerts** - Notify on high CPU/memory usage
- **Memory Leak Detection** - Automatic detection and alerting

### Dashboard
- **Real-time HTML Dashboard** - Visual overview of all metrics
- **Auto-refresh** - Updates every 60 seconds
- **Web Server** - Built-in HTTP server on port 8080

### Log Aggregation
- **Multi-source Log Collection** - Aggregate logs from builds, tests, and editor
- **Log Parsing** - Extract key information from logs
- **Log Reports** - Generate daily/weekly log reports

### Trend Analysis
- **Build Trends** - Analyze build success rates and duration trends
- **Test Trends** - Track test pass rates and identify flaky tests
- **Performance Trends** - Monitor CPU/memory trends and detect leaks
- **Comprehensive Reports** - Generate daily/weekly/monthly trend reports

## Quick Start

### Start All Monitors
```powershell
.\StartMonitoring.ps1 -All
```

### Start Dashboard Only
```powershell
.\StartMonitoring.ps1 -Dashboard
```
Then open http://localhost:8080/ in your browser.

### Start Alerts Only
```powershell
.\StartMonitoring.ps1 -Alerts
```

## Individual Components

### Real-time Monitors
```powershell
# Main realtime monitor
.\RealTime\RealtimeMonitor.ps1 -UpdateInterval 60 -EnableAlerts

# Build status monitor
.\RealTime\MonitorBuildStatus.ps1 -CheckInterval 30

# Test status monitor
.\RealTime\MonitorTestStatus.ps1 -CheckInterval 30

# Performance monitor
.\RealTime\MonitorPerformance.ps1 -ProcessName "UnrealEditor" -CheckInterval 10

# Asset status monitor
.\RealTime\MonitorAssetStatus.ps1 -CheckInterval 60
```

## Data Storage

All monitoring data is stored in the `Data` directory:
- `Data/Build/` - Build status data
- `Data/Test/` - Test status data
- `Data/Performance/` - Performance metrics
- `Data/Assets/` - Asset status data
- `Data/Alerts/` - Alert history
- `Data/Aggregated/` - Aggregated log data

## Requirements

- PowerShell 5.1 or later
- Windows 10/11
- Unreal Engine 5.6 (for process monitoring)
