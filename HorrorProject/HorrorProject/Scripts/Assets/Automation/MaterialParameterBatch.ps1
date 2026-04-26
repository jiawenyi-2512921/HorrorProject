# MaterialParameterBatch.ps1 - 批量设置材质参数
param(
    [Parameter(Mandatory=$true)]
    [string]$MaterialPath,
    [Parameter(Mandatory=$true)]
    [string]$ParameterFile,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MaterialParameter.log",
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

function Load-ParameterConfig {
    param([string]$ConfigPath)

    try {
        $Config = Get-Content -Path $ConfigPath -Raw | ConvertFrom-Json
        Write-Log "成功加载参数配置: $ConfigPath"
        return $Config
    }
    catch {
        Write-Log "加载参数配置失败: $_" "ERROR"
        throw
    }
}

function Get-MaterialFiles {
    Write-Log "扫描材质文件..."

    $MaterialFiles = Get-ChildItem -Path $MaterialPath -Include "*.json" -Recurse -File

    Write-Log "找到 $($MaterialFiles.Count) 个材质配置文件"
    return $MaterialFiles
}

function Update-MaterialParameters {
    param(
        [string]$MaterialFile,
        [object]$Parameters,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "更新材质参数" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "更新材质 [$Index/$Total]: $(Split-Path $MaterialFile -Leaf)"

    try {
        # 备份原文件
        if ($CreateBackup) {
            $BackupFile = "$MaterialFile.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
            Copy-Item -Path $MaterialFile -Destination $BackupFile -Force
            Write-Log "  已创建备份: $BackupFile"
        }

        # 读取材质配置
        $MaterialConfig = Get-Content -Path $MaterialFile -Raw | ConvertFrom-Json

        $UpdatedParams = 0

        # 更新标量参数
        if ($Parameters.ScalarParameters) {
            foreach ($Param in $Parameters.ScalarParameters) {
                $Existing = $MaterialConfig.Parameters.ScalarParameters | Where-Object { $_.Name -eq $Param.Name }

                if ($Existing) {
                    $OldValue = $Existing.Value
                    $Existing.Value = $Param.Value
                    Write-Log "  更新标量参数: $($Param.Name) = $($Param.Value) (旧值: $OldValue)"
                    $UpdatedParams++
                }
                else {
                    $MaterialConfig.Parameters.ScalarParameters += @{
                        Name = $Param.Name
                        Value = $Param.Value
                    }
                    Write-Log "  添加标量参数: $($Param.Name) = $($Param.Value)"
                    $UpdatedParams++
                }
            }
        }

        # 更新向量参数
        if ($Parameters.VectorParameters) {
            foreach ($Param in $Parameters.VectorParameters) {
                $Existing = $MaterialConfig.Parameters.VectorParameters | Where-Object { $_.Name -eq $Param.Name }

                if ($Existing) {
                    $OldValue = $Existing.Value -join ", "
                    $Existing.Value = $Param.Value
                    Write-Log "  更新向量参数: $($Param.Name) = ($($Param.Value -join ', ')) (旧值: ($OldValue))"
                    $UpdatedParams++
                }
                else {
                    $MaterialConfig.Parameters.VectorParameters += @{
                        Name = $Param.Name
                        Value = $Param.Value
                    }
                    Write-Log "  添加向量参数: $($Param.Name) = ($($Param.Value -join ', '))"
                    $UpdatedParams++
                }
            }
        }

        # 更新纹理参数
        if ($Parameters.TextureParameters) {
            foreach ($Param in $Parameters.TextureParameters) {
                $Existing = $MaterialConfig.Parameters.TextureParameters | Where-Object { $_.Name -eq $Param.Name }

                if ($Existing) {
                    $OldValue = $Existing.Value
                    $Existing.Value = $Param.Value
                    Write-Log "  更新纹理参数: $($Param.Name) = $($Param.Value) (旧值: $OldValue)"
                    $UpdatedParams++
                }
                else {
                    $MaterialConfig.Parameters.TextureParameters += @{
                        Name = $Param.Name
                        Value = $Param.Value
                    }
                    Write-Log "  添加纹理参数: $($Param.Name) = $($Param.Value)"
                    $UpdatedParams++
                }
            }
        }

        # 保存更新后的配置
        $MaterialConfig | ConvertTo-Json -Depth 10 | Out-File -FilePath $MaterialFile -Encoding UTF8

        Write-Log "成功更新 $UpdatedParams 个参数" "SUCCESS"

        return @{
            Success = $true
            File = $MaterialFile
            UpdatedParams = $UpdatedParams
        }
    }
    catch {
        Write-Log "更新材质参数失败: $_" "ERROR"
        return @{
            Success = $false
            File = $MaterialFile
            Error = $_.Exception.Message
        }
    }
}

function Process-Materials {
    Write-Log "加载参数配置..."
    $Parameters = Load-ParameterConfig -ConfigPath $ParameterFile

    $MaterialFiles = Get-MaterialFiles

    if ($MaterialFiles.Count -eq 0) {
        Write-Log "未找到材质文件" "WARNING"
        return @()
    }

    $Results = @()
    $Index = 0

    foreach ($MaterialFile in $MaterialFiles) {
        $Index++
        $Result = Update-MaterialParameters `
            -MaterialFile $MaterialFile.FullName `
            -Parameters $Parameters `
            -Index $Index `
            -Total $MaterialFiles.Count

        $Results += $Result
    }

    Write-Progress -Activity "更新材质参数" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成参数更新报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count
    $TotalUpdates = ($Results | Where-Object { $_.Success } | Measure-Object -Property UpdatedParams -Sum).Sum

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MaterialParameter_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
材质参数批量更新报告
========================================
更新时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
材质路径: $MaterialPath
参数文件: $ParameterFile

统计信息:
- 总材质数: $($Results.Count)
- 成功更新: $SuccessCount
- 更新失败: $FailCount
- 总参数更新数: $TotalUpdates

更新详情:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf)"
    "    更新参数数: $($_.UpdatedParams)"
    ""
} | Out-String)

失败的材质:
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
    Write-Log "开始批量更新材质参数" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $MaterialPath)) {
        throw "材质路径不存在: $MaterialPath"
    }

    if (-not (Test-Path $ParameterFile)) {
        throw "参数文件不存在: $ParameterFile"
    }

    $Results = Process-Materials

    if ($Results.Count -gt 0) {
        Generate-Report -Results $Results
    }

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "材质参数更新完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "批量更新材质参数失败: $_" "ERROR"
    exit 1
}
