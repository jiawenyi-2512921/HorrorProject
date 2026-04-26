# ValidateAllAssets.ps1 - 验证所有资产
param(
    [Parameter(Mandatory=$true)]
    [string]$AssetPath,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AssetValidation.log",
    [switch]$FixIssues,
    [switch]$DetailedReport
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

function Get-AllAssets {
    Write-Log "扫描所有资产..."

    $Assets = @{
        Textures = Get-ChildItem -Path $AssetPath -Include "*.png","*.jpg","*.jpeg","*.tga" -Recurse -File
        Meshes = Get-ChildItem -Path $AssetPath -Include "*.fbx","*.obj" -Recurse -File
        Audio = Get-ChildItem -Path $AssetPath -Include "*.wav","*.mp3","*.ogg" -Recurse -File
        Materials = Get-ChildItem -Path $AssetPath -Include "*Material*.json" -Recurse -File
        Configs = Get-ChildItem -Path $AssetPath -Include "*.json" -Recurse -File
    }

    Write-Log "找到资产:"
    Write-Log "  纹理: $($Assets.Textures.Count)"
    Write-Log "  网格: $($Assets.Meshes.Count)"
    Write-Log "  音频: $($Assets.Audio.Count)"
    Write-Log "  材质: $($Assets.Materials.Count)"
    Write-Log "  配置: $($Assets.Configs.Count)"

    return $Assets
}

function Validate-Texture {
    param([string]$FilePath)

    $Issues = @()

    try {
        $FileInfo = Get-Item $FilePath

        # 检查文件大小
        if ($FileInfo.Length -gt 10MB) {
            $Issues += "文件过大: $([math]::Round($FileInfo.Length / 1MB, 2)) MB"
        }

        # 检查文件名
        if ($FileInfo.Name -match '\s') {
            $Issues += "文件名包含空格"
        }

        # 尝试获取图像信息
        try {
            $ImageInfo = & magick identify -format "%w %h" "$FilePath" 2>&1
            if ($ImageInfo -match "(\d+)\s+(\d+)") {
                $Width = [int]$matches[1]
                $Height = [int]$matches[2]

                # 检查尺寸是否为2的幂
                if (-not (($Width -band ($Width - 1)) -eq 0) -or -not (($Height -band ($Height - 1)) -eq 0)) {
                    $Issues += "尺寸不是2的幂: ${Width}x${Height}"
                }

                # 检查尺寸是否过大
                if ($Width -gt 4096 -or $Height -gt 4096) {
                    $Issues += "尺寸过大: ${Width}x${Height}"
                }
            }
        }
        catch {
            $Issues += "无法读取图像信息"
        }
    }
    catch {
        $Issues += "文件访问错误: $_"
    }

    return $Issues
}

function Validate-Mesh {
    param([string]$FilePath)

    $Issues = @()

    try {
        $FileInfo = Get-Item $FilePath

        # 检查文件大小
        if ($FileInfo.Length -gt 50MB) {
            $Issues += "文件过大: $([math]::Round($FileInfo.Length / 1MB, 2)) MB"
        }

        # 检查文件名
        if ($FileInfo.Name -match '\s') {
            $Issues += "文件名包含空格"
        }

        # 检查配置文件
        $ConfigFile = "$FilePath.import.json"
        if (-not (Test-Path $ConfigFile)) {
            $Issues += "缺少导入配置文件"
        }
    }
    catch {
        $Issues += "文件访问错误: $_"
    }

    return $Issues
}

function Validate-Audio {
    param([string]$FilePath)

    $Issues = @()

    try {
        $FileInfo = Get-Item $FilePath

        # 检查文件大小
        if ($FileInfo.Length -gt 20MB) {
            $Issues += "文件过大: $([math]::Round($FileInfo.Length / 1MB, 2)) MB"
        }

        # 检查文件名
        if ($FileInfo.Name -match '\s') {
            $Issues += "文件名包含空格"
        }

        # 检查音频信息
        try {
            $AudioInfo = & ffprobe -v quiet -print_format json -show_format -show_streams "$FilePath" 2>&1 | Out-String
            $Metadata = $AudioInfo | ConvertFrom-Json

            $AudioStream = $Metadata.streams | Where-Object { $_.codec_type -eq "audio" } | Select-Object -First 1

            if ($null -ne $AudioStream) {
                # 检查采样率
                if ($AudioStream.sample_rate -lt 44100) {
                    $Issues += "采样率过低: $($AudioStream.sample_rate) Hz"
                }

                # 检查比特率
                if ($Metadata.format.bit_rate -lt 128000) {
                    $Issues += "比特率过低: $([math]::Round($Metadata.format.bit_rate / 1000, 2)) kbps"
                }
            }
        }
        catch {
            $Issues += "无法读取音频信息"
        }
    }
    catch {
        $Issues += "文件访问错误: $_"
    }

    return $Issues
}

function Validate-Material {
    param([string]$FilePath)

    $Issues = @()

    try {
        $MaterialConfig = Get-Content -Path $FilePath -Raw | ConvertFrom-Json

        # 检查必需字段
        if (-not $MaterialConfig.Type) {
            $Issues += "缺少Type字段"
        }

        if (-not $MaterialConfig.Parameters) {
            $Issues += "缺少Parameters字段"
        }

        # 检查纹理引用
        if ($MaterialConfig.Parameters.TextureParameters) {
            foreach ($Texture in $MaterialConfig.Parameters.TextureParameters) {
                if ($Texture.Value -and $Texture.Value.StartsWith("/Game/")) {
                    # 检查纹理路径是否存在
                    # 这里简化处理
                }
            }
        }
    }
    catch {
        $Issues += "JSON解析错误: $_"
    }

    return $Issues
}

function Validate-Assets {
    param($Assets)

    Write-Log "开始验证资产..."

    $Results = @{
        Textures = @()
        Meshes = @()
        Audio = @()
        Materials = @()
    }

    $TotalAssets = $Assets.Textures.Count + $Assets.Meshes.Count + $Assets.Audio.Count + $Assets.Materials.Count
    $CurrentIndex = 0

    # 验证纹理
    foreach ($Texture in $Assets.Textures) {
        $CurrentIndex++
        $ProgressPercent = [math]::Round(($CurrentIndex / $TotalAssets) * 100, 2)
        Write-Progress -Activity "验证资产" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $Issues = Validate-Texture -FilePath $Texture.FullName

        if ($Issues.Count -gt 0) {
            Write-Log "纹理问题: $($Texture.Name) - $($Issues.Count) 个问题" "WARNING"
        }

        $Results.Textures += @{
            File = $Texture.FullName
            Name = $Texture.Name
            Issues = $Issues
            Valid = ($Issues.Count -eq 0)
        }
    }

    # 验证网格
    foreach ($Mesh in $Assets.Meshes) {
        $CurrentIndex++
        $ProgressPercent = [math]::Round(($CurrentIndex / $TotalAssets) * 100, 2)
        Write-Progress -Activity "验证资产" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $Issues = Validate-Mesh -FilePath $Mesh.FullName

        if ($Issues.Count -gt 0) {
            Write-Log "网格问题: $($Mesh.Name) - $($Issues.Count) 个问题" "WARNING"
        }

        $Results.Meshes += @{
            File = $Mesh.FullName
            Name = $Mesh.Name
            Issues = $Issues
            Valid = ($Issues.Count -eq 0)
        }
    }

    # 验证音频
    foreach ($Audio in $Assets.Audio) {
        $CurrentIndex++
        $ProgressPercent = [math]::Round(($CurrentIndex / $TotalAssets) * 100, 2)
        Write-Progress -Activity "验证资产" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $Issues = Validate-Audio -FilePath $Audio.FullName

        if ($Issues.Count -gt 0) {
            Write-Log "音频问题: $($Audio.Name) - $($Issues.Count) 个问题" "WARNING"
        }

        $Results.Audio += @{
            File = $Audio.FullName
            Name = $Audio.Name
            Issues = $Issues
            Valid = ($Issues.Count -eq 0)
        }
    }

    # 验证材质
    foreach ($Material in $Assets.Materials) {
        $CurrentIndex++
        $ProgressPercent = [math]::Round(($CurrentIndex / $TotalAssets) * 100, 2)
        Write-Progress -Activity "验证资产" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $Issues = Validate-Material -FilePath $Material.FullName

        if ($Issues.Count -gt 0) {
            Write-Log "材质问题: $($Material.Name) - $($Issues.Count) 个问题" "WARNING"
        }

        $Results.Materials += @{
            File = $Material.FullName
            Name = $Material.Name
            Issues = $Issues
            Valid = ($Issues.Count -eq 0)
        }
    }

    Write-Progress -Activity "验证资产" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成验证报告..."

    $TotalTextures = $Results.Textures.Count
    $ValidTextures = ($Results.Textures | Where-Object { $_.Valid }).Count
    $TotalMeshes = $Results.Meshes.Count
    $ValidMeshes = ($Results.Meshes | Where-Object { $_.Valid }).Count
    $TotalAudio = $Results.Audio.Count
    $ValidAudio = ($Results.Audio | Where-Object { $_.Valid }).Count
    $TotalMaterials = $Results.Materials.Count
    $ValidMaterials = ($Results.Materials | Where-Object { $_.Valid }).Count

    $TotalAssets = $TotalTextures + $TotalMeshes + $TotalAudio + $TotalMaterials
    $ValidAssets = $ValidTextures + $ValidMeshes + $ValidAudio + $ValidMaterials

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AssetValidation_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
资产验证报告
========================================
验证时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
资产路径: $AssetPath

总体统计:
- 总资产数: $TotalAssets
- 有效资产: $ValidAssets
- 问题资产: $($TotalAssets - $ValidAssets)
- 有效率: $([math]::Round(($ValidAssets / $TotalAssets) * 100, 2))%

分类统计:
纹理: $ValidTextures / $TotalTextures 有效
网格: $ValidMeshes / $TotalMeshes 有效
音频: $ValidAudio / $TotalAudio 有效
材质: $ValidMaterials / $TotalMaterials 有效

纹理问题:
$($Results.Textures | Where-Object { -not $_.Valid } | ForEach-Object {
    "  ✗ $($_.Name)"
    foreach ($Issue in $_.Issues) {
        "    - $Issue"
    }
    ""
} | Out-String)

网格问题:
$($Results.Meshes | Where-Object { -not $_.Valid } | ForEach-Object {
    "  ✗ $($_.Name)"
    foreach ($Issue in $_.Issues) {
        "    - $Issue"
    }
    ""
} | Out-String)

音频问题:
$($Results.Audio | Where-Object { -not $_.Valid } | ForEach-Object {
    "  ✗ $($_.Name)"
    foreach ($Issue in $_.Issues) {
        "    - $Issue"
    }
    ""
} | Out-String)

材质问题:
$($Results.Materials | Where-Object { -not $_.Valid } | ForEach-Object {
    "  ✗ $($_.Name)"
    foreach ($Issue in $_.Issues) {
        "    - $Issue"
    }
    ""
} | Out-String)

执行时间: $((Get-Date) - $StartTime)
========================================
"@

    $Report | Out-File -FilePath $ReportPath -Encoding UTF8
    Write-Log "报告已生成: $ReportPath"

    return $ReportPath
}

# 主执行流程
try {
    Write-Log "========================================" "INFO"
    Write-Log "开始验证所有资产" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $AssetPath)) {
        throw "资产路径不存在: $AssetPath"
    }

    $Assets = Get-AllAssets
    $Results = Validate-Assets -Assets $Assets
    $ReportPath = Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "资产验证完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "报告路径: $ReportPath" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "资产验证失败: $_" "ERROR"
    exit 1
}
