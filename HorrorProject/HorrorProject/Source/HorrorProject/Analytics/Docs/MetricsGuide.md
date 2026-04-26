# Metrics Guide

## Overview

This guide provides comprehensive information about metrics collection, analysis, and interpretation in the Horror Project Analytics System.

## Metric Types

### 1. Performance Metrics

#### FPS (Frames Per Second)

**Description:** Measures rendering performance.

**Collection:**
```cpp
UPerformanceTelemetry* PerfTelemetry = Telemetry->GetPerformanceTelemetry();
float AvgFPS = PerfTelemetry->GetAverageFPS();
float MinFPS = PerfTelemetry->GetMinFPS();
float MaxFPS = PerfTelemetry->GetMaxFPS();
```

**Interpretation:**
- **Excellent:** ≥60 FPS
- **Good:** 45-59 FPS
- **Fair:** 30-44 FPS
- **Poor:** <30 FPS

**Optimization Targets:**
- Target: 60 FPS average
- Minimum acceptable: 30 FPS
- Critical threshold: <20 FPS

#### Frame Time

**Description:** Time taken to render a single frame (milliseconds).

**Collection:**
```cpp
float AvgFrameTime = PerfTelemetry->GetAverageFrameTime();
```

**Interpretation:**
- **Excellent:** ≤16.67ms (60 FPS)
- **Good:** 16.67-22.22ms (45-60 FPS)
- **Fair:** 22.22-33.33ms (30-45 FPS)
- **Poor:** >33.33ms (<30 FPS)

#### Memory Usage

**Description:** Physical memory used by the application (MB).

**Collection:**
```cpp
float MemoryUsageMB = PerfTelemetry->GetMemoryUsageMB();
```

**Interpretation:**
- **Excellent:** <2048 MB
- **Good:** 2048-4096 MB
- **Fair:** 4096-6144 MB
- **Poor:** >6144 MB

**Warning Thresholds:**
- Caution: >4096 MB
- Warning: >6144 MB
- Critical: >8192 MB

### 2. Gameplay Metrics

#### Playtime

**Description:** Total time spent playing (seconds/hours).

**Collection:**
```cpp
UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();
float TotalPlaytime = GameplayTelemetry->GetTotalPlaytime();
```

**Interpretation:**
- High playtime indicates good engagement
- Track playtime per session for retention analysis
- Compare playtime across levels for difficulty balancing

#### Level Completion Rate

**Description:** Percentage of started levels that are completed.

**Calculation:**
```
Completion Rate = (Levels Completed / Levels Started) × 100%
```

**Interpretation:**
- **Excellent:** >80%
- **Good:** 60-80%
- **Fair:** 40-60%
- **Poor:** <40%

**Action Items:**
- <40%: Level may be too difficult or confusing
- >90%: Level may be too easy

#### Death Rate

**Description:** Number of deaths per hour of gameplay.

**Collection:**
```cpp
int32 Deaths = GameplayTelemetry->GetDeathCount();
float Playtime = GameplayTelemetry->GetTotalPlaytime() / 3600.0f; // Convert to hours
float DeathRate = Deaths / Playtime;
```

**Interpretation:**
- **Easy:** <1 death/hour
- **Moderate:** 1-3 deaths/hour
- **Hard:** 3-5 deaths/hour
- **Very Hard:** >5 deaths/hour

#### Achievement Unlock Rate

**Description:** Percentage of available achievements unlocked.

**Calculation:**
```
Unlock Rate = (Achievements Unlocked / Total Achievements) × 100%
```

**Interpretation:**
- Track which achievements are rarely unlocked
- Identify achievements that may be too difficult
- Balance achievement difficulty distribution

### 3. Session Metrics

#### Session Duration

**Description:** Length of individual play sessions (seconds).

**Collection:**
```cpp
USessionTracker* SessionTracker = Analytics->GetSessionTracker();
float SessionDuration = SessionTracker->GetSessionDuration();
float AvgSessionDuration = SessionTracker->GetAverageSessionDuration();
```

**Interpretation:**
- **Short:** <5 minutes (potential retention issue)
- **Medium:** 5-30 minutes (typical session)
- **Long:** >30 minutes (high engagement)

**Targets:**
- Mobile: 5-15 minutes average
- PC/Console: 30-60 minutes average

#### Session Frequency

**Description:** Number of sessions per user per time period.

**Interpretation:**
- Daily active users (DAU)
- Weekly active users (WAU)
- Monthly active users (MAU)

**Engagement Metrics:**
- DAU/MAU ratio: Indicates stickiness
- Target: >20% for high engagement

### 4. Custom Metrics

#### Recording Custom Metrics

```cpp
UMetricsCollector* Collector = Analytics->GetMetricsCollector();

// Generic metric
TMap<FString, FString> Tags;
Tags.Add(TEXT("level"), TEXT("Level_1"));
Collector->RecordMetric(TEXT("puzzle_solve_time"), 45.5f, Tags);

// Timing metric
Collector->RecordTimingMetric(TEXT("load_time"), 1250.0f);

// Counter metric
Collector->RecordCounterMetric(TEXT("button_clicks"), 1);
```

#### Metric Statistics

```cpp
FMetricStats Stats = Collector->GetMetricStats(TEXT("puzzle_solve_time"));

UE_LOG(LogTemp, Log, TEXT("Min: %.2f"), Stats.Min);
UE_LOG(LogTemp, Log, TEXT("Max: %.2f"), Stats.Max);
UE_LOG(LogTemp, Log, TEXT("Average: %.2f"), Stats.Average);
UE_LOG(LogTemp, Log, TEXT("Total: %.2f"), Stats.Total);
UE_LOG(LogTemp, Log, TEXT("Count: %d"), Stats.Count);
```

## Metric Collection Best Practices

### 1. Sampling Strategy

**High-Frequency Metrics** (every frame):
- FPS
- Frame time
- Memory usage

**Medium-Frequency Metrics** (every second):
- Player position
- Health/status
- Active enemies

**Low-Frequency Metrics** (on event):
- Level completion
- Deaths
- Achievements

### 2. Aggregation

**Time-Based Aggregation:**
```cpp
// Collect samples over time
for (int32 i = 0; i < SampleCount; i++)
{
    Collector->RecordMetric(TEXT("fps"), CurrentFPS);
}

// Get aggregated statistics
FMetricStats Stats = Collector->GetMetricStats(TEXT("fps"));
```

**Session-Based Aggregation:**
```cpp
// Track per-session metrics
SessionMetrics.Add(TEXT("total_deaths"), Deaths);
SessionMetrics.Add(TEXT("total_kills"), Kills);
SessionMetrics.Add(TEXT("completion_time"), CompletionTime);
```

### 3. Metric Tagging

Use tags to add context to metrics:

```cpp
TMap<FString, FString> Tags;
Tags.Add(TEXT("level"), CurrentLevel);
Tags.Add(TEXT("difficulty"), DifficultyLevel);
Tags.Add(TEXT("platform"), PlatformName);

Collector->RecordMetric(TEXT("enemy_defeat_time"), DefeatTime, Tags);
```

## Metric Analysis

### 1. Trend Analysis

Track metrics over time to identify trends:

```cpp
TArray<FPerformanceSnapshot> History = PerfTelemetry->GetPerformanceHistory();

// Analyze FPS trend
float TrendSlope = CalculateTrend(History);
if (TrendSlope < -0.1f)
{
    UE_LOG(LogTemp, Warning, TEXT("Performance degrading over time"));
}
```

### 2. Percentile Analysis

Understand metric distribution:

```
P50 (Median): 50% of values are below this
P90: 90% of values are below this
P95: 95% of values are below this
P99: 99% of values are below this
```

**Example:**
- P50 FPS: 58 (typical performance)
- P95 FPS: 45 (worst 5% of frames)
- P99 FPS: 30 (worst 1% of frames)

### 3. Correlation Analysis

Identify relationships between metrics:

```
High memory usage → Low FPS
Long session duration → High achievement rate
High death rate → Low completion rate
```

## Performance Scoring

### Overall Performance Score

Calculated from multiple metrics:

```cpp
float FPSScore = CalculateFPSScore(AvgFPS);      // 0-100
float MemoryScore = CalculateMemoryScore(MemoryUsage); // 0-100

float PerformanceScore = (FPSScore + MemoryScore) / 2.0f;
```

**Score Interpretation:**
- **90-100:** Excellent
- **75-89:** Good
- **60-74:** Fair
- **<60:** Poor

### Engagement Score

Calculated from gameplay metrics:

```cpp
float SessionScore = CalculateSessionScore(AvgSessionDuration);
float RetentionScore = CalculateRetentionScore(SessionFrequency);
float ProgressionScore = CalculateProgressionScore(CompletionRate);

float EngagementScore = (SessionScore + RetentionScore + ProgressionScore) / 3.0f;
```

## Metric Visualization

### Dashboard Metrics

Key metrics to display on dashboard:

1. **Performance**
   - Current FPS
   - Average FPS (last hour)
   - Memory usage
   - Performance score

2. **Engagement**
   - Active sessions
   - Average session duration
   - Total playtime
   - DAU/MAU ratio

3. **Gameplay**
   - Levels completed
   - Death rate
   - Achievement unlock rate
   - Progression rate

### Charts and Graphs

**Line Charts:**
- FPS over time
- Memory usage over time
- Session duration trend

**Bar Charts:**
- Top events
- Level completion rates
- Death causes distribution

**Pie Charts:**
- Session duration distribution
- Platform distribution
- Difficulty selection

## Metric Alerts

### Setting Up Alerts

```cpp
// Performance alert
if (CurrentFPS < 30.0f)
{
    ErrorTelemetry->ReportError(
        TEXT("Low FPS detected"),
        TEXT("PERF_LOW_FPS"),
        EErrorSeverity::Warning
    );
}

// Memory alert
if (MemoryUsageMB > 6144.0f)
{
    ErrorTelemetry->ReportError(
        TEXT("High memory usage"),
        TEXT("PERF_HIGH_MEMORY"),
        EErrorSeverity::Warning
    );
}
```

### Alert Thresholds

| Metric | Warning | Critical |
|--------|---------|----------|
| FPS | <45 | <30 |
| Frame Time | >22ms | >33ms |
| Memory | >4GB | >6GB |
| Crash Rate | >1% | >5% |
| Error Rate | >5% | >10% |

## Metric Export

### Export to JSON

```cpp
FString ExportPath = FPaths::ProjectSavedDir() / TEXT("Analytics/metrics_export.json");
Collector->ExportMetrics(ExportPath);
```

### Export Format

```json
{
  "metrics": [
    {
      "metric_name": "fps",
      "value": 58.5,
      "timestamp": "2026-04-26T10:30:00Z",
      "tags": {
        "level": "Level_1",
        "platform": "Windows"
      }
    }
  ],
  "stats": {
    "fps": {
      "min": 45.2,
      "max": 60.0,
      "average": 58.5,
      "total": 5850.0,
      "count": 100
    }
  }
}
```

## Advanced Metrics

### Cohort Analysis

Track metrics for user cohorts:

```cpp
// Define cohort
FString CohortId = TEXT("Week_2026_04_20");

// Track cohort metrics
TMap<FString, FString> Tags;
Tags.Add(TEXT("cohort"), CohortId);
Collector->RecordMetric(TEXT("retention_day_1"), RetentionRate, Tags);
```

### Funnel Metrics

Track conversion through funnels:

```cpp
// Tutorial funnel
TrackEvent(TEXT("tutorial_started"));      // 100%
TrackEvent(TEXT("tutorial_step_1"));       // 95%
TrackEvent(TEXT("tutorial_step_2"));       // 85%
TrackEvent(TEXT("tutorial_completed"));    // 75%

// Calculate drop-off rates
float DropOffRate = (StartCount - CompleteCount) / StartCount * 100.0f;
```

### Lifetime Value (LTV)

Track player lifetime value:

```cpp
float TotalRevenue = CalculateTotalRevenue(PlayerId);
float TotalPlaytime = GetTotalPlaytime(PlayerId);
float LTV = TotalRevenue; // Or more complex calculation
```

## Metric Optimization

### Reducing Overhead

1. **Batch Collection:** Collect metrics in batches
2. **Sampling:** Sample high-frequency metrics
3. **Async Processing:** Process metrics asynchronously
4. **Data Pruning:** Remove old metrics automatically

### Memory Management

```cpp
// Automatic pruning in MetricsCollector
static constexpr int32 MaxMetricsPerName = 1000;

void PruneOldMetrics(const FString& MetricName)
{
    TArray<FMetricData>* Metrics = MetricsByName.Find(MetricName);
    if (Metrics && Metrics->Num() > MaxMetricsPerName)
    {
        Metrics->RemoveAt(0, Metrics->Num() - MaxMetricsPerName);
    }
}
```

## Troubleshooting

### Common Issues

**Issue:** Metrics not being collected
- Check user consent is granted
- Verify telemetry is enabled
- Check for errors in logs

**Issue:** Inaccurate FPS metrics
- Ensure Update() is called every frame
- Check for frame time spikes
- Verify delta time calculation

**Issue:** High memory usage
- Check metric history size
- Verify pruning is working
- Review event queue size

## Best Practices Summary

1. ✓ Collect metrics with user consent
2. ✓ Use appropriate sampling rates
3. ✓ Tag metrics with context
4. ✓ Aggregate metrics for analysis
5. ✓ Set up alerts for critical metrics
6. ✓ Export metrics regularly
7. ✓ Visualize metrics on dashboard
8. ✓ Optimize metric collection overhead
9. ✓ Document custom metrics
10. ✓ Review metrics regularly for insights
