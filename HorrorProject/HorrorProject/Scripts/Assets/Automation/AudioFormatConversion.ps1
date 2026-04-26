# AudioFormatConversion.ps1 - 音频格式转换
param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath,
    [Parameter(Mandatory=$true)]
    [ValidateSet("wav", "mp3", "ogg", "flac")]
    [string]$TargetFormat,
    [string]$OutputPath,
    [int]$SampleRate = 48000,
    [int]$BitRate = 320,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioConversion.log",
    [switch]$DeleteOriginal
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

function Get-FFmpegArgs {
    param([string]$Format)

    switch ($Format) {
        "wav" {
            return @("-acodec", "pcm_s16le", "-ar", "$SampleRate")
        }
        "mp3" {
            return @("-acodec", "libmp3lame", "-b:a", "${BitRate}k", "-ar", "$SampleRate")
        }
        "ogg" {
            return @("-acodec", "libvorbis", "-q:a", "8", "-ar", "$SampleRate")
        }
        "flac" {
            return @("-acodec", "flac", "-compression_level", "8", "-ar", "$SampleRate")
        }
    }
}

function Convert-AudioFile {
    param(
        [string]$InputFile,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "转换音频格式" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    $FileName = [System.IO.Path]::GetFileNameWithoutExtension($InputFile)
    $RelativePath = $InputFile.Substring($SourcePath.Length).TrimStart('\', '/')
    $RelativeDir = Split-Path $RelativePath -Parent

    $OutputDir = if ($OutputPath) {
        Join-Path $OutputPath $RelativeDir
    } else {
        Split-Path $InputFile -Parent
    }

    if (-not (Test-Path $OutputDir)) {
        New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    }

    $OutputFile = Join-Path $OutputDir "$FileName.$TargetFormat"

    Write-Log "转换 [$Index/$Total]: $(Split-Path $InputFile -Leaf) -> $TargetFormat"

    try {
        $FFmpegArgs = @("-i", "`"$InputFile`"") + (Get-FFmpegArgs -Format $TargetFormat) + @("-y", "`"$OutputFile`"")

        $Process = Start-Process -FilePath "ffmpeg" -ArgumentList $FFmpegArgs -NoNewWindow -Wait -PassThru -RedirectStandardError "NUL"

        if ($Process.ExitCode -eq 0 -and (Test-Path $OutputFile)) {
            $InputSize = (Get-Item $InputFile).Length
            $OutputSize = (Get-Item $OutputFile).Length

            Write-Log "转换成功: $([math]::Round($InputSize/1KB, 2)) KB -> $([math]::Round($OutputSize/1KB, 2)) KB" "SUCCESS"

            if ($DeleteOriginal) {
                Remove-Item -Path $InputFile -Force
                Write-Log "已删除原文件" "INFO"
            }

            return @{
                Success = $true
                InputFile = $InputFile
                OutputFile = $OutputFile
                InputSize = $InputSize
                OutputSize = $OutputSize
            }
        }
        else {
            throw "FFmpeg转换失败"
        }
    }
    catch {
        Write-Log "转换失败: $_" "ERROR"
        return @{
            Success = $false
            InputFile = $InputFile
            Error = $_.Exception.Message
        }
    }
}

function Process-AudioConversion {
    Write-Log "扫描音频文件..."

    $AudioFiles = Get-ChildItem -Path $SourcePath -Include "*.wav","*.mp3","*.ogg","*.flac" -Recurse -File

    # 过滤掉已经是目标格式的文件
    $AudioFiles = $AudioFiles | Where-Object { $_.Extension.TrimStart('.') -ne $TargetFormat }

    Write-Log "找到 $($AudioFiles.Count) 个需要转换的文件"

    if ($AudioFiles.Count -eq 0) {
        Write-Log "没有需要转换的文件" "WARNING"
        return @()
    }

    $Results = @()
    $Index = 0

    foreach ($AudioFile in $AudioFiles) {
        $Index++
        $Result = Convert-AudioFile -InputFile $AudioFile.FullName -Index $Index -Total $AudioFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "转换音频格式" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成转换报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count
    $TotalInputSize = ($Results | Where-Object { $_.Success } | Measure-Object -Property InputSize -Sum).Sum
    $TotalOutputSize = ($Results | Where-Object { $_.Success } | Measure-Object -Property OutputSize -Sum).Sum

    $CompressionRatio = if ($TotalInputSize -gt 0) {
        [math]::Round(($TotalOutputSize / $TotalInputSize) * 100, 2)
    } else { 0 }

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioConversion_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
音频格式转换报告
========================================
转换时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
源路径: $SourcePath
目标格式: $TargetFormat
采样率: $SampleRate Hz
比特率: $BitRate kbps

统计信息:
- 总文件数: $($Results.Count)
- 成功转换: $SuccessCount
- 转换失败: $FailCount
- 原始大小: $([math]::Round($TotalInputSize / 1MB, 2)) MB
- 转换后大小: $([math]::Round($TotalOutputSize / 1MB, 2)) MB
- 压缩比: $CompressionRatio%

成功转换的文件:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    $InputName = Split-Path $_.InputFile -Leaf
    $OutputName = Split-Path $_.OutputFile -Leaf
    "  ✓ $InputName -> $OutputName ($([math]::Round($_.InputSize/1KB, 2)) KB -> $([math]::Round($_.OutputSize/1KB, 2)) KB)"
} | Out-String)

失败的文件:
$($Results | Where-Object { -not $_.Success } | ForEach-Object {
    "  ✗ $(Split-Path $_.InputFile -Leaf): $($_.Error)"
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
    Write-Log "开始音频格式转换" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $SourcePath)) {
        throw "源路径不存在: $SourcePath"
    }

    $Results = Process-AudioConversion

    if ($Results.Count -gt 0) {
        Generate-Report -Results $Results
    }

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "音频格式转换完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "音频格式转换失败: $_" "ERROR"
    exit 1
}
