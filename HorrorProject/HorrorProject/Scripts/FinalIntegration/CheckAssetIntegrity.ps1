# CheckAssetIntegrity.ps1
# 检查资产完整性

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/AssetIntegrity",
    [switch]$DeepScan,
    [switch]$FixBroken
)

$ErrorActionPreference = "Continue"
$Script:AssetReport = @{
    Valid = @()
    Missing = @()
    Broken = @()
    Orphaned = @()
}

function Write-AssetLog {
    param([string]$Message, [string]$Type = "INFO")
    $color = switch($Type) {
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor $color
}

function Test-ContentAssets {
    Write-AssetLog "检查内容资产..." "INFO"

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) {
        Write-AssetLog "Content目录不存在" "ERROR"
        return
    }

    $assetTypes = @{
        "Blueprints" = @("*.uasset")
        "Materials" = @("*.uasset")
        "Textures" = @("*.uasset", "*.png", "*.jpg", "*.tga")
        "Audio" = @("*.uasset", "*.wav", "*.mp3")
        "Meshes" = @("*.uasset", "*.fbx")
        "Animations" = @("*.uasset")
        "UI" = @("*.uasset")
    }

    foreach ($type in $assetTypes.Keys) {
        $typePath = Join-Path $contentPath $type
        if (Test-Path $typePath) {
            $files = Get-ChildItem -Path $typePath -Recurse -Include $assetTypes[$type] -ErrorAction SilentlyContinue
            Write-AssetLog "$type : $($files.Count) 个资产" "SUCCESS"
            $Script:AssetReport.Valid += @{Type=$type; Count=$files.Count; Path=$typePath}
        } else {
            Write-AssetLog "$type : 目录不存在" "WARNING"
            $Script:AssetReport.Missing += @{Type=$type; Path=$typePath}
        }
    }
}

function Test-AssetReferences {
    Write-AssetLog "检查资产引用..." "INFO"

    $blueprintPath = "$ProjectRoot/HorrorProject/Content/Blueprints"
    if (Test-Path $blueprintPath) {
        $blueprints = Get-ChildItem -Path $blueprintPath -Recurse -Filter "*.uasset"

        foreach ($bp in $blueprints) {
            # 检查蓝图文件大小
            if ($bp.Length -eq 0) {
                Write-AssetLog "空蓝图文件: $($bp.Name)" "ERROR"
                $Script:AssetReport.Broken += $bp.FullName
            } elseif ($bp.Length -lt 1KB) {
                Write-AssetLog "可疑的小蓝图文件: $($bp.Name) ($($bp.Length) bytes)" "WARNING"
            }
        }
    }
}

function Test-TextureIntegrity {
    Write-AssetLog "检查纹理完整性..." "INFO"

    $texturePath = "$ProjectRoot/HorrorProject/Content/Textures"
    if (Test-Path $texturePath) {
        $textures = Get-ChildItem -Path $texturePath -Recurse -Include *.png,*.jpg,*.tga,*.uasset

        $validTextures = 0
        $brokenTextures = 0

        foreach ($texture in $textures) {
            if ($texture.Length -eq 0) {
                Write-AssetLog "空纹理文件: $($texture.Name)" "ERROR"
                $Script:AssetReport.Broken += $texture.FullName
                $brokenTextures++
            } elseif ($texture.Extension -in @(".png",".jpg",".tga") -and $texture.Length -lt 100) {
                Write-AssetLog "可疑的纹理文件: $($texture.Name)" "WARNING"
                $brokenTextures++
            } else {
                $validTextures++
            }
        }

        Write-AssetLog "纹理检查: $validTextures 有效, $brokenTextures 可疑" "INFO"
    }
}

function Test-AudioIntegrity {
    Write-AssetLog "检查音频完整性..." "INFO"

    $audioPath = "$ProjectRoot/HorrorProject/Content/Audio"
    if (Test-Path $audioPath) {
        $audioFiles = Get-ChildItem -Path $audioPath -Recurse -Include *.wav,*.mp3,*.uasset

        $validAudio = 0
        $brokenAudio = 0

        foreach ($audio in $audioFiles) {
            if ($audio.Length -eq 0) {
                Write-AssetLog "空音频文件: $($audio.Name)" "ERROR"
                $Script:AssetReport.Broken += $audio.FullName
                $brokenAudio++
            } elseif ($audio.Extension -in @(".wav",".mp3") -and $audio.Length -lt 1KB) {
                Write-AssetLog "可疑的音频文件: $($audio.Name)" "WARNING"
                $brokenAudio++
            } else {
                $validAudio++
            }
        }

        Write-AssetLog "音频检查: $validAudio 有效, $brokenAudio 可疑" "INFO"
    }
}

function Test-MeshIntegrity {
    Write-AssetLog "检查网格完整性..." "INFO"

    $meshPath = "$ProjectRoot/HorrorProject/Content/Meshes"
    if (Test-Path $meshPath) {
        $meshes = Get-ChildItem -Path $meshPath -Recurse -Include *.uasset,*.fbx

        $validMeshes = 0
        $brokenMeshes = 0

        foreach ($mesh in $meshes) {
            if ($mesh.Length -eq 0) {
                Write-AssetLog "空网格文件: $($mesh.Name)" "ERROR"
                $Script:AssetReport.Broken += $mesh.FullName
                $brokenMeshes++
            } else {
                $validMeshes++
            }
        }

        Write-AssetLog "网格检查: $validMeshes 有效, $brokenMeshes 损坏" "INFO"
    }
}

function Find-OrphanedAssets {
    Write-AssetLog "查找孤立资产..." "INFO"

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) { return }

    # 查找没有被引用的资产（简化版）
    $allAssets = Get-ChildItem -Path $contentPath -Recurse -Include *.uasset
    $blueprints = Get-ChildItem -Path "$contentPath/Blueprints" -Recurse -Include *.uasset -ErrorAction SilentlyContinue

    Write-AssetLog "总资产数: $($allAssets.Count)" "INFO"
    Write-AssetLog "蓝图数: $($blueprints.Count)" "INFO"

    # 这里可以添加更复杂的引用检查逻辑
}

function Test-AssetNaming {
    Write-AssetLog "检查资产命名规范..." "INFO"

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) { return }

    $namingIssues = 0
    $allAssets = Get-ChildItem -Path $contentPath -Recurse -Include *.uasset

    foreach ($asset in $allAssets) {
        # 检查命名规范
        if ($asset.Name -match '\s') {
            Write-AssetLog "资产名包含空格: $($asset.Name)" "WARNING"
            $namingIssues++
        }
        if ($asset.Name -match '[^\x00-\x7F]') {
            Write-AssetLog "资产名包含非ASCII字符: $($asset.Name)" "WARNING"
            $namingIssues++
        }
    }

    if ($namingIssues -eq 0) {
        Write-AssetLog "资产命名规范检查通过" "SUCCESS"
    } else {
        Write-AssetLog "发现 $namingIssues 个命名问题" "WARNING"
    }
}

function Test-AssetSize {
    Write-AssetLog "检查资产大小..." "INFO"

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) { return }

    $largeAssets = @()
    $allFiles = Get-ChildItem -Path $contentPath -Recurse -File

    $totalSize = ($allFiles | Measure-Object -Property Length -Sum).Sum
    $totalSizeMB = [math]::Round($totalSize / 1MB, 2)

    Write-AssetLog "总资产大小: $totalSizeMB MB" "INFO"

    foreach ($file in $allFiles) {
        if ($file.Length -gt 100MB) {
            Write-AssetLog "大文件: $($file.Name) - $([math]::Round($file.Length/1MB, 2)) MB" "WARNING"
            $largeAssets += $file.FullName
        }
    }

    if ($largeAssets.Count -gt 0) {
        Write-AssetLog "发现 $($largeAssets.Count) 个大文件 (>100MB)" "WARNING"
    }
}

function Generate-AssetReport {
    Write-AssetLog "生成资产报告..." "INFO"

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $reportFile = Join-Path $ReportPath "AssetIntegrity_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        Summary = @{
            ValidAssets = $Script:AssetReport.Valid.Count
            MissingAssets = $Script:AssetReport.Missing.Count
            BrokenAssets = $Script:AssetReport.Broken.Count
            OrphanedAssets = $Script:AssetReport.Orphaned.Count
        }
        Details = $Script:AssetReport
        Status = if ($Script:AssetReport.Broken.Count -eq 0) { "HEALTHY" } elseif ($Script:AssetReport.Broken.Count -lt 10) { "NEEDS_ATTENTION" } else { "CRITICAL" }
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-AssetLog "报告已保存: $reportFile" "SUCCESS"
    return $report
}

function Show-AssetSummary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "资产完整性检查摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "有效资产: $($Report.Summary.ValidAssets)" -ForegroundColor Green
    Write-Host "缺失资产: $($Report.Summary.MissingAssets)" -ForegroundColor Yellow
    Write-Host "损坏资产: $($Report.Summary.BrokenAssets)" -ForegroundColor Red
    Write-Host "孤立资产: $($Report.Summary.OrphanedAssets)" -ForegroundColor Yellow
    Write-Host "状态: $($Report.Status)" -ForegroundColor $(if($Report.Status -eq "HEALTHY"){"Green"}elseif($Report.Status -eq "NEEDS_ATTENTION"){"Yellow"}else{"Red"})
    Write-Host "========================================`n" -ForegroundColor Cyan
}

# 主执行流程
Write-Host "`n开始资产完整性检查...`n" -ForegroundColor Cyan

Test-ContentAssets
Test-AssetReferences
Test-TextureIntegrity
Test-AudioIntegrity
Test-MeshIntegrity
Find-OrphanedAssets
Test-AssetNaming
Test-AssetSize

$report = Generate-AssetReport
Show-AssetSummary -Report $report

exit 0
