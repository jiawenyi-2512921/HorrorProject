# AssessRisks.ps1 - 评估风险
# 对识别的风险进行详细评估和优先级排序

param(
    [string]$RiskFile = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data/risks.json",
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"

function Load-Risks {
    param([string]$FilePath)

    if (Test-Path $FilePath) {
        return Get-Content $FilePath -Raw -Encoding UTF8 | ConvertFrom-Json
    }

    Write-Host "错误: 风险文件不存在: $FilePath" -ForegroundColor Red
    exit 1
}

function Calculate-RiskScore {
    param($Risk)

    # 严重程度分数
    $severityScore = switch ($Risk.Severity) {
        "High" { 3 }
        "Medium" { 2 }
        "Low" { 1 }
        default { 1 }
    }

    # 概率分数
    $probabilityScore = switch ($Risk.Probability) {
        "Certain" { 5 }
        "High" { 4 }
        "Medium" { 3 }
        "Low" { 2 }
        "Rare" { 1 }
        default { 3 }
    }

    # 影响分数
    $impactScore = switch ($Risk.Impact) {
        "High" { 3 }
        "Medium" { 2 }
        "Low" { 1 }
        default { 2 }
    }

    # 综合风险分数 = 严重程度 × 概率 × 影响
    $riskScore = $severityScore * $probabilityScore * $impactScore

    return @{
        Score = $riskScore
        SeverityScore = $severityScore
        ProbabilityScore = $probabilityScore
        ImpactScore = $impactScore
        Priority = if ($riskScore -ge 30) { "Critical" }
                   elseif ($riskScore -ge 18) { "High" }
                   elseif ($riskScore -ge 9) { "Medium" }
                   else { "Low" }
    }
}

function Assess-AllRisks {
    param($RiskData)

    $assessments = @()

    foreach ($risk in $RiskData.Risks) {
        $assessment = Calculate-RiskScore -Risk $risk

        $assessedRisk = $risk.PSObject.Copy()
        $assessedRisk | Add-Member -NotePropertyName "RiskScore" -NotePropertyValue $assessment.Score
        $assessedRisk | Add-Member -NotePropertyName "Priority" -NotePropertyValue $assessment.Priority
        $assessedRisk | Add-Member -NotePropertyName "AssessedDate" -NotePropertyValue (Get-Date -Format "yyyy-MM-dd")

        $assessments += $assessedRisk
    }

    return $assessments | Sort-Object RiskScore -Descending
}

function Generate-RiskMatrix {
    param($Assessments)

    Write-Host ""
    Write-Host "=== 风险矩阵 ===" -ForegroundColor Cyan
    Write-Host ""

    # 按概率和影响分组
    $matrix = @{
        "High-High" = @()
        "High-Medium" = @()
        "High-Low" = @()
        "Medium-High" = @()
        "Medium-Medium" = @()
        "Medium-Low" = @()
        "Low-High" = @()
        "Low-Medium" = @()
        "Low-Low" = @()
    }

    foreach ($risk in $Assessments) {
        $key = "$($risk.Probability)-$($risk.Impact)"
        if ($matrix.ContainsKey($key)) {
            $matrix[$key] += $risk
        }
    }

    # 显示矩阵
    Write-Host "        影响 →" -ForegroundColor Gray
    Write-Host "概率 ↓  Low    Medium    High" -ForegroundColor Gray
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray

    foreach ($prob in @("High", "Medium", "Low")) {
        Write-Host -NoNewline "$prob".PadRight(8)

        foreach ($impact in @("Low", "Medium", "High")) {
            $key = "$prob-$impact"
            $count = $matrix[$key].Count

            $color = if ($prob -eq "High" -and $impact -eq "High") { "Red" }
                     elseif ($prob -eq "High" -or $impact -eq "High") { "Yellow" }
                     else { "Green" }

            Write-Host -NoNewline "  $count".PadRight(10) -ForegroundColor $color
        }
        Write-Host ""
    }
}

function Show-AssessmentReport {
    param($Assessments)

    Write-Host ""
    Write-Host "=== 风险评估报告 ===" -ForegroundColor Cyan
    Write-Host ""

    $byPriority = $Assessments | Group-Object Priority

    Write-Host "风险优先级分布:" -ForegroundColor Yellow
    foreach ($group in $byPriority | Sort-Object { @("Critical", "High", "Medium", "Low").IndexOf($_.Name) }) {
        $color = switch ($group.Name) {
            "Critical" { "Red" }
            "High" { "DarkRed" }
            "Medium" { "Yellow" }
            "Low" { "Gray" }
            default { "White" }
        }
        Write-Host "  $($group.Name): $($group.Count)" -ForegroundColor $color
    }

    Write-Host ""
    Write-Host "关键风险 (Critical & High):" -ForegroundColor Red

    $criticalRisks = $Assessments | Where-Object { $_.Priority -in @("Critical", "High") }

    if ($criticalRisks.Count -gt 0) {
        foreach ($risk in $criticalRisks) {
            Write-Host ""
            Write-Host "  [$($risk.Id)] $($risk.Title)" -ForegroundColor Yellow
            Write-Host "    类别: $($risk.Category) | 优先级: $($risk.Priority) | 分数: $($risk.RiskScore)" -ForegroundColor Gray
            Write-Host "    描述: $($risk.Description)" -ForegroundColor White
            Write-Host "    严重程度: $($risk.Severity) | 概率: $($risk.Probability) | 影响: $($risk.Impact)" -ForegroundColor Gray

            if ($risk.Indicators -and $risk.Indicators.Count -gt 0) {
                Write-Host "    指标:" -ForegroundColor Gray
                foreach ($indicator in $risk.Indicators) {
                    Write-Host "      - $indicator" -ForegroundColor DarkGray
                }
            }
        }
    }
    else {
        Write-Host "  无关键风险" -ForegroundColor Green
    }

    Write-Host ""
    Write-Host "建议行动:" -ForegroundColor Cyan

    $actionCount = 0
    foreach ($risk in $criticalRisks | Select-Object -First 5) {
        $actionCount++
        Write-Host "  $actionCount. 立即处理: $($risk.Title)" -ForegroundColor White
    }

    if ($actionCount -eq 0) {
        Write-Host "  继续监控现有风险" -ForegroundColor Green
    }
}

function Export-AssessmentReport {
    param($Assessments, [string]$OutputPath)

    $reportPath = Split-Path $OutputPath -Parent
    $reportFile = Join-Path $reportPath "RiskAssessment_$(Get-Date -Format 'yyyy-MM-dd').md"

    $markdown = @"
# 风险评估报告

**评估日期**: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
**风险总数**: $($Assessments.Count)

---

## 执行摘要

"@

    $byPriority = $Assessments | Group-Object Priority
    $critical = ($byPriority | Where-Object { $_.Name -eq "Critical" }).Count
    $high = ($byPriority | Where-Object { $_.Name -eq "High" }).Count

    $markdown += @"

本次评估识别了 **$($Assessments.Count)** 个风险项，其中：
- 关键风险: $critical 个
- 高风险: $high 个

"@

    if ($critical -gt 0 -or $high -gt 0) {
        $markdown += "`n需要立即关注和处理关键风险项。`n"
    }
    else {
        $markdown += "`n当前风险处于可控范围。`n"
    }

    $markdown += @"


---

## 风险优先级

| 优先级 | 数量 |
|--------|------|
"@

    foreach ($group in $byPriority | Sort-Object { @("Critical", "High", "Medium", "Low").IndexOf($_.Name) }) {
        $markdown += "`n| $($group.Name) | $($group.Count) |"
    }

    $markdown += @"


---

## 关键风险详情

"@

    $criticalRisks = $Assessments | Where-Object { $_.Priority -in @("Critical", "High") }

    foreach ($risk in $criticalRisks) {
        $markdown += @"


### [$($risk.Id)] $($risk.Title)

- **类别**: $($risk.Category)
- **优先级**: $($risk.Priority)
- **风险分数**: $($risk.RiskScore)
- **严重程度**: $($risk.Severity)
- **发生概率**: $($risk.Probability)
- **影响程度**: $($risk.Impact)
- **状态**: $($risk.Status)

**描述**: $($risk.Description)

"@

        if ($risk.Indicators -and $risk.Indicators.Count -gt 0) {
            $markdown += "**关键指标**:`n"
            foreach ($indicator in $risk.Indicators) {
                $markdown += "- $indicator`n"
            }
        }
    }

    $markdown += @"


---

## 风险分布

### 按类别

"@

    $byCategory = $Assessments | Group-Object Category
    foreach ($group in $byCategory) {
        $markdown += "- **$($group.Name)**: $($group.Count) 个`n"
    }

    $markdown += @"


---

## 建议行动

"@

    $actionCount = 0
    foreach ($risk in $criticalRisks | Select-Object -First 10) {
        $actionCount++
        $markdown += "`n$actionCount. **立即处理**: $($risk.Title) [$($risk.Id)]"
    }

    $markdown += @"


---

*本报告由自动化系统生成*
*评估时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')*
"@

    $markdown | Out-File -FilePath $reportFile -Encoding UTF8

    return $reportFile
}

# 主执行流程
Write-Host "=== 风险评估系统 ===" -ForegroundColor Cyan
Write-Host "评估时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

Write-Host "[1/3] 加载风险数据..." -ForegroundColor Yellow
$riskData = Load-Risks -FilePath $RiskFile

Write-Host "[2/3] 评估风险..." -ForegroundColor Yellow
$assessments = Assess-AllRisks -RiskData $riskData

Write-Host "[3/3] 生成评估报告..." -ForegroundColor Yellow

Show-AssessmentReport -Assessments $assessments
Generate-RiskMatrix -Assessments $assessments

if ($GenerateReport) {
    $reportFile = Export-AssessmentReport -Assessments $assessments -OutputPath $RiskFile
    Write-Host ""
    Write-Host "详细报告已生成: $reportFile" -ForegroundColor Green
}

# 更新风险文件
$riskData.Risks = $assessments
$riskData.LastUpdated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$riskData | ConvertTo-Json -Depth 10 | Out-File -FilePath $RiskFile -Encoding UTF8

Write-Host ""
Write-Host "风险评估已更新: $RiskFile" -ForegroundColor Green
Write-Host ""
Write-Host "=== 评估完成 ===" -ForegroundColor Cyan
