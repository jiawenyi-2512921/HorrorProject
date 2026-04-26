# ComplianceValidator.ps1
# 合规性验证器

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/Compliance",
    [ValidateSet("ESRB", "PEGI", "CERO", "All")]
    [string]$RatingSystem = "All"
)

$ErrorActionPreference = "Continue"
$Script:ComplianceResults = @{
    Legal = @{Passed=@(); Issues=@()}
    Privacy = @{Passed=@(); Issues=@()}
    Accessibility = @{Passed=@(); Issues=@()}
    Content = @{Passed=@(); Issues=@()}
}

function Write-ComplianceLog {
    param([string]$Message, [string]$Type = "INFO")
    $color = switch($Type) {
        "CRITICAL" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor $color
}

function Test-LegalCompliance {
    Write-Host "`n=== 法律合规检查 ===" -ForegroundColor Cyan

    # 检查版权信息
    $copyrightFiles = @(
        "$ProjectRoot/LICENSE",
        "$ProjectRoot/COPYRIGHT",
        "$ProjectRoot/NOTICE"
    )

    $hasCopyright = $false
    foreach ($file in $copyrightFiles) {
        if (Test-Path $file) {
            Write-ComplianceLog "版权文件存在: $(Split-Path $file -Leaf)" "SUCCESS"
            $Script:ComplianceResults.Legal.Passed += "版权文件: $(Split-Path $file -Leaf)"
            $hasCopyright = $true
        }
    }

    if (-not $hasCopyright) {
        Write-ComplianceLog "缺少版权声明文件" "CRITICAL"
        $Script:ComplianceResults.Legal.Issues += "缺少版权声明"
    }

    # 检查第三方许可
    $thirdPartyPath = "$ProjectRoot/THIRD_PARTY_LICENSES.md"
    if (Test-Path $thirdPartyPath) {
        Write-ComplianceLog "第三方许可文件存在" "SUCCESS"
        $Script:ComplianceResults.Legal.Passed += "第三方许可文档"
    } else {
        Write-ComplianceLog "缺少第三方许可文档" "WARNING"
        $Script:ComplianceResults.Legal.Issues += "缺少第三方许可文档"
    }

    # 检查商标信息
    $uprojectPath = "$ProjectRoot/HorrorProject/HorrorProject.uproject"
    if (Test-Path $uprojectPath) {
        try {
            $uproject = Get-Content $uprojectPath -Raw | ConvertFrom-Json
            if ($uproject.PSObject.Properties.Name -contains "CompanyName") {
                Write-ComplianceLog "公司名称已配置: $($uproject.CompanyName)" "SUCCESS"
                $Script:ComplianceResults.Legal.Passed += "公司信息完整"
            } else {
                Write-ComplianceLog "缺少公司名称" "WARNING"
                $Script:ComplianceResults.Legal.Issues += "缺少公司信息"
            }

            if ($uproject.PSObject.Properties.Name -contains "CopyrightNotice") {
                Write-ComplianceLog "版权声明已配置" "SUCCESS"
                $Script:ComplianceResults.Legal.Passed += "版权声明配置"
            } else {
                Write-ComplianceLog "缺少版权声明" "WARNING"
                $Script:ComplianceResults.Legal.Issues += "缺少版权声明配置"
            }
        }
        catch {
            Write-ComplianceLog "项目文件解析失败" "CRITICAL"
        }
    }
}

function Test-PrivacyCompliance {
    Write-Host "`n=== 隐私合规检查 ===" -ForegroundColor Cyan

    # 检查隐私政策
    $privacyFiles = @(
        "$ProjectRoot/PRIVACY_POLICY.md",
        "$ProjectRoot/Privacy.md",
        "$ProjectRoot/PrivacyPolicy.txt"
    )

    $hasPrivacy = $false
    foreach ($file in $privacyFiles) {
        if (Test-Path $file) {
            Write-ComplianceLog "隐私政策存在: $(Split-Path $file -Leaf)" "SUCCESS"
            $Script:ComplianceResults.Privacy.Passed += "隐私政策文档"
            $hasPrivacy = $true
            break
        }
    }

    if (-not $hasPrivacy) {
        Write-ComplianceLog "缺少隐私政策" "CRITICAL"
        $Script:ComplianceResults.Privacy.Issues += "缺少隐私政策"
    }

    # 检查数据收集代码
    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (Test-Path $sourcePath) {
        $sourceFiles = Get-ChildItem -Path $sourcePath -Recurse -Include *.cpp,*.h

        $dataCollectionPatterns = @(
            "Analytics",
            "Telemetry",
            "UserData",
            "PersonalInfo"
        )

        $foundDataCollection = $false
        foreach ($pattern in $dataCollectionPatterns) {
            foreach ($file in $sourceFiles) {
                $content = Get-Content $file.FullName -Raw
                if ($content -match $pattern) {
                    $foundDataCollection = $true
                    break
                }
            }
            if ($foundDataCollection) { break }
        }

        if ($foundDataCollection) {
            Write-ComplianceLog "检测到数据收集代码" "WARNING"
            if (-not $hasPrivacy) {
                Write-ComplianceLog "数据收集需要隐私政策" "CRITICAL"
                $Script:ComplianceResults.Privacy.Issues += "数据收集缺少隐私政策"
            } else {
                Write-ComplianceLog "数据收集有隐私政策支持" "SUCCESS"
                $Script:ComplianceResults.Privacy.Passed += "数据收集合规"
            }
        } else {
            Write-ComplianceLog "未检测到数据收集" "SUCCESS"
            $Script:ComplianceResults.Privacy.Passed += "无数据收集"
        }
    }

    # 检查GDPR合规
    $gdprPath = "$ProjectRoot/GDPR_COMPLIANCE.md"
    if (Test-Path $gdprPath) {
        Write-ComplianceLog "GDPR合规文档存在" "SUCCESS"
        $Script:ComplianceResults.Privacy.Passed += "GDPR合规文档"
    } else {
        Write-ComplianceLog "建议添加GDPR合规文档" "WARNING"
    }
}

function Test-AccessibilityCompliance {
    Write-Host "`n=== 可访问性合规检查 ===" -ForegroundColor Cyan

    # 检查可访问性配置
    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (Test-Path $engineIni) {
        $content = Get-Content $engineIni -Raw

        $accessibilityFeatures = @{
            "字幕支持" = "bEnableSubtitles"
            "色盲模式" = "ColorBlindMode"
            "UI缩放" = "UIScaleRule"
        }

        foreach ($feature in $accessibilityFeatures.Keys) {
            if ($content -match [regex]::Escape($accessibilityFeatures[$feature])) {
                Write-ComplianceLog "$feature : 已配置" "SUCCESS"
                $Script:ComplianceResults.Accessibility.Passed += $feature
            } else {
                Write-ComplianceLog "$feature : 未配置" "WARNING"
                $Script:ComplianceResults.Accessibility.Issues += "$feature 未配置"
            }
        }
    }

    # 检查输入配置
    $inputIni = "$ProjectRoot/HorrorProject/Config/DefaultInput.ini"
    if (Test-Path $inputIni) {
        $content = Get-Content $inputIni -Raw

        if ($content -match "ActionMappings" -and $content -match "AxisMappings") {
            Write-ComplianceLog "输入映射已配置" "SUCCESS"
            $Script:ComplianceResults.Accessibility.Passed += "输入映射"
        } else {
            Write-ComplianceLog "输入映射不完整" "WARNING"
            $Script:ComplianceResults.Accessibility.Issues += "输入映射不完整"
        }

        # 检查键盘重映射支持
        if ($content -match "bAllowKeyRebinding") {
            Write-ComplianceLog "支持键盘重映射" "SUCCESS"
            $Script:ComplianceResults.Accessibility.Passed += "键盘重映射"
        } else {
            Write-ComplianceLog "建议支持键盘重映射" "WARNING"
        }
    }

    # 检查可访问性文档
    $accessibilityDoc = "$ProjectRoot/ACCESSIBILITY.md"
    if (Test-Path $accessibilityDoc) {
        Write-ComplianceLog "可访问性文档存在" "SUCCESS"
        $Script:ComplianceResults.Accessibility.Passed += "可访问性文档"
    } else {
        Write-ComplianceLog "建议添加可访问性文档" "WARNING"
    }
}

function Test-ContentRating {
    Write-Host "`n=== 内容分级检查 ===" -ForegroundColor Cyan

    # 检查内容描述符
    $contentDescriptors = @{
        "暴力内容" = @("Blood", "Gore", "Violence", "Kill", "Death")
        "恐怖内容" = @("Horror", "Scary", "Fear", "Terror")
        "语言内容" = @("Profanity", "BadWord", "Curse")
        "性内容" = @("Sexual", "Nudity", "Adult")
    }

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    $contentPath = "$ProjectRoot/HorrorProject/Content"
    $foundContent = @{}

    foreach ($category in $contentDescriptors.Keys) {
        $foundContent[$category] = $false

        if (Test-Path $sourcePath) {
            $sourceFiles = Get-ChildItem -Path $sourcePath -Recurse -Include *.cpp,*.h
            foreach ($file in $sourceFiles) {
                $content = Get-Content $file.FullName -Raw
                foreach ($keyword in $contentDescriptors[$category]) {
                    if ($content -match $keyword) {
                        $foundContent[$category] = $true
                        break
                    }
                }
                if ($foundContent[$category]) { break }
            }
        }

        if ($foundContent[$category]) {
            Write-ComplianceLog "检测到 $category" "WARNING"
            $Script:ComplianceResults.Content.Issues += "包含 $category"
        }
    }

    # 检查年龄分级配置
    $gameIni = "$ProjectRoot/HorrorProject/Config/DefaultGame.ini"
    if (Test-Path $gameIni) {
        $content = Get-Content $gameIni -Raw

        if ($content -match "AgeRating|ContentRating") {
            Write-ComplianceLog "年龄分级已配置" "SUCCESS"
            $Script:ComplianceResults.Content.Passed += "年龄分级配置"
        } else {
            Write-ComplianceLog "建议配置年龄分级" "WARNING"
            $Script:ComplianceResults.Content.Issues += "缺少年龄分级配置"
        }
    }

    # 生成分级建议
    $suggestedRating = Get-SuggestedRating -ContentFound $foundContent
    Write-ComplianceLog "建议分级: $suggestedRating" "INFO"
}

function Get-SuggestedRating {
    param($ContentFound)

    $hasViolence = $ContentFound["暴力内容"]
    $hasHorror = $ContentFound["恐怖内容"]
    $hasLanguage = $ContentFound["语言内容"]
    $hasSexual = $ContentFound["性内容"]

    if ($hasSexual -or ($hasViolence -and $hasLanguage)) {
        return "M (Mature 17+) / PEGI 18"
    }
    elseif ($hasViolence -or $hasHorror) {
        return "T (Teen 13+) / PEGI 16"
    }
    elseif ($hasLanguage) {
        return "T (Teen 13+) / PEGI 12"
    }
    else {
        return "E10+ (Everyone 10+) / PEGI 7"
    }
}

function Test-PlatformCompliance {
    Write-Host "`n=== 平台合规检查 ===" -ForegroundColor Cyan

    # 检查平台特定配置
    $configPath = "$ProjectRoot/HorrorProject/Config"
    $platforms = @("Windows", "Linux", "Mac", "Android", "iOS")

    $configuredPlatforms = @()
    foreach ($platform in $platforms) {
        $platformPath = Join-Path $configPath $platform
        if (Test-Path $platformPath) {
            Write-ComplianceLog "平台配置存在: $platform" "SUCCESS"
            $configuredPlatforms += $platform
            $Script:ComplianceResults.Legal.Passed += "$platform 平台配置"
        }
    }

    if ($configuredPlatforms.Count -eq 0) {
        Write-ComplianceLog "未找到平台特定配置" "WARNING"
    } else {
        Write-ComplianceLog "配置的平台: $($configuredPlatforms -join ', ')" "INFO"
    }

    # 检查商店合规文档
    $storeComplianceDocs = @(
        "$ProjectRoot/STEAM_COMPLIANCE.md",
        "$ProjectRoot/EPIC_COMPLIANCE.md",
        "$ProjectRoot/CONSOLE_COMPLIANCE.md"
    )

    foreach ($doc in $storeComplianceDocs) {
        if (Test-Path $doc) {
            Write-ComplianceLog "商店合规文档存在: $(Split-Path $doc -Leaf)" "SUCCESS"
            $Script:ComplianceResults.Legal.Passed += "$(Split-Path $doc -Leaf)"
        }
    }
}

function Test-SecurityCompliance {
    Write-Host "`n=== 安全合规检查 ===" -ForegroundColor Cyan

    # 检查敏感信息
    $sensitivePatterns = @{
        "API密钥" = "api[_-]?key|apikey"
        "密码" = "password\s*=\s*['\"][\w]+"
        "令牌" = "token\s*=\s*['\"][\w]+"
        "密钥" = "secret\s*=\s*['\"][\w]+"
    }

    $foundSensitive = $false
    $sourcePath = "$ProjectRoot/HorrorProject/Source"

    if (Test-Path $sourcePath) {
        $sourceFiles = Get-ChildItem -Path $sourcePath -Recurse -Include *.cpp,*.h,*.ini,*.json

        foreach ($pattern in $sensitivePatterns.Keys) {
            foreach ($file in $sourceFiles) {
                $content = Get-Content $file.FullName -Raw
                if ($content -match $sensitivePatterns[$pattern]) {
                    Write-ComplianceLog "发现硬编码的 $pattern : $($file.Name)" "CRITICAL"
                    $Script:ComplianceResults.Legal.Issues += "硬编码的 $pattern"
                    $foundSensitive = $true
                }
            }
        }
    }

    if (-not $foundSensitive) {
        Write-ComplianceLog "未发现硬编码的敏感信息" "SUCCESS"
        $Script:ComplianceResults.Legal.Passed += "安全检查通过"
    }

    # 检查 .gitignore
    $gitignorePath = "$ProjectRoot/.gitignore"
    if (Test-Path $gitignorePath) {
        $content = Get-Content $gitignorePath -Raw
        $requiredPatterns = @("*.key", "*.pem", "*.env", "*.secret")
        $missingPatterns = @()

        foreach ($pattern in $requiredPatterns) {
            if ($content -notmatch [regex]::Escape($pattern)) {
                $missingPatterns += $pattern
            }
        }

        if ($missingPatterns.Count -eq 0) {
            Write-ComplianceLog ".gitignore 配置完善" "SUCCESS"
            $Script:ComplianceResults.Legal.Passed += ".gitignore 安全配置"
        } else {
            Write-ComplianceLog ".gitignore 缺少模式: $($missingPatterns -join ', ')" "WARNING"
            $Script:ComplianceResults.Legal.Issues += ".gitignore 配置不完整"
        }
    }
}

function Calculate-ComplianceScore {
    $totalPassed = 0
    $totalIssues = 0

    foreach ($category in $Script:ComplianceResults.Keys) {
        $totalPassed += $Script:ComplianceResults[$category].Passed.Count
        $totalIssues += $Script:ComplianceResults[$category].Issues.Count
    }

    $totalChecks = $totalPassed + $totalIssues
    if ($totalChecks -eq 0) { return 0 }

    $score = ($totalPassed / $totalChecks) * 100
    return [math]::Round($score, 2)
}

function Generate-ComplianceReport {
    Write-Host "`n=== 生成合规报告 ===" -ForegroundColor Cyan

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $score = Calculate-ComplianceScore

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        RatingSystem = $RatingSystem
        Score = $score
        Grade = if ($score -ge 90) { "优秀" } elseif ($score -ge 75) { "良好" } elseif ($score -ge 60) { "及格" } else { "不合格" }
        Status = if ($score -ge 75) { "合规" } elseif ($score -ge 60) { "基本合规" } else { "不合规" }
        Categories = @{
            Legal = @{
                Passed = $Script:ComplianceResults.Legal.Passed.Count
                Issues = $Script:ComplianceResults.Legal.Issues.Count
            }
            Privacy = @{
                Passed = $Script:ComplianceResults.Privacy.Passed.Count
                Issues = $Script:ComplianceResults.Privacy.Issues.Count
            }
            Accessibility = @{
                Passed = $Script:ComplianceResults.Accessibility.Passed.Count
                Issues = $Script:ComplianceResults.Accessibility.Issues.Count
            }
            Content = @{
                Passed = $Script:ComplianceResults.Content.Passed.Count
                Issues = $Script:ComplianceResults.Content.Issues.Count
            }
        }
        Details = $Script:ComplianceResults
        Recommendations = Get-ComplianceRecommendations
    }

    $reportFile = Join-Path $ReportPath "Compliance_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-ComplianceLog "报告已保存: $reportFile" "SUCCESS"
    return $report
}

function Get-ComplianceRecommendations {
    $recommendations = @()

    if ($Script:ComplianceResults.Legal.Issues.Count -gt 0) {
        $recommendations += "完善法律文档（许可证、版权声明）"
        $recommendations += "确保第三方资源有合法授权"
    }

    if ($Script:ComplianceResults.Privacy.Issues.Count -gt 0) {
        $recommendations += "添加隐私政策文档"
        $recommendations += "确保数据收集符合GDPR等法规"
    }

    if ($Script:ComplianceResults.Accessibility.Issues.Count -gt 0) {
        $recommendations += "增加可访问性功能（字幕、色盲模式等）"
        $recommendations += "支持键盘重映射和自定义控制"
    }

    if ($Script:ComplianceResults.Content.Issues.Count -gt 0) {
        $recommendations += "配置适当的年龄分级"
        $recommendations += "添加内容警告"
    }

    if ($recommendations.Count -eq 0) {
        $recommendations += "合规性良好，继续保持"
    }

    return $recommendations
}

function Show-ComplianceSummary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "合规性验证摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "合规评分: $($Report.Score)/100" -ForegroundColor White
    Write-Host "评级: $($Report.Grade)" -ForegroundColor $(if($Report.Grade -eq "优秀"){"Green"}elseif($Report.Grade -eq "良好"){"Cyan"}elseif($Report.Grade -eq "及格"){"Yellow"}else{"Red"})
    Write-Host "状态: $($Report.Status)" -ForegroundColor $(if($Report.Status -eq "合规"){"Green"}elseif($Report.Status -eq "基本合规"){"Yellow"}else{"Red"})

    Write-Host "`n分类统计:" -ForegroundColor White
    foreach ($category in $Report.Categories.Keys) {
        $passed = $Report.Categories[$category].Passed
        $issues = $Report.Categories[$category].Issues
        Write-Host "  $category : 通过 $passed, 问题 $issues" -ForegroundColor $(if($issues -eq 0){"Green"}elseif($issues -lt 3){"Yellow"}else{"Red"})
    }

    if ($Report.Recommendations.Count -gt 0) {
        Write-Host "`n改进建议:" -ForegroundColor Cyan
        foreach ($rec in $Report.Recommendations) {
            Write-Host "  - $rec" -ForegroundColor White
        }
    }

    Write-Host "========================================`n" -ForegroundColor Cyan
}

# 主执行流程
Write-Host "`n开始合规性验证...`n" -ForegroundColor Cyan

Test-LegalCompliance
Test-PrivacyCompliance
Test-AccessibilityCompliance
Test-ContentRating
Test-PlatformCompliance
Test-SecurityCompliance

$report = Generate-ComplianceReport
Show-ComplianceSummary -Report $report

if ($report.Score -lt 60) {
    exit 2
} elseif ($report.Score -lt 75) {
    exit 1
} else {
    exit 0
}
