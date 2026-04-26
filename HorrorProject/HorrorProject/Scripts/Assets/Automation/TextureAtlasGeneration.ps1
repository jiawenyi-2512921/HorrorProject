# TextureAtlasGeneration.ps1 - 纹理图集生成
param(
    [Parameter(Mandatory=$true)]
    [string]$TexturePath,
    [Parameter(Mandatory=$true)]
    [string]$OutputPath,
    [int]$AtlasSize = 2048,
    [int]$Padding = 2,
    [string]$AtlasName = "TextureAtlas",
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AtlasGeneration.log",
    [switch]$GenerateMetadata
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

function Get-TextureInfo {
    param([string]$FilePath)

    try {
        $ImageInfo = & magick identify -format "%w %h %b" "$FilePath" 2>&1

        if ($ImageInfo -match "(\d+)\s+(\d+)\s+(.+)") {
            return @{
                Width = [int]$matches[1]
                Height = [int]$matches[2]
                Size = $matches[3]
                Path = $FilePath
                Name = [System.IO.Path]::GetFileNameWithoutExtension($FilePath)
            }
        }
    }
    catch {
        Write-Log "无法获取纹理信息: $_" "ERROR"
    }

    return $null
}

function Pack-Textures {
    param([array]$Textures, [int]$AtlasWidth, [int]$AtlasHeight)

    Write-Log "开始打包纹理到图集..."

    # 按面积排序（从大到小）
    $SortedTextures = $Textures | Sort-Object { $_.Width * $_.Height } -Descending

    $PackedTextures = @()
    $UsedRects = @()

    foreach ($Texture in $SortedTextures) {
        $Placed = $false

        # 尝试放置纹理
        for ($y = 0; $y -le ($AtlasHeight - $Texture.Height); $y += 1) {
            for ($x = 0; $x -le ($AtlasWidth - $Texture.Width); $x += 1) {
                $NewRect = @{
                    X = $x
                    Y = $y
                    Width = $Texture.Width + $Padding
                    Height = $Texture.Height + $Padding
                }

                # 检查是否与已放置的矩形重叠
                $Overlaps = $false
                foreach ($UsedRect in $UsedRects) {
                    if (Test-RectOverlap -Rect1 $NewRect -Rect2 $UsedRect) {
                        $Overlaps = $true
                        break
                    }
                }

                if (-not $Overlaps) {
                    $PackedTextures += @{
                        Texture = $Texture
                        X = $x
                        Y = $y
                    }
                    $UsedRects += $NewRect
                    $Placed = $true
                    break
                }
            }

            if ($Placed) { break }
        }

        if (-not $Placed) {
            Write-Log "无法放置纹理: $($Texture.Name)" "WARNING"
        }
    }

    Write-Log "成功打包 $($PackedTextures.Count)/$($Textures.Count) 个纹理"

    return $PackedTextures
}

function Test-RectOverlap {
    param($Rect1, $Rect2)

    return -not (
        $Rect1.X + $Rect1.Width -le $Rect2.X -or
        $Rect2.X + $Rect2.Width -le $Rect1.X -or
        $Rect1.Y + $Rect1.Height -le $Rect2.Y -or
        $Rect2.Y + $Rect2.Height -le $Rect1.Y
    )
}

function Generate-Atlas {
    param([array]$PackedTextures, [string]$OutputFile)

    Write-Log "生成图集图像..."

    try {
        # 创建空白图集
        $CreateArgs = @(
            "-size", "${AtlasSize}x${AtlasSize}",
            "xc:transparent",
            "$OutputFile"
        )

        & magick @CreateArgs 2>&1 | Out-Null

        # 将每个纹理合成到图集
        foreach ($Item in $PackedTextures) {
            Write-Log "  添加纹理: $($Item.Texture.Name) at ($($Item.X), $($Item.Y))"

            $CompositeArgs = @(
                "$OutputFile",
                "$($Item.Texture.Path)",
                "-geometry", "+$($Item.X)+$($Item.Y)",
                "-composite",
                "$OutputFile"
            )

            & magick @CompositeArgs 2>&1 | Out-Null
        }

        Write-Log "图集生成成功: $OutputFile" "SUCCESS"
        return $true
    }
    catch {
        Write-Log "生成图集失败: $_" "ERROR"
        return $false
    }
}

function Generate-AtlasMetadata {
    param([array]$PackedTextures, [string]$MetadataFile)

    Write-Log "生成图集元数据..."

    $Metadata = @{
        AtlasName = $AtlasName
        AtlasSize = @{ Width = $AtlasSize; Height = $AtlasSize }
        Padding = $Padding
        TextureCount = $PackedTextures.Count
        GeneratedDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        Textures = @()
    }

    foreach ($Item in $PackedTextures) {
        $Metadata.Textures += @{
            Name = $Item.Texture.Name
            X = $Item.X
            Y = $Item.Y
            Width = $Item.Texture.Width
            Height = $Item.Texture.Height
            UV = @{
                U0 = [math]::Round($Item.X / $AtlasSize, 6)
                V0 = [math]::Round($Item.Y / $AtlasSize, 6)
                U1 = [math]::Round(($Item.X + $Item.Texture.Width) / $AtlasSize, 6)
                V1 = [math]::Round(($Item.Y + $Item.Texture.Height) / $AtlasSize, 6)
            }
        }
    }

    $Metadata | ConvertTo-Json -Depth 10 | Out-File -FilePath $MetadataFile -Encoding UTF8
    Write-Log "元数据已生成: $MetadataFile"
}

function Process-AtlasGeneration {
    Write-Log "扫描纹理文件..."

    $TextureFiles = Get-ChildItem -Path $TexturePath -Include "*.png","*.jpg","*.jpeg","*.tga" -File

    Write-Log "找到 $($TextureFiles.Count) 个纹理文件"

    if ($TextureFiles.Count -eq 0) {
        Write-Log "未找到纹理文件" "WARNING"
        return $null
    }

    # 获取所有纹理信息
    $Textures = @()
    foreach ($TextureFile in $TextureFiles) {
        $Info = Get-TextureInfo -FilePath $TextureFile.FullName
        if ($null -ne $Info) {
            $Textures += $Info
        }
    }

    Write-Log "成功读取 $($Textures.Count) 个纹理信息"

    # 打包纹理
    $PackedTextures = Pack-Textures -Textures $Textures -AtlasWidth $AtlasSize -AtlasHeight $AtlasSize

    if ($PackedTextures.Count -eq 0) {
        Write-Log "没有纹理被打包" "WARNING"
        return $null
    }

    # 创建输出目录
    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    # 生成图集
    $AtlasFile = Join-Path $OutputPath "$AtlasName.png"
    $Success = Generate-Atlas -PackedTextures $PackedTextures -OutputFile $AtlasFile

    if (-not $Success) {
        return $null
    }

    # 生成元数据
    if ($GenerateMetadata) {
        $MetadataFile = Join-Path $OutputPath "$AtlasName.json"
        Generate-AtlasMetadata -PackedTextures $PackedTextures -MetadataFile $MetadataFile
    }

    return @{
        AtlasFile = $AtlasFile
        PackedCount = $PackedTextures.Count
        TotalCount = $Textures.Count
        PackedTextures = $PackedTextures
    }
}

function Generate-Report {
    param($Result)

    Write-Log "生成图集报告..."

    $AtlasSize = (Get-Item $Result.AtlasFile).Length
    $Efficiency = [math]::Round(($Result.PackedCount / $Result.TotalCount) * 100, 2)

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AtlasGeneration_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
纹理图集生成报告
========================================
生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
源路径: $TexturePath
输出路径: $OutputPath
图集名称: $AtlasName

图集信息:
- 图集尺寸: ${AtlasSize}x${AtlasSize}
- 图集大小: $([math]::Round($AtlasSize / 1MB, 2)) MB
- 填充间距: $Padding px

统计信息:
- 总纹理数: $($Result.TotalCount)
- 打包纹理数: $($Result.PackedCount)
- 打包效率: $Efficiency%

打包详情:
$($Result.PackedTextures | ForEach-Object {
    "  ✓ $($_.Texture.Name)"
    "    位置: ($($_.X), $($_.Y))"
    "    尺寸: $($_.Texture.Width)x$($_.Texture.Height)"
    ""
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
    Write-Log "开始生成纹理图集" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $TexturePath)) {
        throw "纹理路径不存在: $TexturePath"
    }

    $Result = Process-AtlasGeneration

    if ($null -ne $Result) {
        Generate-Report -Result $Result
    }

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "纹理图集生成完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "生成纹理图集失败: $_" "ERROR"
    exit 1
}
