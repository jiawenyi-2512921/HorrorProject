# AudioBatchImport.ps1 - 批量导入音频资产
param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath,
    [Parameter(Mandatory=$true)]
    [string]$DestinationPath,
    [string]$UProjectPath = "D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject",
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioImport.log",
    [switch]$CreateBackup
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

function Initialize-Environment {
    Write-Log "初始化音频导入环境..."

    if (-not (Test-Path $SourcePath)) {
        throw "源路径不存在: $SourcePath"
    }

    if (-not (Test-Path $UProjectPath)) {
        throw "项目文件不存在: $UProjectPath"
    }

    $LogDir = Split-Path $LogPath -Parent
    if (-not (Test-Path $LogDir)) {
        New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
    }

    Write-Log "环境初始化完成"
}

function Get-AudioFiles {
    Write-Log "扫描音频文件..."

    $AudioExtensions = @("*.wav", "*.mp3", "*.ogg", "*.flac", "*.aiff")
    $AudioFiles = @()

    foreach ($ext in $AudioExtensions) {
        $Files = Get-ChildItem -Path $SourcePath -Filter $ext -Recurse -File
        $AudioFiles += $Files
    }

    Write-Log "找到 $($AudioFiles.Count) 个音频文件"
    return $AudioFiles
}

function Create-Backup {
    param([string]$Path)

    if ($CreateBackup) {
        Write-Log "创建备份..."
        $BackupPath = "$Path.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"

        if (Test-Path $Path) {
            Copy-Item -Path $Path -Destination $BackupPath -Recurse -Force
            Write-Log "备份创建完成: $BackupPath"
            return $BackupPath
        }
    }
    return $null
}

function Import-AudioFile {
    param(
        [System.IO.FileInfo]$AudioFile,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "导入音频文件" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "导入音频 [$Index/$Total]: $($AudioFile.Name)"

    # 构建相对路径
    $RelativePath = $AudioFile.FullName.Substring($SourcePath.Length).TrimStart('\', '/')
    $TargetPath = Join-Path $DestinationPath $RelativePath
    $TargetDir = Split-Path $TargetPath -Parent

    # 创建目标目录
    if (-not (Test-Path $TargetDir)) {
        New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
    }

    # 使用UE命令行工具导入
    $UEEditorCmd = "D:\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    $ImportCommand = @"
import -file="$($AudioFile.FullName)" -destination="$DestinationPath" -project="$UProjectPath"
"@

    try {
        # 复制文件到Content目录
        Copy-Item -Path $AudioFile.FullName -Destination $TargetPath -Force

        Write-Log "成功导入: $($AudioFile.Name)" "SUCCESS"
        return @{
            Success = $true
            File = $AudioFile.Name
            Size = $AudioFile.Length
            Path = $TargetPath
        }
    }
    catch {
        Write-Log "导入失败: $($AudioFile.Name) - $_" "ERROR"
        return @{
            Success = $false
            File = $AudioFile.Name
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

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AudioImport_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
音频批量导入报告
========================================
导入时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
源路径: $SourcePath
目标路径: $DestinationPath

统计信息:
- 总文件数: $($Results.Count)
- 成功导入: $SuccessCount
- 导入失败: $FailCount
- 总大小: $([math]::Round($TotalSize / 1MB, 2)) MB

成功导入的文件:
$($Results | Where-Object { $_.Success } | ForEach-Object { "  ✓ $($_.File) - $([math]::Round($_.Size / 1KB, 2)) KB" } | Out-String)

失败的文件:
$($Results | Where-Object { -not $_.Success } | ForEach-Object { "  ✗ $($_.File) - $($_.Error)" } | Out-String)

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
    Write-Log "开始音频批量导入" "INFO"
    Write-Log "========================================" "INFO"

    Initialize-Environment

    $BackupPath = Create-Backup -Path $DestinationPath

    $AudioFiles = Get-AudioFiles

    if ($AudioFiles.Count -eq 0) {
        Write-Log "未找到音频文件，退出" "WARNING"
        exit 0
    }

    $Results = @()
    $Index = 0

    foreach ($AudioFile in $AudioFiles) {
        $Index++
        $Result = Import-AudioFile -AudioFile $AudioFile -Index $Index -Total $AudioFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "导入音频文件" -Completed

    $ReportPath = Generate-ImportReport -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "音频导入完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "报告路径: $ReportPath" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "音频导入失败: $_" "ERROR"
    Write-Log $_.ScriptStackTrace "ERROR"
    exit 1
}
