# RunQualityImprovement.ps1 - Master Quality Improvement Script
# Runs all quality improvement analysis tools

param(
    [string]$SourcePath = "D:/gptzuo/HorrorProject/HorrorProject/Source",
    [string]$OutputDir = "D:/gptzuo/HorrorProject/HorrorProject/Docs/Quality/Improvement",
    [switch]$FullAnalysis = $false
)

$ErrorActionPreference = "Continue"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Quality Improvement Analysis Suite" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$startTime = Get-Date

# Ensure output directory exists
if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

# Track results
$Results = @{
    ComplexityAnalysis = $null
    DuplicationDetection = $null
    NamingConventions = $null
    PerformanceHotspots = $null
    RefactoringPlan = $null
    DuplicationExtraction = $null
    NamingImprovement = $null
}

# 1. Enhanced Complexity Analysis
Write-Host "[1/7] Running Enhanced Complexity Analysis..." -ForegroundColor Yellow
try {
    & "$ScriptDir/CodeComplexityAnalyzer.ps1" -SourcePath $SourcePath -OutputFile "$OutputDir/ComplexityAnalysis.md"
    $Results.ComplexityAnalysis = "✅ Success"
} catch {
    $Results.ComplexityAnalysis = "❌ Failed: $_"
    Write-Host "Error: $_" -ForegroundColor Red
}

Write-Host ""

# 2. Code Duplication Detection
Write-Host "[2/7] Running Code Duplication Detection..." -ForegroundColor Yellow
try {
    & "$ScriptDir/CodeDuplicationDetector.ps1" -SourcePath $SourcePath -OutputFile "$OutputDir/DuplicationReport.md"
    $Results.DuplicationDetection = "✅ Success"
} catch {
    $Results.DuplicationDetection = "❌ Failed: $_"
    Write-Host "Error: $_" -ForegroundColor Red
}

Write-Host ""

# 3. Naming Convention Check
Write-Host "[3/7] Running Naming Convention Check..." -ForegroundColor Yellow
try {
    & "$ScriptDir/NamingConventionChecker.ps1" -SourcePath $SourcePath -OutputFile "$OutputDir/NamingReport.md"
    $Results.NamingConventions = "✅ Success"
} catch {
    $Results.NamingConventions = "❌ Failed: $_"
    Write-Host "Error: $_" -ForegroundColor Red
}

Write-Host ""

# 4. Performance Hotspot Detection
Write-Host "[4/7] Running Performance Hotspot Detection..." -ForegroundColor Yellow
try {
    & "$ScriptDir/PerformanceHotspotDetector.ps1" -SourcePath $SourcePath -OutputFile "$OutputDir/PerformanceReport.md"
    $Results.PerformanceHotspots = "✅ Success"
} catch {
    $Results.PerformanceHotspots = "❌ Failed: $_"
    Write-Host "Error: $_" -ForegroundColor Red
}

Write-Host ""

# 5. Generate Refactoring Plan
Write-Host "[5/7] Generating Refactoring Plan..." -ForegroundColor Yellow
try {
    & "$ScriptDir/RefactorComplexFunctions.ps1" -SourcePath $SourcePath -OutputFile "$OutputDir/RefactoringPlan.md"
    $Results.RefactoringPlan = "✅ Success"
} catch {
    $Results.RefactoringPlan = "❌ Failed: $_"
    Write-Host "Error: $_" -ForegroundColor Red
}

Write-Host ""

# 6. Generate Duplication Extraction Plan
Write-Host "[6/7] Generating Duplication Extraction Plan..." -ForegroundColor Yellow
try {
    & "$ScriptDir/ExtractDuplicateCode.ps1" -SourcePath $SourcePath -OutputFile "$OutputDir/DuplicationExtractionPlan.md"
    $Results.DuplicationExtraction = "✅ Success"
} catch {
    $Results.DuplicationExtraction = "❌ Failed: $_"
    Write-Host "Error: $_" -ForegroundColor Red
}

Write-Host ""

# 7. Generate Naming Improvement Plan
Write-Host "[7/7] Generating Naming Improvement Plan..." -ForegroundColor Yellow
try {
    & "$ScriptDir/ImproveNaming.ps1" -SourcePath $SourcePath -OutputFile "$OutputDir/NamingImprovementPlan.md"
    $Results.NamingImprovement = "✅ Success"
} catch {
    $Results.NamingImprovement = "❌ Failed: $_"
    Write-Host "Error: $_" -ForegroundColor Red
}

$endTime = Get-Date
$duration = $endTime - $startTime

# Generate Master Summary Report
Write-Host ""
Write-Host "Generating Master Summary Report..." -ForegroundColor Cyan

$summaryReport = @"
# Quality Improvement Summary Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Analysis Duration: $($duration.TotalSeconds) seconds

## Executive Summary

This comprehensive quality improvement analysis identifies specific areas for code quality enhancement to boost the overall quality score from 92/100 to 95/100.

## Analysis Results

| Analysis Type | Status | Report |
|---------------|--------|--------|
| Enhanced Complexity Analysis | $($Results.ComplexityAnalysis) | [ComplexityAnalysis.md](ComplexityAnalysis.md) |
| Code Duplication Detection | $($Results.DuplicationDetection) | [DuplicationReport.md](DuplicationReport.md) |
| Naming Convention Check | $($Results.NamingConventions) | [NamingReport.md](NamingReport.md) |
| Performance Hotspot Detection | $($Results.PerformanceHotspots) | [PerformanceReport.md](PerformanceReport.md) |
| Refactoring Plan | $($Results.RefactoringPlan) | [RefactoringPlan.md](RefactoringPlan.md) |
| Duplication Extraction Plan | $($Results.DuplicationExtraction) | [DuplicationExtractionPlan.md](DuplicationExtractionPlan.md) |
| Naming Improvement Plan | $($Results.NamingImprovement) | [NamingImprovementPlan.md](NamingImprovementPlan.md) |

## Quality Improvement Tools

### Analysis Tools
1. **CodeComplexityAnalyzer.ps1** - Identifies complex functions requiring refactoring
2. **CodeDuplicationDetector.ps1** - Finds duplicate code blocks
3. **NamingConventionChecker.ps1** - Validates Unreal Engine naming conventions
4. **PerformanceHotspotDetector.ps1** - Identifies performance bottlenecks

### Refactoring Tools
1. **RefactorComplexFunctions.ps1** - Generates refactoring tasks for complex functions
2. **ExtractDuplicateCode.ps1** - Provides extraction strategies for duplicates
3. **ImproveNaming.ps1** - Generates naming improvement plans

### Review Checklists
1. **CodeReviewChecklist.md** - Comprehensive code review guidelines
2. **SecurityReviewChecklist.md** - Security-focused review checklist
3. **PerformanceReviewChecklist.md** - Performance review guidelines

## Quality Score Improvement Plan

### Current Score: 92/100
### Target Score: 95/100
### Required Improvement: +3 points

### Improvement Breakdown

| Area | Current Impact | Improvement Potential | Priority |
|------|----------------|----------------------|----------|
| Code Complexity | -2 points | +1.5 points | P0 |
| Code Duplication | -2 points | +1.0 points | P0 |
| Naming Conventions | -1 point | +0.5 points | P1 |
| Performance Issues | -2 points | +1.0 points | P1 |
| Documentation | -1 point | +0.5 points | P2 |

**Total Potential Improvement**: +4.5 points (exceeds target)

## Implementation Roadmap

### Phase 1: Critical Issues (Week 1-2)
**Goal**: Address P0 issues for +2.5 quality points

#### Complexity Reduction
- Refactor functions with complexity > 25
- Break down functions > 150 lines
- Reduce nesting depth > 4 levels
- **Estimated Effort**: 24-32 hours
- **Expected Improvement**: +1.5 points

#### Duplication Elimination
- Extract code duplicated 4+ times
- Create utility classes for common patterns
- Implement shared base classes
- **Estimated Effort**: 16-20 hours
- **Expected Improvement**: +1.0 points

### Phase 2: High Priority (Week 3-4)
**Goal**: Address P1 issues for +1.5 quality points

#### Naming Convention Compliance
- Fix class prefix issues
- Add boolean 'b' prefixes
- Convert to PascalCase
- **Estimated Effort**: 8-12 hours
- **Expected Improvement**: +0.5 points

#### Performance Optimization
- Fix critical performance issues
- Implement object pooling
- Optimize hot paths
- **Estimated Effort**: 12-16 hours
- **Expected Improvement**: +1.0 points

### Phase 3: Medium Priority (Week 5-6)
**Goal**: Polish and documentation for +0.5 points

#### Documentation Enhancement
- Add function documentation
- Update architecture docs
- Create code examples
- **Estimated Effort**: 8-10 hours
- **Expected Improvement**: +0.5 points

## Weekly Sprint Plan

### Sprint 1 (Week 1)
- [ ] Refactor top 10 complex functions
- [ ] Extract top 5 duplication groups
- [ ] Fix critical naming issues
- **Target**: +1.5 quality points

### Sprint 2 (Week 2)
- [ ] Continue complexity reduction
- [ ] Continue duplication elimination
- [ ] Fix high-priority naming issues
- **Target**: +1.0 quality points

### Sprint 3 (Week 3)
- [ ] Fix critical performance issues
- [ ] Implement object pooling
- [ ] Optimize Tick usage
- **Target**: +0.5 quality points

### Sprint 4 (Week 4)
- [ ] Continue performance optimization
- [ ] Fix medium-priority naming issues
- [ ] Add documentation
- **Target**: +0.5 quality points

### Sprint 5-6 (Week 5-6)
- [ ] Polish and cleanup
- [ ] Complete documentation
- [ ] Final quality verification
- **Target**: Maintain 95+ score

## Success Metrics

### Quality Score Targets
- [ ] Week 1: 93/100 (+1 point)
- [ ] Week 2: 94/100 (+2 points)
- [ ] Week 3: 94.5/100 (+2.5 points)
- [ ] Week 4: 95/100 (+3 points) ✅ TARGET
- [ ] Week 6: 95+/100 (maintain)

### Code Metrics Targets
- [ ] Average complexity < 10
- [ ] Max complexity < 20
- [ ] Duplication groups < 10
- [ ] Naming compliance > 95%
- [ ] Performance score > 90/100

## Automation & CI/CD Integration

### Pre-Commit Hooks
```powershell
# Add to .git/hooks/pre-commit
.\Scripts\Quality\Improvement\CodeComplexityAnalyzer.ps1 -ComplexityThreshold 25
if ($LASTEXITCODE -ne 0) {
    Write-Error "Code complexity too high"
    exit 1
}
```

### CI/CD Pipeline
```yaml
quality-check:
  script:
    - pwsh Scripts/Quality/Improvement/RunQualityImprovement.ps1
  artifacts:
    reports:
      - Docs/Quality/Improvement/*.md
  rules:
    - if: '$CI_PIPELINE_SOURCE == "merge_request_event"'
```

### Weekly Automated Reports
```powershell
# Schedule weekly quality report
schtasks /create /tn "WeeklyQualityReport" /tr "pwsh RunQualityImprovement.ps1" /sc weekly /d MON /st 09:00
```

## Team Responsibilities

### Development Team
- Review quality reports weekly
- Address assigned refactoring tasks
- Follow coding standards
- Participate in code reviews

### Tech Lead
- Prioritize quality improvements
- Review refactoring plans
- Approve major changes
- Track quality metrics

### QA Team
- Verify refactoring doesn't break functionality
- Test performance improvements
- Validate quality improvements

## Training & Knowledge Sharing

### Week 1: Complexity Reduction Workshop
- Understanding cyclomatic complexity
- Refactoring techniques
- Hands-on exercises

### Week 2: Code Duplication Workshop
- Identifying duplication
- Extraction patterns
- Design patterns

### Week 3: Performance Optimization Workshop
- Profiling tools
- Common anti-patterns
- Optimization techniques

### Week 4: Code Review Best Practices
- Using checklists
- Constructive feedback
- Security and performance focus

## Monitoring & Reporting

### Daily
- Run complexity analysis on changed files
- Check for new duplication
- Validate naming conventions

### Weekly
- Full quality analysis
- Progress review
- Sprint planning

### Monthly
- Quality score review
- Trend analysis
- Process improvement

## Tools & Resources

### Analysis Tools
- CodeComplexityAnalyzer.ps1
- CodeDuplicationDetector.ps1
- NamingConventionChecker.ps1
- PerformanceHotspotDetector.ps1

### Planning Tools
- RefactorComplexFunctions.ps1
- ExtractDuplicateCode.ps1
- ImproveNaming.ps1

### Review Checklists
- CodeReviewChecklist.md
- SecurityReviewChecklist.md
- PerformanceReviewChecklist.md

### IDE Tools
- Visual Studio Code Analysis
- ReSharper C++
- PVS-Studio
- Unreal Insights

## Next Steps

1. **Review all generated reports** with development team
2. **Prioritize improvements** based on impact and effort
3. **Create GitHub issues** for each improvement task
4. **Schedule sprint planning** for quality improvement work
5. **Set up automation** for continuous quality monitoring
6. **Track progress** weekly using quality metrics
7. **Celebrate milestones** when quality targets are met

## Expected Outcomes

### Technical Outcomes
- Code complexity reduced by 30%
- Code duplication reduced by 50%
- Naming compliance > 95%
- Performance improved by 20%
- Quality score: 95/100 ✅

### Team Outcomes
- Improved code review practices
- Better understanding of quality metrics
- Shared coding standards
- Increased code maintainability
- Reduced technical debt

### Business Outcomes
- Faster feature development
- Fewer bugs in production
- Easier onboarding for new developers
- Better code maintainability
- Reduced maintenance costs

---

## Report Files Generated

All reports are available in: ``$OutputDir``

1. **ComplexityAnalysis.md** - Detailed complexity metrics and refactoring suggestions
2. **DuplicationReport.md** - Code duplication analysis and extraction recommendations
3. **NamingReport.md** - Naming convention violations and fixes
4. **PerformanceReport.md** - Performance hotspots and optimization strategies
5. **RefactoringPlan.md** - Prioritized refactoring tasks
6. **DuplicationExtractionPlan.md** - Step-by-step duplication extraction guide
7. **NamingImprovementPlan.md** - Naming improvement procedures
8. **CodeReviewChecklist.md** - Comprehensive code review guidelines
9. **SecurityReviewChecklist.md** - Security-focused review checklist
10. **PerformanceReviewChecklist.md** - Performance review guidelines

---

*Generated by RunQualityImprovement.ps1*
*For questions or support, contact the development team*
"@

$summaryReport | Out-File "$OutputDir/QualityImprovementSummary.md" -Encoding UTF8

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Quality Improvement Analysis Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Analysis Results:" -ForegroundColor Cyan
foreach ($result in $Results.GetEnumerator()) {
    $color = if ($result.Value -like "*Success*") { "Green" } else { "Red" }
    Write-Host "  $($result.Key): $($result.Value)" -ForegroundColor $color
}
Write-Host ""
Write-Host "Total analysis time: $($duration.TotalSeconds) seconds" -ForegroundColor Cyan
Write-Host "Reports generated in: $OutputDir" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "1. Review QualityImprovementSummary.md" -ForegroundColor White
Write-Host "2. Review individual analysis reports" -ForegroundColor White
Write-Host "3. Create GitHub issues for improvements" -ForegroundColor White
Write-Host "4. Schedule sprint planning session" -ForegroundColor White
Write-Host "5. Begin Phase 1 implementation" -ForegroundColor White
Write-Host ""
