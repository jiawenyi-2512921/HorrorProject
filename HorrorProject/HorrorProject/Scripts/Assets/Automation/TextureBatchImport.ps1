# TextureBatchImport.ps1 - 批量导入纹理资产
param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath,
    [Parameter(Mandatory=$true)]
    [string]$DestinationPath,
    [string]$UProjectPath = "D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject",
    [ValidateSet("Default", "Normal", "UI", "HDR")]
    [string]$TextureType = "Default",
    [switch]$GenerateMipmaps = $true,
    [switch]$sRGB = $true,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\TextureImport.log"
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

function Get-TextureSettings {
    param([string]$Type)

    switch ($Type) {
        "Normal" {
            return @{
                CompressionSettings = "TC_Normalmap"
                sRGB = $false
                LODGroup = "TEXTUREGROUP_WorldNormalMap"
            }
        }
        "UI" {
            return @{
                CompressionSettings = "TC_EditorIcon"
                sRGB = $true
                LODGroup = "TEXTUREGROUP_UI"
            }
        }
        "HDR" {
            return @{
                CompressionSettings = "TC_HDR"
                sRGB = $false
                LODGroup = "TEXTUREGROUP_World"
            }
        }
        default {
            return @{
                CompressionSettings = "TC_Default"
                sRGB = $sRGB
                LODGroup = "TEXTUREGROUP_World"
            }
        }
    }
}

function Get-TextureFiles {
    Write-Log "扫描纹理文件..."

    $TextureExtensions = @("*.png", "*.jpg", "*.jpeg", "*.tga", "*.bmp", "*.exr", "*.hdr")
    $TextureFiles = @()

    foreach ($ext in $TextureExtensions) {
        $Files = Get-ChildItem -Path $SourcePath -Filter $ext -Recurse -File
        $TextureFiles += $Files
    }

    Write-Log "找到 $($TextureFiles.Count) 个纹理文件"
    return $TextureFiles
}

function Import-TextureFile {
    param(
        [System.IO.FileInfo]$TextureFile,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "导入纹理文件" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "导入纹理 [$Index/$Total]: $($TextureFile.Name)"

    $RelativePath = $TextureFile.FullName.Substring($SourcePath.Length).TrimStart('\', '/')
    $TargetPath = Join-Path $DestinationPath $RelativePath
    $TargetDir = Split-Path $TargetPath -Parent

    if (-not (Test-Path $TargetDir)) {
        New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
    }

    try {
        # 复制纹理文件
        Copy-Item -Path $TextureFile.FullName -Destination $TargetPath -Force

        # 获取纹理设置
        $Settings = Get-TextureSettings -Type $TextureType

        # 创建纹理导入配置文件
        $ConfigFile = "$TargetPath.import.json"
        $ImportConfig = @{
            TextureType = $TextureType
            CompressionSettings = $Settings.CompressionSettings
            sRGB = $Settings.sRGB
            LODGroup = $Settings.LODGroup
            MipGenSettings = if ($GenerateMipmaps) { "TMGS_FromTextureGroup" } else { "TMGS_NoMipmaps" }
        }

        $ImportConfig | ConvertTo-Json | Out-File -FilePath $ConfigFile -Encoding UTF8

        Write-Log "成功导入: $($TextureFile.Name)" "SUCCESS"

        return @{
            Success = $true
            File = $TextureFile.Name
            Size = $TextureFile.Length
            Path = $TargetPath
            Type = $TextureType
        }
    }
    catch {
        Write-Log "导入失败: $($TextureFile.Name) - $_" "ERROR"
        return @{
            Success = $false
            File = $TextureFile.Name
            Error = $_.Exception.Message
        }
    }
}

function Generate-ImportReport {
    param($Results)

    Write-Log "生成导入报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count
    $TotalSize = ($Results | Where-Object { $_.Success } | Measure-Object -Property Size -Sum).Sum

    $TypeStats = $Results | Where-Object { $_.Success } | Group-Object -Property Type | Select-Object Name, Count

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\TextureImport_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
纹理批量导入报告
========================================
导入时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
源路径: $SourcePath
目标路径: $DestinationPath
纹理类型: $TextureType

统计信息:
- 总文件数: $($Results.Count)
- 成功导入: $SuccessCount
- 导入失败: $FailCount
- 总大小: $([math]::Round($TotalSize / 1MB, 2)) MB

类型分布:
$($TypeStats | ForEach-Object { "  - $($_.Name): $($_.Count) 个文件" } | Out-String)

成功导入的文件:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $($_.File) - $([math]::Round($_.Size / 1KB, 2)) KB [$($_.Type)]"
} | Out-String)

失败的文件:
$($Results | Where-Object { -not $_.Success } | ForEach-Object {
    "  ✗ $($_.File) - $($_.Error)"
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
    Write-Log "开始纹理批量导入" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $SourcePath)) {
        throw "源路径不存在: $SourcePath"
    }

    $TextureFiles = Get-TextureFiles

    if ($TextureFiles.Count -eq 0) {
        Write-Log "未找到纹理文件，退出" "WARNING"
        exit 0
    }

    $Results = @()
    $Index = 0

    foreach ($TextureFile in $TextureFiles) {
        $Index++
        $Result = Import-TextureFile -TextureFile $TextureFile -Index $Index -Total $TextureFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "导入纹理文件" -Completed

    Generate-ImportReport -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "纹理导入完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "纹理导入失败: $_" "ERROR"
    exit 1
}
