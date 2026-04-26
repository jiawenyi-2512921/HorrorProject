# TextureCompression.ps1 - 纹理压缩优化
param(
    [Parameter(Mandatory=$true)]
    [string]$TexturePath,
    [ValidateSet("High", "Medium", "Low")]
    [string]$Quality = "Medium",
    [ValidateSet("BC1", "BC3", "BC5", "BC7", "ASTC")]
    [string]$CompressionFormat = "BC7",
    [int]$MaxTextureSize = 2048,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\TextureCompression.log",
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

function Get-CompressionSettings {
    param([string]$Quality, [string]$Format)

    $Settings = @{
        BC1 = @{ High = 100; Medium = 85; Low = 70 }
        BC3 = @{ High = 100; Medium = 85; Low = 70 }
        BC5 = @{ High = 100; Medium = 90; Low = 75 }
        BC7 = @{ High = 100; Medium = 90; Low = 75 }
        ASTC = @{ High = "4x4"; Medium = "6x6"; Low = "8x8" }
    }

    return $Settings[$Format][$Quality]
}

function Get-TextureInfo {
    param([string]$FilePath)

    try {
        $ImageInfo = & magick identify -format "%w %h %b" "$FilePath" 2>&1

        if ($ImageInfo -match "(\d+)\s+(\d+)\s+(.+)") {
            return @{
                Width = [int]$matches[1]
                Height = [int]$matches[2]
                Size = $matches[3]
            }
        }
    }
    catch {
        Write-Log "无法获取纹理信息: $_" "ERROR"
    }

    return $null
}

function Compress-TextureFile {
    param(
        [string]$FilePath,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "压缩纹理" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "压缩纹理 [$Index/$Total]: $(Split-Path $FilePath -Leaf)"

    $OriginalInfo = Get-TextureInfo -FilePath $FilePath

    if ($null -eq $OriginalInfo) {
        return @{ Success = $false; File = $FilePath; Error = "无法读取纹理信息" }
    }

    $OriginalSize = (Get-Item $FilePath).Length

    # 检查是否需要调整大小
    $NeedsResize = $OriginalInfo.Width -gt $MaxTextureSize -or $OriginalInfo.Height -gt $MaxTextureSize

    $TempFile = "$FilePath.temp.png"
    $BackupFile = "$FilePath.backup"

    try {
        # 备份原文件
        Copy-Item -Path $FilePath -Destination $BackupFile -Force

        # 调整大小（如果需要）
        if ($NeedsResize) {
            Write-Log "调整纹理大小: $($OriginalInfo.Width)x$($OriginalInfo.Height) -> ${MaxTextureSize}x${MaxTextureSize}"

            $ResizeArgs = @(
                "$FilePath",
                "-resize", "${MaxTextureSize}x${MaxTextureSize}>",
                "-quality", "$(Get-CompressionSettings -Quality $Quality -Format $CompressionFormat)",
                "$TempFile"
            )

            & magick @ResizeArgs 2>&1 | Out-Null
        }
        else {
            # 仅压缩
            $CompressArgs = @(
                "$FilePath",
                "-quality", "$(Get-CompressionSettings -Quality $Quality -Format $CompressionFormat)",
                "$TempFile"
            )

            & magick @CompressArgs 2>&1 | Out-Null
        }

        if (Test-Path $TempFile) {
            $NewSize = (Get-Item $TempFile).Length
            $NewInfo = Get-TextureInfo -FilePath $TempFile

            # 只有在压缩后文件更小时才替换
            if ($NewSize -lt $OriginalSize) {
                Move-Item -Path $TempFile -Destination $FilePath -Force
                Remove-Item -Path $BackupFile -Force

                $CompressionRatio = [math]::Round((1 - ($NewSize / $OriginalSize)) * 100, 2)

                Write-Log "压缩成功: $([math]::Round($OriginalSize/1KB, 2)) KB -> $([math]::Round($NewSize/1KB, 2)) KB (节省 $CompressionRatio%)" "SUCCESS"

                return @{
                    Success = $true
                    File = $FilePath
                    OriginalSize = $OriginalSize
                    NewSize = $NewSize
                    OriginalDimensions = "$($OriginalInfo.Width)x$($OriginalInfo.Height)"
                    NewDimensions = "$($NewInfo.Width)x$($NewInfo.Height)"
                    CompressionRatio = $CompressionRatio
                }
            }
            else {
                # 恢复原文件
                Move-Item -Path $BackupFile -Destination $FilePath -Force
                Remove-Item -Path $TempFile -Force

                Write-Log "压缩后文件更大，保持原文件" "INFO"

                return @{
                    Success = $true
                    File = $FilePath
                    Skipped = $true
                    Reason = "压缩后文件更大"
                }
            }
        }
        else {
            throw "压缩失败，未生成输出文件"
        }
    }
    catch {
        Write-Log "压缩失败: $_" "ERROR"

        if (Test-Path $BackupFile) {
            Move-Item -Path $BackupFile -Destination $FilePath -Force
        }
        if (Test-Path $TempFile) {
            Remove-Item -Path $TempFile -Force
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
        $Result = Compress-TextureFile -FilePath $TextureFile.FullName -Index $Index -Total $TextureFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "压缩纹理" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成压缩报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success -and -not $_.Skipped }).Count
    $SkippedCount = ($Results | Where-Object { $_.Skipped }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count

    $TotalOriginalSize = ($Results | Where-Object { $_.Success -and -not $_.Skipped } | Measure-Object -Property OriginalSize -Sum).Sum
    $TotalNewSize = ($Results | Where-Object { $_.Success -and -not $_.Skipped } | Measure-Object -Property NewSize -Sum).Sum
    $TotalSaved = $TotalOriginalSize - $TotalNewSize
    $OverallRatio = if ($TotalOriginalSize -gt 0) { [math]::Round(($TotalSaved / $TotalOriginalSize) * 100, 2) } else { 0 }

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\TextureCompression_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
纹理压缩报告
========================================
压缩时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
质量设置: $Quality
压缩格式: $CompressionFormat
最大尺寸: ${MaxTextureSize}x${MaxTextureSize}

统计信息:
- 总文件数: $($Results.Count)
- 成功压缩: $SuccessCount
- 跳过处理: $SkippedCount
- 压缩失败: $FailCount
- 原始大小: $([math]::Round($TotalOriginalSize / 1MB, 2)) MB
- 压缩后大小: $([math]::Round($TotalNewSize / 1MB, 2)) MB
- 节省空间: $([math]::Round($TotalSaved / 1MB, 2)) MB ($OverallRatio%)

压缩详情:
$($Results | Where-Object { $_.Success -and -not $_.Skipped } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf)"
    "    尺寸: $($_.OriginalDimensions) -> $($_.NewDimensions)"
    "    大小: $([math]::Round($_.OriginalSize/1KB, 2)) KB -> $([math]::Round($_.NewSize/1KB, 2)) KB"
    "    节省: $($_.CompressionRatio)%"
    ""
} | Out-String)

跳过的文件:
$($Results | Where-Object { $_.Skipped } | ForEach-Object {
    "  - $(Split-Path $_.File -Leaf): $($_.Reason)"
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
    Write-Log "开始纹理压缩" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $TexturePath)) {
        throw "纹理路径不存在: $TexturePath"
    }

    $Results = Process-Textures

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "纹理压缩完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "纹理压缩失败: $_" "ERROR"
    exit 1
}
