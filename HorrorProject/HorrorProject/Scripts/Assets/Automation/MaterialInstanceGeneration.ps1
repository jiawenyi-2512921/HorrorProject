# MaterialInstanceGeneration.ps1 - 材质实例生成
param(
    [Parameter(Mandatory=$true)]
    [string]$ParentMaterialPath,
    [Parameter(Mandatory=$true)]
    [string]$OutputPath,
    [string]$ConfigFile,
    [int]$InstanceCount = 1,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MaterialInstance.log"
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

function Load-InstanceConfig {
    param([string]$ConfigPath)

    if ([string]::IsNullOrEmpty($ConfigPath) -or -not (Test-Path $ConfigPath)) {
        Write-Log "未提供配置文件，使用默认配置" "WARNING"
        return @()
    }

    try {
        $Config = Get-Content -Path $ConfigPath -Raw | ConvertFrom-Json
        Write-Log "成功加载配置文件: $ConfigPath"
        return $Config.Instances
    }
    catch {
        Write-Log "加载配置文件失败: $_" "ERROR"
        return @()
    }
}

function Generate-MaterialInstance {
    param(
        [string]$ParentMaterial,
        [string]$InstanceName,
        [hashtable]$Parameters,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "生成材质实例" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "生成材质实例 [$Index/$Total]: $InstanceName"

    $InstancePath = Join-Path $OutputPath "$InstanceName.uasset"

    try {
        # 创建材质实例配置
        $MaterialInstance = @{
            Type = "MaterialInstanceConstant"
            Parent = $ParentMaterial
            Name = $InstanceName
            Parameters = @{
                ScalarParameters = @()
                VectorParameters = @()
                TextureParameters = @()
            }
        }

        # 添加参数
        if ($null -ne $Parameters) {
            foreach ($Key in $Parameters.Keys) {
                $Value = $Parameters[$Key]

                if ($Value -is [float] -or $Value -is [int]) {
                    $MaterialInstance.Parameters.ScalarParameters += @{
                        Name = $Key
                        Value = [float]$Value
                    }
                }
                elseif ($Value -is [array] -and $Value.Count -in @(3, 4)) {
                    $MaterialInstance.Parameters.VectorParameters += @{
                        Name = $Key
                        Value = $Value
                    }
                }
                elseif ($Value -is [string] -and $Value.StartsWith("/Game/")) {
                    $MaterialInstance.Parameters.TextureParameters += @{
                        Name = $Key
                        Value = $Value
                    }
                }
            }
        }

        # 导出为JSON配置
        $ConfigPath = Join-Path $OutputPath "$InstanceName.json"
        $MaterialInstance | ConvertTo-Json -Depth 10 | Out-File -FilePath $ConfigPath -Encoding UTF8

        Write-Log "材质实例配置已生成: $ConfigPath" "SUCCESS"

        return @{
            Success = $true
            Name = $InstanceName
            ConfigPath = $ConfigPath
            ParameterCount = $MaterialInstance.Parameters.ScalarParameters.Count +
                           $MaterialInstance.Parameters.VectorParameters.Count +
                           $MaterialInstance.Parameters.TextureParameters.Count
        }
    }
    catch {
        Write-Log "生成材质实例失败: $_" "ERROR"
        return @{
            Success = $false
            Name = $InstanceName
            Error = $_.Exception.Message
        }
    }
}

function Process-MaterialInstances {
    Write-Log "开始生成材质实例..."

    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    $Instances = Load-InstanceConfig -ConfigPath $ConfigFile

    if ($Instances.Count -eq 0) {
        Write-Log "从配置生成默认实例..."
        $Instances = @()
        for ($i = 1; $i -le $InstanceCount; $i++) {
            $Instances += @{
                Name = "MI_Material_$i"
                Parameters = @{}
            }
        }
    }

    Write-Log "将生成 $($Instances.Count) 个材质实例"

    $Results = @()
    $Index = 0

    foreach ($Instance in $Instances) {
        $Index++
        $Result = Generate-MaterialInstance `
            -ParentMaterial $ParentMaterialPath `
            -InstanceName $Instance.Name `
            -Parameters $Instance.Parameters `
            -Index $Index `
            -Total $Instances.Count

        $Results += $Result
    }

    Write-Progress -Activity "生成材质实例" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成材质实例报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count
    $TotalParams = ($Results | Where-Object { $_.Success } | Measure-Object -Property ParameterCount -Sum).Sum

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MaterialInstance_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
材质实例生成报告
========================================
生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
父材质: $ParentMaterialPath
输出路径: $OutputPath

统计信息:
- 总实例数: $($Results.Count)
- 成功生成: $SuccessCount
- 生成失败: $FailCount
- 总参数数: $TotalParams

生成详情:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $($_.Name)"
    "    配置文件: $($_.ConfigPath)"
    "    参数数量: $($_.ParameterCount)"
    ""
} | Out-String)

失败的实例:
$($Results | Where-Object { -not $_.Success } | ForEach-Object {
    "  ✗ $($_.Name): $($_.Error)"
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
    Write-Log "开始生成材质实例" "INFO"
    Write-Log "========================================" "INFO"

    $Results = Process-MaterialInstances

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "材质实例生成完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "生成材质实例失败: $_" "ERROR"
    exit 1
}
