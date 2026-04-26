# AudioNormalization.ps1 - 音频标准化处理
param(
    [Parameter(Mandatory=$true)]
    [string]$AudioPath,
    [float]$TargetLUFS = -16.0,
    [float]$PeakLevel = -1.0,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioNormalization.log",
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

function Test-FFmpeg {
    Write-Log "检查FFmpeg..."

    try {
        $FFmpegVersion = & ffmpeg -version 2>&1 | Select-Object -First 1
        Write-Log "FFmpeg已安装: $FFmpegVersion"
        return $true
    }
    catch {
        Write-Log "FFmpeg未安装或不在PATH中" "ERROR"
        return $false
    }
}

function Get-AudioLoudness {
    param([string]$FilePath)

    Write-Log "分析音频响度: $FilePath"

    $FFmpegOutput = & ffmpeg -i "$FilePath" -af loudnorm=print_format=json -f null - 2>&1 | Out-String

    if ($FFmpegOutput -match '"input_i"\s*:\s*"([^"]+)"') {
        $InputLUFS = [float]$matches[1]
        Write-Log "当前LUFS: $InputLUFS"
        return $InputLUFS
    }

    return $null
}

function Normalize-AudioFile {
    param(
        [string]$FilePath,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "标准化音频" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "处理音频 [$Index/$Total]: $(Split-Path $FilePath -Leaf)"

    $CurrentLUFS = Get-AudioLoudness -FilePath $FilePath

    if ($null -eq $CurrentLUFS) {
        Write-Log "无法分析音频响度" "ERROR"
        return @{ Success = $false; File = $FilePath; Error = "无法分析响度" }
    }

    $Difference = [math]::Abs($CurrentLUFS - $TargetLUFS)

    if ($Difference -lt 0.5) {
        Write-Log "音频已符合标准，跳过" "INFO"
        return @{ Success = $true; File = $FilePath; Skipped = $true; CurrentLUFS = $CurrentLUFS }
    }

    $TempFile = "$FilePath.temp.wav"
    $BackupFile = "$FilePath.backup"

    try {
        # 备份原文件
        Copy-Item -Path $FilePath -Destination $BackupFile -Force

        # 标准化处理
        $FFmpegArgs = @(
            "-i", "`"$FilePath`"",
            "-af", "loudnorm=I=$($TargetLUFS):TP=$($PeakLevel):LRA=11",
            "-ar", "48000",
            "-y",
            "`"$TempFile`""
        )

        $Process = Start-Process -FilePath "ffmpeg" -ArgumentList $FFmpegArgs -NoNewWindow -Wait -PassThru

        if ($Process.ExitCode -eq 0 -and (Test-Path $TempFile)) {
            Move-Item -Path $TempFile -Destination $FilePath -Force
            Remove-Item -Path $BackupFile -Force

            $NewLUFS = Get-AudioLoudness -FilePath $FilePath
            Write-Log "标准化完成: $CurrentLUFS LUFS -> $NewLUFS LUFS" "SUCCESS"

            return @{
                Success = $true
                File = $FilePath
                OldLUFS = $CurrentLUFS
                NewLUFS = $NewLUFS
            }
        }
        else {
            # 恢复备份
            Move-Item -Path $BackupFile -Destination $FilePath -Force
            throw "FFmpeg处理失败"
        }
    }
    catch {
        Write-Log "标准化失败: $_" "ERROR"

        if (Test-Path $BackupFile) {
            Move-Item -Path $BackupFile -Destination $FilePath -Force
        }
        if (Test-Path $TempFile) {
            Remove-Item -Path $TempFile -Force
        }

        return @{ Success = $false; File = $FilePath; Error = $_.Exception.Message }
    }
}

function Process-AudioFiles {
    Write-Log "扫描音频文件..."

    $SearchOption = if ($ProcessSubfolders) { "-Recurse" } else { "" }
    $AudioFiles = Get-ChildItem -Path $AudioPath -Include "*.wav","*.mp3","*.ogg" -File @SearchOption

    Write-Log "找到 $($AudioFiles.Count) 个音频文件"

    $Results = @()
    $Index = 0

    foreach ($AudioFile in $AudioFiles) {
        $Index++
        $Result = Normalize-AudioFile -FilePath $AudioFile.FullName -Index $Index -Total $AudioFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "标准化音频" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成标准化报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success -and -not $_.Skipped }).Count
    $SkippedCount = ($Results | Where-Object { $_.Skipped }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioNormalization_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
音频标准化报告
========================================
处理时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
目标LUFS: $TargetLUFS
峰值电平: $PeakLevel dB

统计信息:
- 总文件数: $($Results.Count)
- 成功处理: $SuccessCount
- 跳过处理: $SkippedCount
- 处理失败: $FailCount

处理详情:
$($Results | Where-Object { $_.Success -and -not $_.Skipped } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf): $([math]::Round($_.OldLUFS, 2)) -> $([math]::Round($_.NewLUFS, 2)) LUFS"
} | Out-String)

跳过的文件:
$($Results | Where-Object { $_.Skipped } | ForEach-Object {
    "  - $(Split-Path $_.File -Leaf): $([math]::Round($_.CurrentLUFS, 2)) LUFS (已符合标准)"
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
    Write-Log "开始音频标准化处理" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-FFmpeg)) {
        throw "FFmpeg未安装，请先安装FFmpeg"
    }

    if (-not (Test-Path $AudioPath)) {
        throw "音频路径不存在: $AudioPath"
    }

    $Results = Process-AudioFiles

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "音频标准化完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "音频标准化失败: $_" "ERROR"
    exit 1
}
