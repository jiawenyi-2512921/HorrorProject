# 📊 HorrorProject Monitoring System

Real-time monitoring and decision support system for autonomous development.

## 🎯 Overview

This monitoring system provides real-time visibility into project status, enabling data-driven decision making and autonomous agent coordination.

## 📁 Components

### Core Dashboard Files (ContextVault)
- **DASHBOARD.md** - Real-time project status overview
- **DECISION_MATRIX.md** - Autonomous decision-making rules
- **PROGRESS_TRACKER.md** - Hour-by-hour progress analysis
- **ALERTS.md** - Real-time alert system
- **TIMELINE.md** - Visual development timeline (generated)

### Monitoring Scripts
- **RunMonitoring.ps1** - Master script (runs all monitoring)
- **UpdateDashboard.ps1** - Updates dashboard with latest data
- **CheckAgentStatus.ps1** - Monitors agent health and status
- **CollectMetrics.ps1** - Collects comprehensive metrics
- **GenerateTimeline.ps1** - Generates visual timeline

## 🚀 Quick Start

### Single Update
```powershell
.\RunMonitoring.ps1
```

### Continuous Monitoring (Every 5 minutes)
```powershell
.\RunMonitoring.ps1 -Continuous
```

### Custom Interval (Every 10 minutes)
```powershell
.\RunMonitoring.ps1 -Continuous -IntervalMinutes 10
```

## 📊 Individual Scripts

### Update Dashboard
```powershell
.\UpdateDashboard.ps1
```
Updates DASHBOARD.md with current metrics.

### Check Agent Status
```powershell
.\CheckAgentStatus.ps1
.\CheckAgentStatus.ps1 -Detailed  # More verbose output
```
Monitors agent execution and detects issues.

### Collect Metrics
```powershell
.\CollectMetrics.ps1
.\CollectMetrics.ps1 -Export  # Export to JSON
```
Gathers comprehensive project metrics.

### Generate Timeline
```powershell
.\GenerateTimeline.ps1
.\GenerateTimeline.ps1 -Detailed  # More detailed timeline
```
Creates visual development timeline.

## 📈 Metrics Tracked

### Project Metrics
- Total files and size
- Code statistics (C++, headers)
- Lines of code
- Asset counts

### Git Metrics
- Commit history
- Branch status
- Uncommitted changes
- Agent completions

### Performance Metrics
- CPU usage
- Memory usage
- Disk space
- Build times

### Quality Metrics
- Build status
- Code quality score
- Issue count
- Health score

### Velocity Metrics
- Agents completed
- Completion rate
- Estimated time remaining
- Progress percentage

## 🚨 Alert System

### Alert Levels
- 🔴 **CRITICAL** - System-breaking issues (< 5min response)
- 🟠 **HIGH** - Major blocking issues (< 30min response)
- 🟡 **MEDIUM** - Quality/velocity issues (< 2h response)
- 🟢 **LOW** - Minor improvements (< 24h response)
- 🔵 **INFO** - Status updates

### Alert Triggers
- Build failures
- Performance regressions
- Agent failures
- Resource exhaustion
- Git conflicts

## 🎯 Decision Support

The DECISION_MATRIX.md provides:
- Automated decision rules
- Priority matrices
- Resource allocation strategies
- Risk response procedures
- Velocity optimization

## 🔄 Automation

### Scheduled Monitoring
Create a Windows Task Scheduler task:
```powershell
$action = New-ScheduledTaskAction -Execute "PowerShell.exe" -Argument "-File D:\gptzuo\HorrorProject\HorrorProject\Scripts\Monitoring\RunMonitoring.ps1"
$trigger = New-ScheduledTaskTrigger -Once -At (Get-Date) -RepetitionInterval (New-TimeSpan -Minutes 5)
Register-ScheduledTask -TaskName "HorrorProject-Monitoring" -Action $action -Trigger $trigger
```

### Manual Scheduling
Run in background:
```powershell
Start-Job -FilePath .\RunMonitoring.ps1 -ArgumentList @{Continuous=$true}
```

## 📊 Dashboard Access

View real-time status:
- **Main Dashboard:** `D:\gptzuo\ContextVault\DASHBOARD.md`
- **Progress Tracker:** `D:\gptzuo\ContextVault\PROGRESS_TRACKER.md`
- **Alerts:** `D:\gptzuo\ContextVault\ALERTS.md`
- **Timeline:** `D:\gptzuo\ContextVault\TIMELINE.md`

## 🎯 Use Cases

### For Mastermind Agent
- Monitor overall project health
- Make autonomous decisions
- Prioritize agent execution
- Detect and respond to issues

### For Development Team
- Track progress in real-time
- Identify bottlenecks
- Monitor resource usage
- Review velocity trends

### For Stakeholders
- View project status
- Check milestone progress
- Review timeline
- Assess risks

## 🔧 Configuration

### Update Frequency
Edit `RunMonitoring.ps1`:
```powershell
$IntervalMinutes = 5  # Change to desired interval
```

### Metrics Export
Enable JSON export in `CollectMetrics.ps1`:
```powershell
.\CollectMetrics.ps1 -Export -OutputPath "custom-path.json"
```

## 📝 Maintenance

### Log Rotation
Metrics are stored in memory and updated in place. No log rotation needed.

### Performance Impact
- CPU: < 5% during monitoring cycle
- Memory: < 100MB
- Disk I/O: Minimal (only file updates)
- Duration: ~5-10 seconds per cycle

## 🎯 Best Practices

1. **Run continuously** during active development
2. **Review dashboard** before starting new agents
3. **Check alerts** regularly for issues
4. **Monitor velocity** to stay on schedule
5. **Use decision matrix** for autonomous operation

## 🔮 Future Enhancements

- [ ] Web-based dashboard UI
- [ ] Real-time FPS monitoring
- [ ] Automated performance profiling
- [ ] Predictive analytics
- [ ] Slack/Discord notifications
- [ ] Historical trend analysis
- [ ] Custom alert rules
- [ ] Multi-project support

## 📞 Support

For issues or questions:
- Check ALERTS.md for current issues
- Review DECISION_MATRIX.md for guidance
- Consult DASHBOARD.md for status

---

**Version:** 1.0.0
**Created:** 2026-04-26
**Maintained by:** ContextVault Agent 98 (Dashboard_Monitor)
