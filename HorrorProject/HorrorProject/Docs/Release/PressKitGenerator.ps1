# Press Kit Generator
# 新闻资料包生成器

<#
.SYNOPSIS
    自动生成游戏新闻资料包

.DESCRIPTION
    此脚本自动收集和组织游戏资源，生成专业的新闻资料包，
    包括截图、视频、Logo、游戏信息等，供媒体和内容创作者使用。

.PARAMETER GameName
    游戏名称

.PARAMETER OutputPath
    输出路径

.PARAMETER IncludeVideos
    是否包含视频文件

.EXAMPLE
    .\PressKitGenerator.ps1 -GameName "HorrorProject" -OutputPath "D:\PressKit"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$GameName,

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = ".\PressKit",

    [Parameter(Mandatory=$false)]
    [switch]$IncludeVideos = $false,

    [Parameter(Mandatory=$false)]
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",

    [Parameter(Mandatory=$false)]
    [string]$Language = "zh-CN"
)

# 配置
$ErrorActionPreference = "Stop"
$ProgressPreference = "Continue"

# 颜色输出函数
function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

# 创建目录函数
function New-DirectoryIfNotExists {
    param([string]$Path)
    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-ColorOutput "✓ 创建目录: $Path" "Green"
    }
}

# 复制文件函数
function Copy-FileWithProgress {
    param(
        [string]$Source,
        [string]$Destination,
        [string]$Description
    )
    if (Test-Path $Source) {
        Copy-Item -Path $Source -Destination $Destination -Force
        Write-ColorOutput "✓ 复制: $Description" "Green"
        return $true
    } else {
        Write-ColorOutput "⚠ 未找到: $Description" "Yellow"
        return $false
    }
}

# 主函数
function New-PressKit {
    Write-ColorOutput "`n========================================" "Cyan"
    Write-ColorOutput "  新闻资料包生成器" "Cyan"
    Write-ColorOutput "========================================`n" "Cyan"

    # 创建主目录结构
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $pressKitPath = Join-Path $OutputPath "$GameName`_PressKit_$timestamp"

    Write-ColorOutput "创建目录结构..." "Yellow"
    New-DirectoryIfNotExists $pressKitPath

    # 创建子目录
    $directories = @{
        "Screenshots" = "截图"
        "Logos" = "Logo和图标"
        "Videos" = "视频"
        "Artwork" = "美术资源"
        "Documents" = "文档"
        "Audio" = "音频"
    }

    foreach ($dir in $directories.Keys) {
        $dirPath = Join-Path $pressKitPath $dir
        New-DirectoryIfNotExists $dirPath
    }

    # 生成README
    Write-ColorOutput "`n生成README文件..." "Yellow"
    $readmeContent = @"
# $GameName - 新闻资料包

生成日期: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## 目录结构

- **Screenshots/** - 游戏截图（高清PNG格式）
- **Logos/** - 游戏Logo和图标（多种格式和尺寸）
- **Videos/** - 预告片和游戏玩法视频
- **Artwork/** - 概念艺术和宣传图
- **Documents/** - 游戏信息、事实表、开发者简介
- **Audio/** - 音乐样本和音效

## 使用许可

本资料包中的所有内容仅供媒体报道和评测使用。
未经许可，不得用于商业用途。

## 联系信息

- **官方网站:** [网站URL]
- **媒体联系:** press@example.com
- **社交媒体:**
  - Twitter: @GameName
  - Discord: [Discord链接]
  - Steam: [Steam页面]

## 游戏信息

**游戏名称:** $GameName
**类型:** 恐怖/冒险
**平台:** PC (Windows)
**发布日期:** [发布日期]
**开发商:** [开发商名称]
**发行商:** [发行商名称]

## 快速事实

- 单人游戏体验
- 预计游戏时长: 8-12小时
- 支持多种语言
- 支持手柄和键鼠操作
- 年龄分级: [分级]

---

感谢您对 $GameName 的关注！
"@

    $readmeContent | Out-File -FilePath (Join-Path $pressKitPath "README.md") -Encoding UTF8
    Write-ColorOutput "✓ README.md 已生成" "Green"

    # 生成游戏事实表
    Write-ColorOutput "`n生成游戏事实表..." "Yellow"
    $factSheetContent = @"
# $GameName - 游戏事实表

## 基本信息

**游戏名称:** $GameName
**类型:** 恐怖/冒险/生存
**平台:** PC (Windows 10/11)
**发布日期:** [发布日期]
**价格:** [价格]
**语言:** 简体中文、英语、日语、韩语等

## 开发信息

**开发商:** [开发商名称]
**发行商:** [发行商名称]
**引擎:** Unreal Engine 5.6
**开发时间:** [X]年
**团队规模:** [X]人

## 游戏特色

- **沉浸式恐怖体验** - 利用UE5的先进技术打造逼真的恐怖氛围
- **动态叙事系统** - 玩家的选择影响故事发展和结局
- **先进的AI系统** - 敌人会学习和适应玩家的策略
- **精美的视觉效果** - Lumen全局光照和Nanite虚拟几何
- **沉浸式音效** - 3D音频和动态音乐系统
- **多重结局** - 根据玩家选择有不同的故事结局

## 游戏玩法

$GameName 是一款第一人称恐怖冒险游戏，玩家将探索一个充满谜题和危险的环境。
游戏强调氛围营造、探索和解谜，而非单纯的战斗。

**核心机制:**
- 探索和调查
- 解谜和互动
- 资源管理
- 隐蔽和逃跑
- 故事选择

## 系统需求

### 最低配置
- **操作系统:** Windows 10 64-bit
- **处理器:** Intel Core i5-8400 / AMD Ryzen 5 2600
- **内存:** 16 GB RAM
- **显卡:** NVIDIA GTX 1060 6GB / AMD RX 580 8GB
- **DirectX:** 版本 12
- **存储空间:** 50 GB 可用空间
- **附注事项:** SSD推荐

### 推荐配置
- **操作系统:** Windows 11 64-bit
- **处理器:** Intel Core i7-10700K / AMD Ryzen 7 3700X
- **内存:** 32 GB RAM
- **显卡:** NVIDIA RTX 3070 / AMD RX 6800
- **DirectX:** 版本 12
- **存储空间:** 50 GB 可用空间
- **附注事项:** SSD必需

## 关键卖点

1. **次世代视觉效果** - 充分利用UE5技术
2. **深度叙事** - 引人入胜的故事和角色
3. **创新玩法** - 独特的恐怖机制
4. **高重玩价值** - 多重结局和隐藏内容
5. **精心设计** - 每个细节都经过精心打磨

## 奖项和认可

- [奖项1]
- [奖项2]
- [媒体评价]

## 开发者简介

[开发团队简介]

## 联系信息

**媒体咨询:** press@example.com
**商务合作:** business@example.com
**技术支持:** support@example.com

**官方网站:** [URL]
**Steam页面:** [URL]
**Discord社区:** [URL]

**社交媒体:**
- Twitter/X: @GameName
- Facebook: /GameName
- Instagram: @GameName
- YouTube: /GameNameOfficial

---

最后更新: $(Get-Date -Format "yyyy-MM-dd")
"@

    $factSheetPath = Join-Path $pressKitPath "Documents\FactSheet.md"
    $factSheetContent | Out-File -FilePath $factSheetPath -Encoding UTF8
    Write-ColorOutput "✓ FactSheet.md 已生成" "Green"

    # 生成开发者简介
    Write-ColorOutput "`n生成开发者简介..." "Yellow"
    $devBioContent = @"
# 开发者简介

## 关于开发团队

[开发团队名称] 是一支充满激情的独立游戏开发团队，专注于创造独特的恐怖游戏体验。

## 团队历史

成立于 [年份]，团队由一群热爱恐怖游戏的开发者组成。我们的目标是创造
既恐怖又有深度的游戏体验，不仅仅依赖跳吓，而是通过氛围、叙事和创新
玩法来营造真正的恐惧感。

## 核心团队成员

### [姓名] - 创意总监
[简介]

### [姓名] - 技术总监
[简介]

### [姓名] - 美术总监
[简介]

### [姓名] - 音频总监
[简介]

## 开发理念

我们相信恐怖游戏不仅仅是吓唬玩家，而是创造一种持久的不安感和紧张氛围。
$GameName 是我们这一理念的体现，结合了心理恐怖、环境叙事和创新玩法。

## 技术专长

- Unreal Engine 5 开发
- 先进的光照和渲染技术
- AI行为系统
- 音频设计和实现
- 叙事设计

## 联系方式

**媒体咨询:** press@example.com
**商务合作:** business@example.com

---

"@

    $devBioPath = Join-Path $pressKitPath "Documents\DeveloperBio.md"
    $devBioContent | Out-File -FilePath $devBioPath -Encoding UTF8
    Write-ColorOutput "✓ DeveloperBio.md 已生成" "Green"

    # 生成引用指南
    Write-ColorOutput "`n生成引用指南..." "Yellow"
    $quoteGuideContent = @"
# 媒体引用指南

## 官方游戏描述

### 简短版本（50字）
[简短描述]

### 标准版本（150字）
[标准描述]

### 详细版本（300字）
[详细描述]

## 官方引用

### 关于游戏愿景
"[引用内容]" - [姓名], [职位]

### 关于技术
"[引用内容]" - [姓名], [职位]

### 关于玩法
"[引用内容]" - [姓名], [职位]

## 关键词和标签

**类型标签:**
#Horror #Adventure #Survival #Psychological #FirstPerson

**特色标签:**
#UnrealEngine5 #IndieGame #Atmospheric #StoryDriven #MultipleEndings

**平台标签:**
#PCGaming #Steam #WindowsGaming

## 社交媒体句柄

- Twitter/X: @GameName
- Instagram: @GameName
- Facebook: /GameName
- Discord: [邀请链接]

## 官方Hashtag

主标签: #$GameName
活动标签: #[活动标签]

## 使用指南

在报道或评测 $GameName 时，欢迎使用本资料包中的任何内容。
我们只要求：

1. 正确标注游戏名称
2. 注明开发商/发行商
3. 链接到官方页面（如可能）
4. 不要修改Logo或商标

---

"@

    $quoteGuidePath = Join-Path $pressKitPath "Documents\QuoteGuide.md"
    $quoteGuideContent | Out-File -FilePath $quoteGuidePath -Encoding UTF8
    Write-ColorOutput "✓ QuoteGuide.md 已生成" "Green"

    # 搜索和复制资源文件
    Write-ColorOutput "`n搜索项目资源..." "Yellow"

    # 搜索截图
    $screenshotSources = @(
        "$ProjectPath\Content\Screenshots",
        "$ProjectPath\Saved\Screenshots",
        "$ProjectPath\Marketing\Screenshots"
    )

    $screenshotCount = 0
    foreach ($source in $screenshotSources) {
        if (Test-Path $source) {
            $screenshots = Get-ChildItem -Path $source -Include *.png,*.jpg,*.jpeg -Recurse -ErrorAction SilentlyContinue
            foreach ($screenshot in $screenshots) {
                $dest = Join-Path $pressKitPath "Screenshots\$($screenshot.Name)"
                Copy-Item -Path $screenshot.FullName -Destination $dest -Force
                $screenshotCount++
            }
        }
    }
    Write-ColorOutput "✓ 复制了 $screenshotCount 张截图" "Green"

    # 搜索Logo
    $logoSources = @(
        "$ProjectPath\Content\UI\Logo",
        "$ProjectPath\Marketing\Logos",
        "$ProjectPath\Branding"
    )

    $logoCount = 0
    foreach ($source in $logoSources) {
        if (Test-Path $source) {
            $logos = Get-ChildItem -Path $source -Include *.png,*.svg,*.ai,*.psd -Recurse -ErrorAction SilentlyContinue
            foreach ($logo in $logos) {
                $dest = Join-Path $pressKitPath "Logos\$($logo.Name)"
                Copy-Item -Path $logo.FullName -Destination $dest -Force
                $logoCount++
            }
        }
    }
    Write-ColorOutput "✓ 复制了 $logoCount 个Logo文件" "Green"

    # 搜索美术资源
    $artworkSources = @(
        "$ProjectPath\Marketing\Artwork",
        "$ProjectPath\Content\ConceptArt"
    )

    $artworkCount = 0
    foreach ($source in $artworkSources) {
        if (Test-Path $source) {
            $artworks = Get-ChildItem -Path $source -Include *.png,*.jpg,*.jpeg -Recurse -ErrorAction SilentlyContinue
            foreach ($artwork in $artworks) {
                $dest = Join-Path $pressKitPath "Artwork\$($artwork.Name)"
                Copy-Item -Path $artwork.FullName -Destination $dest -Force
                $artworkCount++
            }
        }
    }
    Write-ColorOutput "✓ 复制了 $artworkCount 个美术资源" "Green"

    # 复制视频（如果启用）
    if ($IncludeVideos) {
        Write-ColorOutput "`n复制视频文件..." "Yellow"
        $videoSources = @(
            "$ProjectPath\Marketing\Videos",
            "$ProjectPath\Trailers"
        )

        $videoCount = 0
        foreach ($source in $videoSources) {
            if (Test-Path $source) {
                $videos = Get-ChildItem -Path $source -Include *.mp4,*.mov,*.avi -Recurse -ErrorAction SilentlyContinue
                foreach ($video in $videos) {
                    $dest = Join-Path $pressKitPath "Videos\$($video.Name)"
                    Write-ColorOutput "  复制视频: $($video.Name) ($('{0:N2}' -f ($video.Length / 1MB)) MB)" "Cyan"
                    Copy-Item -Path $video.FullName -Destination $dest -Force
                    $videoCount++
                }
            }
        }
        Write-ColorOutput "✓ 复制了 $videoCount 个视频文件" "Green"
    } else {
        Write-ColorOutput "⊘ 跳过视频文件（使用 -IncludeVideos 参数包含视频）" "Yellow"
    }

    # 生成资源清单
    Write-ColorOutput "`n生成资源清单..." "Yellow"
    $inventoryContent = @"
# 资源清单

生成日期: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## 统计

- 截图: $screenshotCount 个文件
- Logo: $logoCount 个文件
- 美术资源: $artworkCount 个文件
- 视频: $(if ($IncludeVideos) { $videoCount } else { "未包含" })

## 文件列表

### 截图
"@

    $screenshots = Get-ChildItem -Path (Join-Path $pressKitPath "Screenshots") -File
    foreach ($file in $screenshots) {
        $inventoryContent += "`n- $($file.Name) ($('{0:N2}' -f ($file.Length / 1MB)) MB)"
    }

    $inventoryContent += "`n`n### Logo"
    $logos = Get-ChildItem -Path (Join-Path $pressKitPath "Logos") -File
    foreach ($file in $logos) {
        $inventoryContent += "`n- $($file.Name) ($('{0:N2}' -f ($file.Length / 1MB)) MB)"
    }

    $inventoryContent += "`n`n### 美术资源"
    $artworks = Get-ChildItem -Path (Join-Path $pressKitPath "Artwork") -File
    foreach ($file in $artworks) {
        $inventoryContent += "`n- $($file.Name) ($('{0:N2}' -f ($file.Length / 1MB)) MB)"
    }

    if ($IncludeVideos) {
        $inventoryContent += "`n`n### 视频"
        $videos = Get-ChildItem -Path (Join-Path $pressKitPath "Videos") -File
        foreach ($file in $videos) {
            $inventoryContent += "`n- $($file.Name) ($('{0:N2}' -f ($file.Length / 1MB)) MB)"
        }
    }

    $inventoryPath = Join-Path $pressKitPath "INVENTORY.md"
    $inventoryContent | Out-File -FilePath $inventoryPath -Encoding UTF8
    Write-ColorOutput "✓ INVENTORY.md 已生成" "Green"

    # 创建压缩包
    Write-ColorOutput "`n创建压缩包..." "Yellow"
    $zipPath = "$pressKitPath.zip"

    if (Test-Path $zipPath) {
        Remove-Item $zipPath -Force
    }

    Compress-Archive -Path $pressKitPath -DestinationPath $zipPath -CompressionLevel Optimal
    $zipSize = (Get-Item $zipPath).Length / 1MB
    Write-ColorOutput "✓ 压缩包已创建: $zipPath ($('{0:N2}' -f $zipSize) MB)" "Green"

    # 生成完成报告
    Write-ColorOutput "`n========================================" "Cyan"
    Write-ColorOutput "  新闻资料包生成完成！" "Green"
    Write-ColorOutput "========================================`n" "Cyan"

    Write-ColorOutput "输出位置: $pressKitPath" "White"
    Write-ColorOutput "压缩包: $zipPath" "White"
    Write-ColorOutput "`n资源统计:" "Yellow"
    Write-ColorOutput "  - 截图: $screenshotCount" "White"
    Write-ColorOutput "  - Logo: $logoCount" "White"
    Write-ColorOutput "  - 美术资源: $artworkCount" "White"
    if ($IncludeVideos) {
        Write-ColorOutput "  - 视频: $videoCount" "White"
    }

    Write-ColorOutput "`n下一步:" "Yellow"
    Write-ColorOutput "  1. 检查生成的文件" "White"
    Write-ColorOutput "  2. 补充缺失的资源" "White"
    Write-ColorOutput "  3. 更新文档中的占位符" "White"
    Write-ColorOutput "  4. 上传到媒体服务器或云存储" "White"
    Write-ColorOutput "  5. 分享给媒体和内容创作者`n" "White"

    # 打开输出目录
    if ($Host.Name -eq "ConsoleHost") {
        $openFolder = Read-Host "是否打开输出目录？(Y/N)"
        if ($openFolder -eq "Y" -or $openFolder -eq "y") {
            Invoke-Item $pressKitPath
        }
    }
}

# 执行主函数
try {
    New-PressKit
} catch {
    Write-ColorOutput "`n错误: $_" "Red"
    Write-ColorOutput $_.ScriptStackTrace "Red"
    exit 1
}
