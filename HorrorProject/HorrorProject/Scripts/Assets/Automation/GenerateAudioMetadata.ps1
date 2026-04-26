# GenerateAudioMetadata.ps1 - 生成音频元数据
param(
    [Parameter(Mandatory=$true)]
    [string]$AudioPath,
    [string]$OutputPath = "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Metadata",
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioMetadata.log",
    [switch]$IncludeWaveform
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

function Get-AudioMetadata {
    param([string]$FilePath)

    Write-Log "提取元数据: $(Split-Path $FilePath -Leaf)"

    try {
        $FFprobeOutput = & ffprobe -v quiet -print_format json -show_format -show_streams "$FilePath" 2>&1 | Out-String
        $Metadata = $FFprobeOutput | ConvertFrom-Json

        $AudioStream = $Metadata.streams | Where-Object { $_.codec_type -eq "audio" } | Select-Object -First 1

        if ($null -eq $AudioStream) {
            throw "未找到音频流"
        }

        $FileInfo = Get-Item $FilePath

        return @{
            FileName = $FileInfo.Name
            FilePath = $FilePath
            FileSize = $FileInfo.Length
            Duration = [float]$AudioStream.duration
            SampleRate = [int]$AudioStream.sample_rate
            Channels = [int]$AudioStream.channels
            BitRate = [int]$Metadata.format.bit_rate
            Codec = $AudioStream.codec_name
            Format = $Metadata.format.format_name
            CreatedDate = $FileInfo.CreationTime
            ModifiedDate = $FileInfo.LastWriteTime
        }
    }
    catch {
        Write-Log "提取元数据失败: $_" "ERROR"
        return $null
    }
}

function Generate-Waveform {
    param([string]$FilePath, [string]$OutputImage)

    Write-Log "生成波形图: $(Split-Path $FilePath -Leaf)"

    try {
        $FFmpegArgs = @(
            "-i", "`"$FilePath`"",
            "-filter_complex", "showwavespic=s=1920x400:colors=0x00FF00",
            "-frames:v", "1",
            "-y",
            "`"$OutputImage`""
        )

        $Process = Start-Process -FilePath "ffmpeg" -ArgumentList $FFmpegArgs -NoNewWindow -Wait -PassThru -RedirectStandardError "NUL"

        if ($Process.ExitCode -eq 0 -and (Test-Path $OutputImage)) {
            Write-Log "波形图生成成功" "SUCCESS"
            return $OutputImage
        }
        else {
            throw "FFmpeg生成波形图失败"
        }
    }
    catch {
        Write-Log "生成波形图失败: $_" "ERROR"
        return $null
    }
}

function Export-MetadataToJson {
    param([object]$Metadata, [string]$OutputFile)

    $JsonData = @{
        AudioMetadata = $Metadata
        GeneratedDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        Version = "1.0"
    }

    $JsonData | ConvertTo-Json -Depth 10 | Out-File -FilePath $OutputFile -Encoding UTF8
    Write-Log "元数据已导出: $OutputFile"
}

function Process-AudioFiles {
    Write-Log "扫描音频文件..."

    $AudioFiles = Get-ChildItem -Path $AudioPath -Include "*.wav","*.mp3","*.ogg","*.flac" -Recurse -File

    Write-Log "找到 $($AudioFiles.Count) 个音频文件"

    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    $AllMetadata = @()
    $Index = 0

    foreach ($AudioFile in $AudioFiles) {
        $Index++
        $ProgressPercent = [math]::Round(($Index / $AudioFiles.Count) * 100, 2)
        Write-Progress -Activity "生成音频元数据" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

        $Metadata = Get-AudioMetadata -FilePath $AudioFile.FullName

        if ($null -ne $Metadata) {
            # 生成波形图
            if ($IncludeWaveform) {
                $WaveformPath = Join-Path $OutputPath "$($AudioFile.BaseName)_waveform.png"
                $WaveformFile = Generate-Waveform -FilePath $AudioFile.FullName -OutputImage $WaveformPath
                $Metadata.WaveformImage = $WaveformFile
            }

            # 导出单个文件的元数据
            $MetadataFile = Join-Path $OutputPath "$($AudioFile.BaseName)_metadata.json"
            Export-MetadataToJson -Metadata $Metadata -OutputFile $MetadataFile

            $AllMetadata += $Metadata
        }
    }

    Write-Progress -Activity "生成音频元数据" -Completed

    # 导出汇总元数据
    $SummaryFile = Join-Path $OutputPath "AudioMetadata_Summary.json"
    Export-MetadataToJson -Metadata $AllMetadata -OutputFile $SummaryFile

    return $AllMetadata
}

function Generate-Report {
    param($Metadata)

    Write-Log "生成元数据报告..."

    $TotalSize = ($Metadata | Measure-Object -Property FileSize -Sum).Sum
    $TotalDuration = ($Metadata | Measure-Object -Property Duration -Sum).Sum
    $AvgBitRate = ($Metadata | Measure-Object -Property BitRate -Average).Average

    $CodecStats = $Metadata | Group-Object -Property Codec | Select-Object Name, Count
    $SampleRateStats = $Metadata | Group-Object -Property SampleRate | Select-Object Name, Count

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioMetadata_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
音频元数据报告
========================================
生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
音频路径: $AudioPath
输出路径: $OutputPath

统计信息:
- 总文件数: $($Metadata.Count)
- 总大小: $([math]::Round($TotalSize / 1MB, 2)) MB
- 总时长: $([math]::Round($TotalDuration / 60, 2)) 分钟
- 平均比特率: $([math]::Round($AvgBitRate / 1000, 2)) kbps

编码格式分布:
$($CodecStats | ForEach-Object { "  - $($_.Name): $($_.Count) 个文件" } | Out-String)

采样率分布:
$($SampleRateStats | ForEach-Object { "  - $($_.Name) Hz: $($_.Count) 个文件" } | Out-String)

音频文件列表:
$($Metadata | ForEach-Object {
    "  • $($_.FileName)"
    "    大小: $([math]::Round($_.FileSize / 1KB, 2)) KB"
    "    时长: $([math]::Round($_.Duration, 2)) 秒"
    "    采样率: $($_.SampleRate) Hz"
    "    声道: $($_.Channels)"
    "    编码: $($_.Codec)"
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
    Write-Log "开始生成音频元数据" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $AudioPath)) {
        throw "音频路径不存在: $AudioPath"
    }

    $Metadata = Process-AudioFiles

    if ($Metadata.Count -gt 0) {
        Generate-Report -Metadata $Metadata
    }

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "音频元数据生成完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "生成音频元数据失败: $_" "ERROR"
    exit 1
}
