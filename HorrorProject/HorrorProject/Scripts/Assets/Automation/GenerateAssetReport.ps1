# GenerateAssetReport.ps1 - 生成资产报告
param(
    [Parameter(Mandatory=$true)]
    [string]$AssetPath,
    [string]$OutputPath = "D:\gptzuo\HorrorProject\HorrorProject\Reports",
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AssetReport.log",
    [switch]$IncludeStatistics,
    [switch]$IncludeReferences,
    [switch]$GenerateHTML
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

function Collect-AssetStatistics {
    Write-Log "收集资产统计信息..."

    $Stats = @{
        Textures = @{
            Count = 0
            TotalSize = 0
            ByFormat = @{}
            BySize = @{}
        }
        Meshes = @{
            Count = 0
            TotalSize = 0
            ByFormat = @{}
        }
        Audio = @{
            Count = 0
            TotalSize = 0
            TotalDuration = 0
            ByFormat = @{}
        }
        Materials = @{
            Count = 0
        }
    }

    # 统计纹理
    Write-Log "统计纹理..."
    $Textures = Get-ChildItem -Path $AssetPath -Include "*.png","*.jpg","*.jpeg","*.tga" -Recurse -File

    foreach ($Texture in $Textures) {
        $Stats.Textures.Count++
        $Stats.Textures.TotalSize += $Texture.Length

        $Format = $Texture.Extension.ToLower()
        if (-not $Stats.Textures.ByFormat.ContainsKey($Format)) {
            $Stats.Textures.ByFormat[$Format] = @{ Count = 0; Size = 0 }
        }
        $Stats.Textures.ByFormat[$Format].Count++
        $Stats.Textures.ByFormat[$Format].Size += $Texture.Length

        # 按大小分类
        $SizeCategory = if ($Texture.Length -lt 100KB) { "Small" }
                       elseif ($Texture.Length -lt 1MB) { "Medium" }
                       elseif ($Texture.Length -lt 5MB) { "Large" }
                       else { "VeryLarge" }

        if (-not $Stats.Textures.BySize.ContainsKey($SizeCategory)) {
            $Stats.Textures.BySize[$SizeCategory] = 0
        }
        $Stats.Textures.BySize[$SizeCategory]++
    }

    # 统计网格
    Write-Log "统计网格..."
    $Meshes = Get-ChildItem -Path $AssetPath -Include "*.fbx","*.obj","*.dae" -Recurse -File

    foreach ($Mesh in $Meshes) {
        $Stats.Meshes.Count++
        $Stats.Meshes.TotalSize += $Mesh.Length

        $Format = $Mesh.Extension.ToLower()
        if (-not $Stats.Meshes.ByFormat.ContainsKey($Format)) {
            $Stats.Meshes.ByFormat[$Format] = @{ Count = 0; Size = 0 }
        }
        $Stats.Meshes.ByFormat[$Format].Count++
        $Stats.Meshes.ByFormat[$Format].Size += $Mesh.Length
    }

    # 统计音频
    Write-Log "统计音频..."
    $AudioFiles = Get-ChildItem -Path $AssetPath -Include "*.wav","*.mp3","*.ogg" -Recurse -File

    foreach ($Audio in $AudioFiles) {
        $Stats.Audio.Count++
        $Stats.Audio.TotalSize += $Audio.Length

        $Format = $Audio.Extension.ToLower()
        if (-not $Stats.Audio.ByFormat.ContainsKey($Format)) {
            $Stats.Audio.ByFormat[$Format] = @{ Count = 0; Size = 0; Duration = 0 }
        }
        $Stats.Audio.ByFormat[$Format].Count++
        $Stats.Audio.ByFormat[$Format].Size += $Audio.Length

        # 获取音频时长
        try {
            $AudioInfo = & ffprobe -v quiet -print_format json -show_format "$($Audio.FullName)" 2>&1 | Out-String
            $Metadata = $AudioInfo | ConvertFrom-Json
            if ($Metadata.format.duration) {
                $Duration = [float]$Metadata.format.duration
                $Stats.Audio.TotalDuration += $Duration
                $Stats.Audio.ByFormat[$Format].Duration += $Duration
            }
        }
        catch {
            # 忽略错误
        }
    }

    # 统计材质
    Write-Log "统计材质..."
    $Materials = Get-ChildItem -Path $AssetPath -Include "*Material*.json" -Recurse -File
    $Stats.Materials.Count = $Materials.Count

    Write-Log "统计完成"

    return $Stats
}

function Generate-TextReport {
    param($Stats)

    Write-Log "生成文本报告..."

    $ReportPath = Join-Path $OutputPath "AssetReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    $Report = @"
========================================
资产统计报告
========================================
生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
资产路径: $AssetPath

总体统计:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

纹理资产:
  总数: $($Stats.Textures.Count)
  总大小: $([math]::Round($Stats.Textures.TotalSize / 1MB, 2)) MB

  格式分布:
$($Stats.Textures.ByFormat.GetEnumerator() | ForEach-Object {
    "    $($_.Key): $($_.Value.Count) 个 ($([math]::Round($_.Value.Size / 1MB, 2)) MB)"
} | Out-String)

  大小分布:
$($Stats.Textures.BySize.GetEnumerator() | ForEach-Object {
    $Category = switch ($_.Key) {
        "Small" { "小型 (<100KB)" }
        "Medium" { "中型 (100KB-1MB)" }
        "Large" { "大型 (1MB-5MB)" }
        "VeryLarge" { "超大 (>5MB)" }
    }
    "    $Category : $($_.Value) 个"
} | Out-String)

网格资产:
  总数: $($Stats.Meshes.Count)
  总大小: $([math]::Round($Stats.Meshes.TotalSize / 1MB, 2)) MB

  格式分布:
$($Stats.Meshes.ByFormat.GetEnumerator() | ForEach-Object {
    "    $($_.Key): $($_.Value.Count) 个 ($([math]::Round($_.Value.Size / 1MB, 2)) MB)"
} | Out-String)

音频资产:
  总数: $($Stats.Audio.Count)
  总大小: $([math]::Round($Stats.Audio.TotalSize / 1MB, 2)) MB
  总时长: $([math]::Round($Stats.Audio.TotalDuration / 60, 2)) 分钟

  格式分布:
$($Stats.Audio.ByFormat.GetEnumerator() | ForEach-Object {
    "    $($_.Key): $($_.Value.Count) 个 ($([math]::Round($_.Value.Size / 1MB, 2)) MB, $([math]::Round($_.Value.Duration / 60, 2)) 分钟)"
} | Out-String)

材质资产:
  总数: $($Stats.Materials.Count)

总计:
  资产总数: $($Stats.Textures.Count + $Stats.Meshes.Count + $Stats.Audio.Count + $Stats.Materials.Count)
  总大小: $([math]::Round(($Stats.Textures.TotalSize + $Stats.Meshes.TotalSize + $Stats.Audio.TotalSize) / 1MB, 2)) MB

执行时间: $((Get-Date) - $StartTime)
========================================
"@

    $Report | Out-File -FilePath $ReportPath -Encoding UTF8
    Write-Log "文本报告已生成: $ReportPath"

    return $ReportPath
}

function Generate-HTMLReport {
    param($Stats)

    Write-Log "生成HTML报告..."

    $ReportPath = Join-Path $OutputPath "AssetReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"

    $HTML = @"
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>资产统计报告</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            padding: 40px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 10px;
        }
        .timestamp {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
        }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .stat-card {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .stat-card h3 {
            margin: 0 0 10px 0;
            font-size: 16px;
            opacity: 0.9;
        }
        .stat-card .value {
            font-size: 32px;
            font-weight: bold;
            margin-bottom: 5px;
        }
        .stat-card .label {
            font-size: 14px;
            opacity: 0.8;
        }
        .section {
            margin-bottom: 30px;
        }
        .section h2 {
            color: #667eea;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 10px;
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #667eea;
            color: white;
            font-weight: 600;
        }
        tr:hover {
            background-color: #f5f5f5;
        }
        .chart {
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎮 资产统计报告</h1>
        <div class="timestamp">生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')</div>

        <div class="stats-grid">
            <div class="stat-card">
                <h3>纹理资产</h3>
                <div class="value">$($Stats.Textures.Count)</div>
                <div class="label">$([math]::Round($Stats.Textures.TotalSize / 1MB, 2)) MB</div>
            </div>
            <div class="stat-card">
                <h3>网格资产</h3>
                <div class="value">$($Stats.Meshes.Count)</div>
                <div class="label">$([math]::Round($Stats.Meshes.TotalSize / 1MB, 2)) MB</div>
            </div>
            <div class="stat-card">
                <h3>音频资产</h3>
                <div class="value">$($Stats.Audio.Count)</div>
                <div class="label">$([math]::Round($Stats.Audio.TotalDuration / 60, 2)) 分钟</div>
            </div>
            <div class="stat-card">
                <h3>材质资产</h3>
                <div class="value">$($Stats.Materials.Count)</div>
                <div class="label">配置文件</div>
            </div>
        </div>

        <div class="section">
            <h2>📊 纹理格式分布</h2>
            <table>
                <thead>
                    <tr>
                        <th>格式</th>
                        <th>数量</th>
                        <th>大小</th>
                    </tr>
                </thead>
                <tbody>
$($Stats.Textures.ByFormat.GetEnumerator() | ForEach-Object {
    "                    <tr>
                        <td>$($_.Key)</td>
                        <td>$($_.Value.Count)</td>
                        <td>$([math]::Round($_.Value.Size / 1MB, 2)) MB</td>
                    </tr>"
} | Out-String)
                </tbody>
            </table>
        </div>

        <div class="section">
            <h2>🎵 音频格式分布</h2>
            <table>
                <thead>
                    <tr>
                        <th>格式</th>
                        <th>数量</th>
                        <th>大小</th>
                        <th>时长</th>
                    </tr>
                </thead>
                <tbody>
$($Stats.Audio.ByFormat.GetEnumerator() | ForEach-Object {
    "                    <tr>
                        <td>$($_.Key)</td>
                        <td>$($_.Value.Count)</td>
                        <td>$([math]::Round($_.Value.Size / 1MB, 2)) MB</td>
                        <td>$([math]::Round($_.Value.Duration / 60, 2)) 分钟</td>
                    </tr>"
} | Out-String)
                </tbody>
            </table>
        </div>

        <div class="section">
            <h2>📦 网格格式分布</h2>
            <table>
                <thead>
                    <tr>
                        <th>格式</th>
                        <th>数量</th>
                        <th>大小</th>
                    </tr>
                </thead>
                <tbody>
$($Stats.Meshes.ByFormat.GetEnumerator() | ForEach-Object {
    "                    <tr>
                        <td>$($_.Key)</td>
                        <td>$($_.Value.Count)</td>
                        <td>$([math]::Round($_.Value.Size / 1MB, 2)) MB</td>
                    </tr>"
} | Out-String)
                </tbody>
            </table>
        </div>
    </div>
</body>
</html>
"@

    $HTML | Out-File -FilePath $ReportPath -Encoding UTF8
    Write-Log "HTML报告已生成: $ReportPath"

    return $ReportPath
}

# 主执行流程
try {
    Write-Log "========================================" "INFO"
    Write-Log "开始生成资产报告" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $AssetPath)) {
        throw "资产路径不存在: $AssetPath"
    }

    $Stats = Collect-AssetStatistics

    $TextReportPath = Generate-TextReport -Stats $Stats

    if ($GenerateHTML) {
        $HTMLReportPath = Generate-HTMLReport -Stats $Stats
    }

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "资产报告生成完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "文本报告: $TextReportPath" "INFO"
    if ($GenerateHTML) {
        Write-Log "HTML报告: $HTMLReportPath" "INFO"
    }
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "生成资产报告失败: $_" "ERROR"
    exit 1
}
