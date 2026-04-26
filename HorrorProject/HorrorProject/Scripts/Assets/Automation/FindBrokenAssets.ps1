# FindBrokenAssets.ps1 - 查找损坏资产
param(
    [Parameter(Mandatory=$true)]
    [string]$AssetPath,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\BrokenAssets.log",
    [switch]$AttemptRepair,
    [switch]$Quarantine
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

function Test-TextureIntegrity {
    param([string]$FilePath)

    try {
        $Result = & magick identify "$FilePath" 2>&1
        if ($LASTEXITCODE -ne 0) {
            return @{ Broken = $true; Reason = "无法识别图像格式" }
        }

        # 尝试读取图像数据
        $TestResult = & magick "$FilePath" -format "%w %h" info: 2>&1
        if ($LASTEXITCODE -ne 0) {
            return @{ Broken = $true; Reason = "图像数据损坏" }
        }

        return @{ Broken = $false }
    }
    catch {
        return @{ Broken = $true; Reason = "测试失败: $_" }
    }
}

function Test-MeshIntegrity {
    param([string]$FilePath)

    try {
        $FileInfo = Get-Item $FilePath

        # 检查文件大小
        if ($FileInfo.Length -eq 0) {
            return @{ Broken = $true; Reason = "文件为空" }
        }

        # 检查文件头
        $Header = Get-Content -Path $FilePath -Encoding Byte -TotalCount 100 -ErrorAction SilentlyContinue

        if ($null -eq $Header -or $Header.Length -eq 0) {
            return @{ Broken = $true; Reason = "无法读取文件头" }
        }

        # 检查FBX文件头
        if ($FilePath -match '\.fbx$') {
            $HeaderString = [System.Text.Encoding]::ASCII.GetString($Header)
            if ($HeaderString -notmatch 'Kaydara FBX Binary') {
                return @{ Broken = $true; Reason = "FBX文件头损坏" }
            }
        }

        return @{ Broken = $false }
    }
    catch {
        return @{ Broken = $true; Reason = "测试失败: $_" }
    }
}

function Test-AudioIntegrity {
    param([string]$FilePath)

    try {
        $Result = & ffprobe -v error -show_format -show_streams "$FilePath" 2>&1
        if ($LASTEXITCODE -ne 0) {
            return @{ Broken = $true; Reason = "无法解析音频格式" }
        }

        # 尝试解码音频
        $TestResult = & ffmpeg -v error -i "$FilePath" -f null - 2>&1
        if ($LASTEXITCODE -ne 0) {
            return @{ Broken = $true; Reason = "音频数据损坏" }
        }

        return @{ Broken = $false }
    }
    catch {
        return @{ Broken = $true; Reason = "测试失败: $_" }
    }
}

function Test-JsonIntegrity {
    param([string]$FilePath)

    try {
        $Content = Get-Content -Path $FilePath -Raw
        $Json = $Content | ConvertFrom-Json

        if ($null -eq $Json) {
            return @{ Broken = $true; Reason = "JSON为空" }
        }

        return @{ Broken = $false }
    }
    catch {
        return @{ Broken = $true; Reason = "JSON解析错误: $_" }
    }
}

function Scan-BrokenAssets {
    Write-Log "扫描损坏的资产..."

    $BrokenAssets = @{
        Textures = @()
        Meshes = @()
        Audio = @()
        Configs = @()
    }

    # 扫描纹理
    Write-Log "检查纹理..."
    $Textures = Get-ChildItem -Path $AssetPath -Include "*.png","*.jpg","*.jpeg","*.tga" -Recurse -File
    $Index = 0

    foreach ($Texture in $Textures) {
        $Index++
        $ProgressPercent = [math]::Round(($Index / $Textures.Count) * 100, 2)
        Write-Progress -Activity "检查纹理" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $TestResult = Test-TextureIntegrity -FilePath $Texture.FullName

        if ($TestResult.Broken) {
            Write-Log "发现损坏纹理: $($Texture.Name) - $($TestResult.Reason)" "WARNING"
            $BrokenAssets.Textures += @{
                File = $Texture.FullName
                Name = $Texture.Name
                Reason = $TestResult.Reason
                Size = $Texture.Length
            }
        }
    }

    Write-Progress -Activity "检查纹理" -Completed

    # 扫描网格
    Write-Log "检查网格..."
    $Meshes = Get-ChildItem -Path $AssetPath -Include "*.fbx","*.obj" -Recurse -File
    $Index = 0

    foreach ($Mesh in $Meshes) {
        $Index++
        $ProgressPercent = [math]::Round(($Index / $Meshes.Count) * 100, 2)
        Write-Progress -Activity "检查网格" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $TestResult = Test-MeshIntegrity -FilePath $Mesh.FullName

        if ($TestResult.Broken) {
            Write-Log "发现损坏网格: $($Mesh.Name) - $($TestResult.Reason)" "WARNING"
            $BrokenAssets.Meshes += @{
                File = $Mesh.FullName
                Name = $Mesh.Name
                Reason = $TestResult.Reason
                Size = $Mesh.Length
            }
        }
    }

    Write-Progress -Activity "检查网格" -Completed

    # 扫描音频
    Write-Log "检查音频..."
    $AudioFiles = Get-ChildItem -Path $AssetPath -Include "*.wav","*.mp3","*.ogg" -Recurse -File
    $Index = 0

    foreach ($Audio in $AudioFiles) {
        $Index++
        $ProgressPercent = [math]::Round(($Index / $AudioFiles.Count) * 100, 2)
        Write-Progress -Activity "检查音频" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $TestResult = Test-AudioIntegrity -FilePath $Audio.FullName

        if ($TestResult.Broken) {
            Write-Log "发现损坏音频: $($Audio.Name) - $($TestResult.Reason)" "WARNING"
            $BrokenAssets.Audio += @{
                File = $Audio.FullName
                Name = $Audio.Name
                Reason = $TestResult.Reason
                Size = $Audio.Length
            }
        }
    }

    Write-Progress -Activity "检查音频" -Completed

    # 扫描配置文件
    Write-Log "检查配置文件..."
    $Configs = Get-ChildItem -Path $AssetPath -Include "*.json" -Recurse -File
    $Index = 0

    foreach ($Config in $Configs) {
        $Index++
        $ProgressPercent = [math]::Round(($Index / $Configs.Count) * 100, 2)
        Write-Progress -Activity "检查配置" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $TestResult = Test-JsonIntegrity -FilePath $Config.FullName

        if ($TestResult.Broken) {
            Write-Log "发现损坏配置: $($Config.Name) - $($TestResult.Reason)" "WARNING"
            $BrokenAssets.Configs += @{
                File = $Config.FullName
                Name = $Config.Name
                Reason = $TestResult.Reason
                Size = $Config.Length
            }
        }
    }

    Write-Progress -Activity "检查配置" -Completed

    Write-Log "扫描完成，发现损坏资产:"
    Write-Log "  纹理: $($BrokenAssets.Textures.Count)"
    Write-Log "  网格: $($BrokenAssets.Meshes.Count)"
    Write-Log "  音频: $($BrokenAssets.Audio.Count)"
    Write-Log "  配置: $($BrokenAssets.Configs.Count)"

    return $BrokenAssets
}

function Quarantine-BrokenAssets {
    param($BrokenAssets)

    Write-Log "隔离损坏的资产..."

    $QuarantinePath = Join-Path $AssetPath "_Quarantine"
    if (-not (Test-Path $QuarantinePath)) {
        New-Item -ItemType Directory -Path $QuarantinePath -Force | Out-Null
    }

    $QuarantineCount = 0

    foreach ($Category in @("Textures", "Meshes", "Audio", "Configs")) {
        foreach ($Asset in $BrokenAssets[$Category]) {
            try {
                $TargetPath = Join-Path $QuarantinePath $Asset.Name
                Move-Item -Path $Asset.File -Destination $TargetPath -Force
                Write-Log "已隔离: $($Asset.Name)"
                $QuarantineCount++
            }
            catch {
                Write-Log "隔离失败: $($Asset.Name) - $_" "ERROR"
            }
        }
    }

    Write-Log "已隔离 $QuarantineCount 个损坏资产到: $QuarantinePath"
}

function Generate-Report {
    param($BrokenAssets)

    Write-Log "生成损坏资产报告..."

    $TotalBroken = $BrokenAssets.Textures.Count + $BrokenAssets.Meshes.Count +
                   $BrokenAssets.Audio.Count + $BrokenAssets.Configs.Count

    $TotalSize = 0
    foreach ($Category in @("Textures", "Meshes", "Audio", "Configs")) {
        $TotalSize += ($BrokenAssets[$Category] | Measure-Object -Property Size -Sum).Sum
    }

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\BrokenAssets_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
损坏资产报告
========================================
扫描时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
资产路径: $AssetPath

统计信息:
- 总损坏资产: $TotalBroken
- 总大小: $([math]::Round($TotalSize / 1MB, 2)) MB

分类统计:
- 纹理: $($BrokenAssets.Textures.Count)
- 网格: $($BrokenAssets.Meshes.Count)
- 音频: $($BrokenAssets.Audio.Count)
- 配置: $($BrokenAssets.Configs.Count)

损坏的纹理:
$($BrokenAssets.Textures | ForEach-Object {
    "  ✗ $($_.Name)"
    "    原因: $($_.Reason)"
    "    大小: $([math]::Round($_.Size / 1KB, 2)) KB"
    "    路径: $($_.File)"
    ""
} | Out-String)

损坏的网格:
$($BrokenAssets.Meshes | ForEach-Object {
    "  ✗ $($_.Name)"
    "    原因: $($_.Reason)"
    "    大小: $([math]::Round($_.Size / 1KB, 2)) KB"
    "    路径: $($_.File)"
    ""
} | Out-String)

损坏的音频:
$($BrokenAssets.Audio | ForEach-Object {
    "  ✗ $($_.Name)"
    "    原因: $($_.Reason)"
    "    大小: $([math]::Round($_.Size / 1KB, 2)) KB"
    "    路径: $($_.File)"
    ""
} | Out-String)

损坏的配置:
$($BrokenAssets.Configs | ForEach-Object {
    "  ✗ $($_.Name)"
    "    原因: $($_.Reason)"
    "    大小: $([math]::Round($_.Size / 1KB, 2)) KB"
    "    路径: $($_.File)"
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
    Write-Log "开始查找损坏资产" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $AssetPath)) {
        throw "资产路径不存在: $AssetPath"
    }

    $BrokenAssets = Scan-BrokenAssets

    if ($Quarantine) {
        Quarantine-BrokenAssets -BrokenAssets $BrokenAssets
    }

    $ReportPath = Generate-Report -BrokenAssets $BrokenAssets

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "损坏资产扫描完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "报告路径: $ReportPath" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "查找损坏资产失败: $_" "ERROR"
    exit 1
}
