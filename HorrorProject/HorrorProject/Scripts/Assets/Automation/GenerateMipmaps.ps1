# GenerateMipmaps.ps1 - 生成纹理Mipmap
param(
    [Parameter(Mandatory=$true)]
    [string]$TexturePath,
    [ValidateSet("Box", "Triangle", "Kaiser", "Lanczos")]
    [string]$FilterType = "Kaiser",
    [int]$MaxMipLevels = 0,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MipmapGeneration.log",
    [switch]$ProcessSubfolders
)

$ErrorActionPreference = "Stop"
$StartTime = Get-Date

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $LogMessage = "[$Timestamp] [$Level] $Message"
    Write-Host $LogMessage
    Add-Content -Path $LogPath -Value $LogMessage
}

function Get-MipLevelCount {
    param([int]$Width, [int]$Height)

    $MaxDimension = [math]::Max($Width, $Height)
    $MipLevels = [math]::Floor([math]::Log($MaxDimension, 2)) + 1

    if ($MaxMipLevels -gt 0 -and $MipLevels -gt $MaxMipLevels) {
        return $MaxMipLevels
    }

    return $MipLevels
}

function Get-TextureDimensions {
    param([string]$FilePath)

    try {
        $ImageInfo = & magick identify -format "%w %h" "$FilePath" 2>&1

        if ($ImageInfo -match "(\d+)\s+(\d+)") {
            return @{
                Width = [int]$matches[1]
                Height = [int]$matches[2]
            }
        }
    }
    catch {
        Write-Log "无法获取纹理尺寸: $_" "ERROR"
    }

    return $null
}

function Generate-MipmapChain {
    param(
        [string]$FilePath,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "生成Mipmap" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "生成Mipmap [$Index/$Total]: $(Split-Path $FilePath -Leaf)"

    $Dimensions = Get-TextureDimensions -FilePath $FilePath

    if ($null -eq $Dimensions) {
        return @{ Success = $false; File = $FilePath; Error = "无法读取纹理尺寸" }
    }

    $MipLevels = Get-MipLevelCount -Width $Dimensions.Width -Height $Dimensions.Height

    Write-Log "纹理尺寸: $($Dimensions.Width)x$($Dimensions.Height), Mip级别: $MipLevels"

    $FileName = [System.IO.Path]::GetFileNameWithoutExtension($FilePath)
    $FileDir = Split-Path $FilePath -Parent
    $FileExt = [System.IO.Path]::GetExtension($FilePath)

    $MipmapDir = Join-Path $FileDir "Mipmaps"
    if (-not (Test-Path $MipmapDir)) {
        New-Item -ItemType Directory -Path $MipmapDir -Force | Out-Null
    }

    $GeneratedMips = @()

    try {
        for ($i = 1; $i -lt $MipLevels; $i++) {
            $MipWidth = [math]::Max(1, [math]::Floor($Dimensions.Width / [math]::Pow(2, $i)))
            $MipHeight = [math]::Max(1, [math]::Floor($Dimensions.Height / [math]::Pow(2, $i)))

            $MipFile = Join-Path $MipmapDir "${FileName}_mip${i}${FileExt}"

            Write-Log "  生成Mip $i : ${MipWidth}x${MipHeight}"

            $FilterArg = switch ($FilterType) {
                "Box" { "box" }
                "Triangle" { "triangle" }
                "Kaiser" { "kaiser" }
                "Lanczos" { "lanczos" }
                default { "kaiser" }
            }

            $MagickArgs = @(
                "$FilePath",
                "-filter", $FilterArg,
                "-resize", "${MipWidth}x${MipHeight}!",
                "$MipFile"
            )

            & magick @MagickArgs 2>&1 | Out-Null

            if (Test-Path $MipFile) {
                $MipSize = (Get-Item $MipFile).Length
                $GeneratedMips += @{
                    Level = $i
                    Width = $MipWidth
                    Height = $MipHeight
                    Size = $MipSize
                    Path = $MipFile
                }
            }
            else {
                throw "生成Mip $i 失败"
            }
        }

        Write-Log "成功生成 $($GeneratedMips.Count) 个Mip级别" "SUCCESS"

        return @{
            Success = $true
            File = $FilePath
            OriginalDimensions = "$($Dimensions.Width)x$($Dimensions.Height)"
            MipLevels = $MipLevels
            GeneratedMips = $GeneratedMips
        }
    }
    catch {
        Write-Log "生成Mipmap失败: $_" "ERROR"

        # 清理已生成的Mip文件
        foreach ($Mip in $GeneratedMips) {
            if (Test-Path $Mip.Path) {
                Remove-Item -Path $Mip.Path -Force
            }
        }

        return @{ Success = $false; File = $FilePath; Error = $_.Exception.Message }
    }
}

function Process-Textures {
    Write-Log "扫描纹理文件..."

    $SearchOption = if ($ProcessSubfolders) { "-Recurse" } else { "" }
    $TextureFiles = Get-ChildItem -Path $TexturePath -Include "*.png","*.jpg","*.jpeg","*.tga" -File @SearchOption

    Write-Log "找到 $($TextureFiles.Count) 个纹理文件"

    $Results = @()
    $Index = 0

    foreach ($TextureFile in $TextureFiles) {
        $Index++
        $Result = Generate-MipmapChain -FilePath $TextureFile.FullName -Index $Index -Total $TextureFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "生成Mipmap" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成Mipmap报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count
    $TotalMips = ($Results | Where-Object { $_.Success } | ForEach-Object { $_.GeneratedMips.Count } | Measure-Object -Sum).Sum

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MipmapGeneration_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
Mipmap生成报告
========================================
生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
纹理路径: $TexturePath
过滤类型: $FilterType
最大Mip级别: $(if ($MaxMipLevels -eq 0) { "自动" } else { $MaxMipLevels })

统计信息:
- 总文件数: $($Results.Count)
- 成功生成: $SuccessCount
- 生成失败: $FailCount
- 总Mip数: $TotalMips

生成详情:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf)"
    "    原始尺寸: $($_.OriginalDimensions)"
    "    Mip级别: $($_.MipLevels)"
    "    生成的Mip:"
    foreach ($Mip in $_.GeneratedMips) {
        "      Mip $($Mip.Level): $($Mip.Width)x$($Mip.Height) - $([math]::Round($Mip.Size/1KB, 2)) KB"
    }
    ""
} | Out-String)

失败的文件:
$($Results | Where-Object { -not $_.Success } | ForEach-Object {
    "  ✗ $(Split-Path $_.File -Leaf): $($_.Error)"
} | Out-String)

执行时间: $((Get-Date) - $StartTime)
========================================
"@

    $Report | Out-File -FilePath $ReportPath -Encoding UTF8
    Write-Log "报告已生成: $ReportPath"
}

# 主执行流程
try {
    Write-Log "========================================" "INFO"
    Write-Log "开始生成Mipmap" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $TexturePath)) {
        throw "纹理路径不存在: $TexturePath"
    }

    $Results = Process-Textures

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "Mipmap生成完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "生成Mipmap失败: $_" "ERROR"
    exit 1
}
