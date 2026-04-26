# Analytics Architecture

## Overview

The Horror Project Analytics System is a comprehensive, GDPR/CCPA-compliant analytics and telemetry solution designed to collect, analyze, and visualize game data while respecting user privacy.

## System Components

### 1. Analytics Subsystem (`AnalyticsSubsystem`)

The core analytics system that manages event tracking, user consent, and data collection.

**Key Features:**
- Event tracking with parameters
- User consent management (GDPR/CCPA)
- User ID anonymization
- Data export and deletion
- Session management integration

**Privacy Compliance:**
- Requires explicit user consent before tracking
- Anonymizes user identifiers using MD5 hashing
- Supports data deletion requests
- Exports user data on request
- Consent versioning and timestamping

### 2. Event Tracker (`EventTracker`)

Tracks and categorizes analytics events.

**Features:**
- Event counting and aggregation
- Automatic event categorization
- Top events analysis
- Events per session calculation

**Event Categories:**
- Navigation (screen views)
- Errors (crashes, exceptions)
- Monetization (purchases)
- Gameplay (levels, achievements)
- Social (sharing, multiplayer)
- Configuration (settings)

### 3. Session Tracker (`SessionTracker`)

Manages user session tracking and analysis.

**Features:**
- Session ID generation
- Session duration tracking
- Session history management
- Average session duration calculation
- Platform and version tracking

### 4. Metrics Collector (`MetricsCollector`)

Collects and aggregates performance and gameplay metrics.

**Features:**
- Generic metric recording with tags
- Timing metrics (duration measurements)
- Counter metrics (event counts)
- Statistical analysis (min, max, average)
- Metric export

### 5. Telemetry Subsystem (`TelemetrySubsystem`)

Comprehensive telemetry system integrating performance, gameplay, and error tracking.

**Components:**
- Performance Telemetry
- Gameplay Telemetry
- Error Telemetry

**Features:**
- Periodic telemetry collection
- Report generation
- Data export
- Component integration

### 6. Performance Telemetry (`PerformanceTelemetry`)

Tracks game performance metrics.

**Metrics:**
- FPS (frames per second)
- Frame time (milliseconds)
- Memory usage (MB)
- Performance snapshots
- Min/Max/Average calculations

### 7. Gameplay Telemetry (`GameplayTelemetry`)

Tracks gameplay-specific metrics.

**Metrics:**
- Total playtime
- Levels completed
- Death count and causes
- Achievements unlocked
- Items collected
- Enemies defeated
- Distance traveled

### 8. Error Telemetry (`ErrorTelemetry`)

Tracks errors, exceptions, and crashes.

**Features:**
- Error severity levels (Info, Warning, Error, Critical)
- Stack trace capture
- Error categorization
- Error history
- Error export

## Data Processing Pipeline

### 1. Data Aggregator (`DataAggregator.ps1`)

Aggregates raw analytics data from multiple sources.

**Functions:**
- Event aggregation
- Session aggregation
- Telemetry aggregation
- Data consolidation

**Output:**
- Aggregated JSON reports
- Event counts and distributions
- Session statistics
- Performance summaries

### 2. Data Analyzer (`DataAnalyzer.ps1`)

Analyzes aggregated data to extract insights.

**Analysis Types:**
- Event pattern analysis
- Session behavior analysis
- Performance analysis
- Gameplay analysis

**Insights Generated:**
- Performance ratings
- Engagement metrics
- Difficulty assessment
- Optimization recommendations

### 3. Report Generator (`ReportGenerator.ps1`)

Generates comprehensive analytics reports.

**Report Formats:**
- HTML (interactive dashboard)
- Markdown (documentation)
- JSON (data export)

**Report Sections:**
- Performance overview
- Session behavior
- Gameplay metrics
- Top events
- Key insights

### 4. Dashboard Updater (`DashboardUpdater.ps1`)

Creates and updates real-time analytics dashboard.

**Features:**
- Interactive HTML dashboard
- Real-time metrics display
- Auto-refresh capability
- Visual status indicators
- Responsive design

## Data Flow

```
Game Events → Analytics Subsystem → Event Queue → Flush to Disk
                                                      ↓
                                              events.jsonl
                                                      ↓
                                            Data Aggregator
                                                      ↓
                                          Aggregated Reports
                                                      ↓
                                             Data Analyzer
                                                      ↓
                                            Analysis Reports
                                                      ↓
                                           Report Generator
                                                      ↓
                                        HTML/Markdown Reports
                                                      ↓
                                          Dashboard Updater
                                                      ↓
                                        Interactive Dashboard
```

## Privacy & Compliance

### GDPR Compliance

1. **Consent Management**
   - Explicit opt-in required
   - Granular consent options
   - Consent versioning
   - Consent withdrawal

2. **Data Minimization**
   - Only collect necessary data
   - Anonymize personal identifiers
   - No PII collection

3. **Right to Access**
   - Export user data on request
   - JSON format for portability

4. **Right to Erasure**
   - Complete data deletion
   - Consent revocation
   - Local data cleanup

### CCPA Compliance

1. **Transparency**
   - Clear privacy policy
   - Data usage disclosure

2. **Opt-Out Rights**
   - Easy consent withdrawal
   - Personalized ads opt-out

3. **Data Sale Prohibition**
   - No data selling
   - No third-party sharing

## File Storage

### Directory Structure

```
Saved/Analytics/
├── events.jsonl              # Event log (JSONL format)
├── sessions.json             # Session history
├── user_consent.json         # User consent data
├── Telemetry/
│   └── report_*.json         # Telemetry reports
├── Aggregated/
│   └── aggregated_*.json     # Aggregated data
├── Analysis/
│   └── analysis_*.json       # Analysis results
├── Reports/
│   ├── report_*.html         # HTML reports
│   ├── report_*.md           # Markdown reports
│   └── report_*.json         # JSON reports
└── Dashboard/
    └── index.html            # Interactive dashboard
```

## Integration Guide

### 1. Enable Analytics

```cpp
UAnalyticsSubsystem* Analytics = GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();

FUserConsent Consent;
Consent.bAnalyticsEnabled = true;
Consent.bCrashReportingEnabled = true;
Analytics->SetUserConsent(Consent);
```

### 2. Track Events

```cpp
TMap<FString, FString> Parameters;
Parameters.Add(TEXT("level_name"), TEXT("Level_1"));
Parameters.Add(TEXT("completion_time"), FString::Printf(TEXT("%.2f"), CompletionTime));

Analytics->TrackEvent(TEXT("level_complete"), Parameters);
```

### 3. Track Performance

```cpp
UTelemetrySubsystem* Telemetry = GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
UPerformanceTelemetry* PerfTelemetry = Telemetry->GetPerformanceTelemetry();

// Performance is tracked automatically via Tick
float AvgFPS = PerfTelemetry->GetAverageFPS();
```

### 4. Track Gameplay

```cpp
UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();

GameplayTelemetry->RecordLevelCompleted(TEXT("Level_1"));
GameplayTelemetry->RecordDeath(TEXT("Enemy"));
GameplayTelemetry->RecordAchievement(TEXT("ACH_FIRST_LEVEL"));
```

### 5. Generate Reports

```powershell
# Aggregate data
.\DataAggregator.ps1 -IncludeTelemetry

# Analyze data
.\DataAnalyzer.ps1

# Generate report
.\ReportGenerator.ps1 -Format HTML

# Update dashboard
.\DashboardUpdater.ps1 -AutoRefresh
```

## Performance Considerations

- Events are queued and flushed periodically (30s intervals)
- Maximum 1000 events per queue
- Maximum 1000 metrics per metric name
- Maximum 500 error reports
- Automatic data pruning to prevent memory issues

## Security

- User IDs are anonymized using MD5 hashing
- No sensitive data collection
- Local data storage only
- No network transmission (can be extended)
- Secure file permissions

## Extensibility

The system is designed for easy extension:

1. **Custom Events**: Add new event types with custom parameters
2. **Custom Metrics**: Define nerics with tags
3. **Custom Telemetry**: Extend telemetry components
4. **Custom Reports**: Create new report formats
5. **Backend Integration**: Add network transmission for centralized analytics

## Best Practices

1. Always check user consent before tracking
2. Use descriptive event names
3. Include relevant parameters
4. Track errors with context
5. Generate reports regularly
6. Monitor dashboard for issues
7. Respect user privacy
8. Document custom events
9. Test analytics in development
10. Review privacy policy regularly
