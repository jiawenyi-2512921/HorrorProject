# PerformanceHotspotDetector.ps1 - Performance Analysis
# Identifies potential performance bottlenecks in code

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/Improvement/PerformanceReport.md"
)

$Hotspots = @()

function Analyze-PerformanceIssues {
    param($File, $Content)

    $issues = @()

    # Check for Tick usage
    $tickMatches = [regex]::Matches($Content, '(?m)PrimaryComponentTick\.bCanEverTick\s*=\s*true|void\s+\w+::Tick\(')
    if ($tickMatches.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "Tick Usage"
            Severity = "Medium"
            Description = "Component uses Tick - consider using timers or events"
            Impact = "CPU overhead every frame"
            Suggestion = "Use FTimerManager or event-driven approach"
        }
    }

    # Check for string operations in loops
    $stringInLoop = [regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*FString[^}]*\+')
    if ($stringInLoop.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "String Concatenation in Loop"
            Severity = "High"
            Description = "String concatenation inside loop"
            Impact = "Memory allocations, poor performance"
            Suggestion = "Use TArray and FString::Join() or StringBuilder pattern"
        }
    }

    # Check for TArray operations without Reserve
    $arrayWithoutReserve = [regex]::Matches($Content, 'TArray<[^>]+>\s+\w+;[^}]*\.Add\(')
    if ($arrayWithoutReserve.Count -gt 0) {
        $reserveCheck = [regex]::Matches($Content, '\.Reserve\(')
        if ($reserveCheck.Count -eq 0) {
            $issues += [PSCustomObject]@{
                Type = "TArray Without Reserve"
                Severity = "Medium"
                Description = "TArray used without Reserve() call"
                Impact = "Multiple reallocations during growth"
                Suggestion = "Call Reserve() if size is known in advance"
            }
        }
    }

    # Check for FindByPredicate in loops
    $findInLoop = [regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*\.FindByPredicate\(')
    if ($findInLoop.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "FindByPredicate in Loop"
            Severity = "High"
            Description = "Nested iteration with FindByPredicate"
            Impact = "O(n²) complexity"
            Suggestion = "Use TMap for lookups or cache results"
        }
    }

    # Check for GetWorld() calls in loops
    $getWorldInLoop = [regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*GetWorld\(\)')
    if ($getWorldInLoop.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "GetWorld() in Loop"
            Severity = "Low"
            Description = "Repeated GetWorld() calls"
            Impact = "Unnecessary function calls"
            Suggestion = "Cache GetWorld() result before loop"
        }
    }

    # Check for Cast in loops
    $castInLoop = [regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*Cast<')
    if ($castInLoop.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "Cast in Loop"
            Severity = "Medium"
            Description = "Dynamic cast inside loop"
            Impact = "RTTI overhead per iteration"
            Suggestion = "Filter by type first or use TArray of specific type"
        }
    }

    # Check for NewObject in Tick
    $newObjectInTick = [regex]::Matches($Content, 'void\s+\w+::Tick\([^)]*\)[^{]*\{[^}]*NewObject<')
    if ($newObjectInTick.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "NewObject in Tick"
            Severity = "Critical"
            Description = "Object creation in Tick function"
            Impact = "Memory allocation every frame, GC pressure"
            Suggestion = "Use object pooling or create objects once"
        }
    }

    # Check for SpawnActor in Tick
    $spawnInTick = [regex]::Matches($Content, 'void\s+\w+::Tick\([^)]*\)[^{]*\{[^}]*SpawnActor')
    if ($spawnInTick.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "SpawnActor in Tick"
            Severity = "Critical"
            Description = "Actor spawning in Tick function"
            Impact = "Severe performance impact, GC pressure"
            Suggestion = "Use object pooling pattern"
        }
    }

    # Check for GetComponentByClass usage
    $getComponentByClass = [regex]::Matches($Content, '\.GetComponentByClass\(')
    if ($getComponentByClass.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "GetComponentByClass"
            Severity = "Medium"
            Description = "Using GetComponentByClass (slow lookup)"
            Impact = "Linear search through components"
            Suggestion = "Cache component reference in BeginPlay"
        }
    }

    # Check for GetActorOfClass in Tick
    $getActorInTick = [regex]::Matches($Content, 'void\s+\w+::Tick\([^)]*\)[^{]*\{[^}]*GetActorOfClass')
    if ($getActorInTick.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "GetActorOfClass in Tick"
            Severity = "Critical"
            Description = "World query in Tick function"
            Impact = "Iterates all actors every frame"
            Suggestion = "Cache reference or use event-driven approach"
        }
    }

    # Check for DrawDebugLine without check
    $debugDrawWithoutCheck = [regex]::Matches($Content, 'DrawDebug(?!.*#if)')
    if ($debugDrawWithoutCheck.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "Debug Draw Without Guard"
            Severity = "Low"
            Description = "Debug drawing without preprocessor guard"
            Impact = "Debug code in shipping builds"
            Suggestion = "Wrap in #if !UE_BUILD_SHIPPING or use bDrawDebug flag"
        }
    }

    # Check for FString::Printf in loops
    $printfInLoop = [regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*FString::Printf')
    if ($printfInLoop.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "FString::Printf in Loop"
            Severity = "High"
            Description = "String formatting in loop"
            Impact = "Memory allocations per iteration"
            Suggestion = "Move formatting outside loop or use FStringBuilder"
        }
    }

    # Check for UE_LOG in hot paths
    $logInLoop = [regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*UE_LOG')
    if ($logInLoop.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "UE_LOG in Loop"
            Severity = "Medium"
            Description = "Logging inside loop"
            Impact = "I/O overhead, string formatting"
            Suggestion = "Remove or guard with verbosity check"
        }
    }

    # Check for TArray copy
    $arrayCopy = [regex]::Matches($Content, 'TArray<[^>]+>\s+\w+\s*=\s*\w+;')
    if ($arrayCopy.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "TArray Copy"
            Severity = "Medium"
            Description = "Potential TArray copy by value"
            Impact = "Deep copy of array contents"
            Suggestion = "Use const reference or MoveTemp()"
        }
    }

    # Check for auto without const&
    $autoValue = [regex]::Matches($Content, 'for\s*\(\s*auto\s+\w+\s*:')
    if ($autoValue.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "Range-for Without Reference"
            Severity = "Medium"
            Description = "Range-based for loop copies elements"
            Impact = "Unnecessary copies per iteration"
            Suggestion = "Use 'const auto&' or 'auto&'"
        }
    }

    # Check for FVector operations in loops
    $vectorOpsInLoop = [regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*(FVector::|\.Size\(\)|\.GetSafeNormal\(\))')
    if ($vectorOpsInLoop.Count -gt 0) {
        $issues += [PSCustomObject]@{
            Type = "Vector Operations in Loop"
            Severity = "Low"
            Description = "Expensive vector math in loop"
            Impact = "Sqrt and division operations"
            Suggestion = "Use SizeSquared() when possible, cache normalized vectors"
        }
    }

    return $issues
}

function Calculate-PerformanceScore {
    param($Issues)

    $score = 100
    foreach ($issue in $Issues) {
        switch ($issue.Severity) {
            "Critical" { $score -= 10 }
            "High" { $score -= 5 }
            "Medium" { $score -= 2 }
            "Low" { $score -= 1 }
        }
    }

    return [Math]::Max(0, $score)
}

# Main execution
Write-Host "Performance Hotspot Detection Starting..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object { $_.FullName -notmatch '\\Intermediate\\' -and $_.FullName -notmatch '\\Tests\\' }

Write-Host "Analyzing $($files.Count) files for performance issues..." -ForegroundColor Gray

foreach ($file in $files) {
    Write-Host "Scanning: $($file.Name)" -ForegroundColor Gray
    $content = Get-Content $file.FullName -Raw

    $issues = Analyze-PerformanceIssues $file.Name $content

    if ($issues.Count -gt 0) {
        $perfScore = Calculate-PerformanceScore $issues

        $script:Hotspots += [PSCustomObject]@{
            File = $file.Name
            Issues = $issues
            IssueCount = $issues.Count
            CriticalCount = ($issues | Where-Object { $_.Severity -eq "Critical" }).Count
            HighCount = ($issues | Where-Object { $_.Severity -eq "High" }).Count
            MediumCount = ($issues | Where-Object { $_.Severity -eq "Medium" }).Count
            LowCount = ($issues | Where-Object { $_.Severity -eq "Low" }).Count
            PerformanceScore = $perfScore
        }
    }
}

# Generate report
$totalIssues = ($Hotspots | Measure-Object -Property IssueCount -Sum).Sum
$criticalIssues = ($Hotspots | Measure-Object -Property CriticalCount -Sum).Sum
$highIssues = ($Hotspots | Measure-Object -Property HighCount -Sum).Sum
$mediumIssues = ($Hotspots | Measure-Object -Property MediumCount -Sum).Sum
$lowIssues = ($Hotspots | Measure-Object -Property LowCount -Sum).Sum
$avgScore = if ($Hotspots.Count -gt 0) { [Math]::Round(($Hotspots | Measure-Object -Property PerformanceScore -Average).Average, 1) } else { 100 }

$report = @"
# Performance Hotspot Analysis Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Executive Summary
- **Files Analyzed**: $($files.Count)
- **Files with Issues**: $($Hotspots.Count)
- **Total Performance Issues**: $totalIssues
- **Average Performance Score**: $avgScore/100

## Severity Breakdown

| Severity | Count | Impact | Priority |
|----------|-------|--------|----------|
| Critical | $criticalIssues | Severe frame drops, memory issues | P0 - Fix immediately |
| High | $highIssues | Noticeable performance impact | P1 - Fix this sprint |
| Medium | $mediumIssues | Moderate performance impact | P2 - Fix next sprint |
| Low | $lowIssues | Minor optimization opportunity | P3 - Consider fixing |

## Performance Score Distribution

| Score Range | Count | Status |
|-------------|-------|--------|
| 90-100 | $(($Hotspots | Where-Object { $_.PerformanceScore -ge 90 }).Count) | ✅ Excellent |
| 70-89 | $(($Hotspots | Where-Object { $_.PerformanceScore -ge 70 -and $_.PerformanceScore -lt 90 }).Count) | ⚠️ Good |
| 50-69 | $(($Hotspots | Where-Object { $_.PerformanceScore -ge 50 -and $_.PerformanceScore -lt 70 }).Count) | ⚠️ Needs Work |
| 0-49 | $(($Hotspots | Where-Object { $_.PerformanceScore -lt 50 }).Count) | ❌ Critical |

## Top Performance Hotspots

| File | Score | Critical | High | Medium | Low | Total Issues |
|------|-------|----------|------|--------|-----|--------------|
"@

foreach ($hotspot in ($Hotspots | Sort-Object PerformanceScore | Select-Object -First 20)) {
    $report += "| $($hotspot.File) | $($hotspot.PerformanceScore) | $($hotspot.CriticalCount) | $($hotspot.HighCount) | $($hotspot.MediumCount) | $($hotspot.LowCount) | $($hotspot.IssueCount) |`n"
}

$report += @"

## Detailed Issue Analysis

"@

foreach ($hotspot in ($Hotspots | Sort-Object PerformanceScore | Select-Object -First 10)) {
    $report += @"

### $($hotspot.File) - Performance Score: $($hotspot.PerformanceScore)/100

"@

    foreach ($issue in $hotspot.Issues) {
        $report += @"

#### $($issue.Type) - $($issue.Severity) Severity
**Description**: $($issue.Description)
**Performance Impact**: $($issue.Impact)
**Recommendation**: $($issue.Suggestion)

"@
    }
}

$report += @"

## Common Performance Anti-Patterns

### 1. Tick Function Overuse
**Problem**: Using Tick for everything
**Impact**: CPU overhead every frame (60+ times per second)
**Solution**:
```cpp
// Bad
void UMyComponent::Tick(float DeltaTime) {
    CheckSomething(); // Runs 60 times per second
}

// Good
void UMyComponent::BeginPlay() {
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle, this, &UMyComponent::CheckSomething,
        1.0f, true); // Runs once per second
}
```

### 2. Object Creation in Hot Paths
**Problem**: Creating objects in Tick or loops
**Impact**: Memory allocations, garbage collection pressure
**Solution**:
```cpp
// Bad
void UMyComponent::Tick(float DeltaTime) {
    AActor* NewActor = GetWorld()->SpawnActor<AActor>();
}

// Good - Object Pooling
class UObjectPool {
    TArray<AActor*> AvailableActors;

    AActor* GetActor() {
        if (AvailableActors.Num() > 0) {
            return AvailableActors.Pop();
        }
        return SpawnNewActor();
    }

    void ReturnActor(AActor* Actor) {
        Actor->SetActorHiddenInGame(true);
        AvailableActors.Add(Actor);
    }
};
```

### 3. Inefficient Lookups
**Problem**: Repeated searches and queries
**Impact**: O(n) or O(n²) complexity
**Solution**:
```cpp
// Bad
void UMyComponent::Tick(float DeltaTime) {
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    // Searches world every frame
}

// Good
class UMyComponent {
    UPROPERTY()
    APlayerController* CachedPlayerController;

    void BeginPlay() {
        CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
    }

    void Tick(float DeltaTime) {
        // Use cached reference
    }
};
```

### 4. String Operations in Loops
**Problem**: String concatenation or formatting in loops
**Impact**: Memory allocations per iteration
**Solution**:
```cpp
// Bad
FString Result;
for (const auto& Item : Items) {
    Result += Item.ToString() + TEXT(", ");
}

// Good
TArray<FString> Parts;
Parts.Reserve(Items.Num());
for (const auto& Item : Items) {
    Parts.Add(Item.ToString());
}
FString Result = FString::Join(Parts, TEXT(", "));
```

### 5. Unnecessary Copies
**Problem**: Copying large data structures
**Impact**: Memory and CPU overhead
**Solution**:
```cpp
// Bad
void ProcessArray(TArray<FLargeStruct> Data) {
    // Copies entire array
}

// Good
void ProcessArray(const TArray<FLargeStruct>& Data) {
    // No copy, read-only access
}

// Also Good
void ProcessArray(TArray<FLargeStruct>&& Data) {
    // Move semantics, no copy
}
```

### 6. Range-Based For Without References
**Problem**: Copying elements in range-based for loops
**Impact**: Unnecessary copies per iteration
**Solution**:
```cpp
// Bad
for (auto Item : LargeArray) {
    // Copies each item
}

// Good
for (const auto& Item : LargeArray) {
    // No copy, read-only
}

// Also Good (if modifying)
for (auto& Item : LargeArray) {
    // No copy, can modify
}
```

### 7. Expensive Math in Loops
**Problem**: Sqrt, division, normalization in loops
**Impact**: Expensive floating-point operations
**Solution**:
```cpp
// Bad
for (const auto& Actor : Actors) {
    float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
    if (Distance < Radius) { }
}

// Good
float RadiusSquared = Radius * Radius;
for (const auto& Actor : Actors) {
    float DistanceSquared = FVector::DistSquared(MyLocation, Actor->GetActorLocation());
    if (DistanceSquared < RadiusSquared) { }
}
```

### 8. Dynamic Cast in Loops
**Problem**: RTTI overhead for each cast
**Impact**: Type checking per iteration
**Solution**:
```cpp
// Bad
for (AActor* Actor : AllActors) {
    if (AEnemy* Enemy = Cast<AEnemy>(Actor)) {
        // Process enemy
    }
}

// Good
TArray<AEnemy*> Enemies;
UGameplayStatics::GetAllActorsOfClass(this, AEnemy::StaticClass(), Enemies);
for (AEnemy* Enemy : Enemies) {
    // Already filtered, no cast needed
}
```

## Performance Optimization Checklist

### Hot Path Optimization
- [ ] No object creation in Tick functions
- [ ] No world queries in Tick functions
- [ ] Cache frequently accessed components
- [ ] Use object pooling for spawned actors
- [ ] Minimize Tick usage, prefer timers/events

### Memory Optimization
- [ ] Reserve TArray capacity when size is known
- [ ] Use const references for large parameters
- [ ] Avoid unnecessary copies in loops
- [ ] Use move semantics where appropriate
- [ ] Minimize string operations

### Algorithm Optimization
- [ ] Use TMap for lookups instead of FindByPredicate
- [ ] Use SizeSquared() instead of Size() when possible
- [ ] Cache expensive calculations
- [ ] Avoid nested loops where possible
- [ ] Use appropriate data structures (TSet for uniqueness, TMap for lookups)

### Unreal-Specific Optimization
- [ ] Disable Tick when not needed
- [ ] Use FTimerManager instead of Tick
- [ ] Cache GetWorld() and component references
- [ ] Use TWeakObjectPtr for safe references
- [ ] Implement proper object pooling

### Debug Code
- [ ] Guard debug drawing with preprocessor directives
- [ ] Remove or guard verbose logging
- [ ] Use stat commands for profiling
- [ ] Profile before optimizing

## Profiling Tools

### Unreal Insights
```
UnrealInsights.exe
```
- CPU profiling
- Memory tracking
- Frame analysis
- Asset loading

### Stat Commands
```
stat fps          - Show FPS
stat unit         - Show frame time breakdown
stat game         - Show game thread stats
stat slow         - Show slow operations
stat memory       - Show memory usage
```

### Console Commands
```
t.MaxFPS 60       - Cap frame rate
r.ScreenPercentage 50 - Reduce resolution
stat startfile    - Start stat capture
stat stopfile     - Stop stat capture
```

## Implementation Plan

### Phase 1: Critical Issues (Week 1)
- Fix object creation in Tick
- Fix world queries in hot paths
- Implement object pooling
- Estimated effort: $($criticalIssues * 2)h
- Expected improvement: +2 quality points

### Phase 2: High Priority (Week 2)
- Optimize string operations
- Fix inefficient lookups
- Cache component references
- Estimated effort: $($highIssues * 1)h
- Expected improvement: +1 quality point

### Phase 3: Medium Priority (Week 3)
- Optimize array operations
- Fix unnecessary copies
- Improve algorithm efficiency
- Estimated effort: $($mediumIssues * 0.5)h
- Expected improvement: +0.5 quality points

### Phase 4: Low Priority (Week 4)
- Clean up debug code
- Minor optimizations
- Code cleanup
- Estimated effort: $($lowIssues * 0.25)h
- Expected improvement: +0.3 quality points

### Total Expected Improvement: +3.8 Quality Points

## Quality Metrics

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| Avg Performance Score | $avgScore | > 90 | $(if ($avgScore -gt 90) { "✅ Good" } else { "⚠️ Needs Work" }) |
| Critical Issues | $criticalIssues | 0 | $(if ($criticalIssues -eq 0) { "✅ Good" } else { "❌ Critical" }) |
| High Issues | $highIssues | < 5 | $(if ($highIssues -lt 5) { "✅ Good" } else { "⚠️ Needs Work" }) |
| Files with Issues | $($Hotspots.Count) | < 10% | $(if ($Hotspots.Count -lt ($files.Count * 0.1)) { "✅ Good" } else { "⚠️ Needs Work" }) |

## Best Practices

### 1. Profile First
- Don't optimize prematurely
- Use Unreal Insights to find real bottlenecks
- Focus on hot paths (code that runs frequently)

### 2. Measure Impact
- Benchmark before and after
- Use stat commands to verify improvements
- Track frame time and memory usage

### 3. Maintain Readability
- Don't sacrifice clarity for micro-optimizations
- Comment non-obvious optimizations
- Balance performance with maintainability

### 4. Test Thoroughly
- Ensure optimizations don't break functionality
- Test on target hardware
- Check for edge cases

### 5. Document Decisions
- Explain why optimizations were made
- Note performance characteristics
- Update documentation

## Resources

- [Unreal Engine Performance Guidelines](https://docs.unrealengine.com/5.3/en-US/performance-guidelines-for-unreal-engine/)
- [Unreal Insights Documentation](https://docs.unrealengine.com/5.3/en-US/unreal-insights-in-unreal-engine/)
- [C++ Performance Tips](https://www.agner.org/optimize/)
- [Game Programming Patterns - Object Pool](https://gameprogrammingpatterns.com/object-pool.html)

## Next Steps

1. **Review critical issues** immediately
2. **Profile the application** using Unreal Insights
3. **Prioritize optimizations** based on profiling data
4. **Implement fixes** following the phased plan
5. **Measure improvements** after each phase
6. **Update coding guidelines** with performance best practices

---
*Generated by PerformanceHotspotDetector.ps1*
"@

# Ensure output directory exists
$outputDir = Split-Path $OutputFile -Parent
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nPerformance Analysis Complete!" -ForegroundColor Green
Write-Host "Found $totalIssues performance issues in $($Hotspots.Count) files" -ForegroundColor Yellow
Write-Host "Critical: $criticalIssues, High: $highIssues, Medium: $mediumIssues, Low: $lowIssues" -ForegroundColor Cyan
Write-Host "Average Performance Score: $avgScore/100" -ForegroundColor Cyan
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
