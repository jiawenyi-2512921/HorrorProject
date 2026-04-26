# CheckAssetReferences.ps1 - 检查资产引用
param(
    [Parameter(Mandatory=$true)]
    [string]$AssetPath,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AssetReferences.log",
    [switch]$FindUnused,
    [switch]$FindMissing
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

function Get-AssetReferences {
    Write-Log "扫描资产引用..."

    $References = @{
        Materials = @{}
        Textures = @{}
        Meshes = @{}
        Audio = @{}
    }

    # 扫描材质配置文件
    $MaterialFiles = Get-ChildItem -Path $AssetPath -Include "*Material*.json" -Recurse -File

    foreach ($MaterialFile in $MaterialFiles) {
        try {
            $MaterialConfig = Get-Content -Path $MaterialFile.FullName -Raw | ConvertFrom-Json

            $MaterialName = [System.IO.Path]::GetFileNameWithoutExtension($MaterialFile.Name)
            $References.Materials[$MaterialName] = @{
                File = $MaterialFile.FullName
                ReferencedTextures = @()
                ReferencedBy = @()
            }

            # 提取纹理引用
            if ($MaterialConfig.Parameters.TextureParameters) {
                foreach ($Texture in $MaterialConfig.Parameters.TextureParameters) {
                    if ($Texture.Value) {
                        $References.Materials[$MaterialName].ReferencedTextures += $Texture.Value
                    }
                }
            }
        }
        catch {
            Write-Log "解析材质文件失败: $($MaterialFile.Name) - $_" "ERROR"
        }
    }

    # 扫描网格配置文件
    $MeshFiles = Get-ChildItem -Path $AssetPath -Include "*.import.json" -Recurse -File

    foreach ($MeshFile in $MeshFiles) {
        try {
            $MeshConfig = Get-Content -Path $MeshFile.FullName -Raw | ConvertFrom-Json

            $MeshName = [System.IO.Path]::GetFileNameWithoutExtension($MeshFile.Name -replace '\.import$', '')
            $References.Meshes[$MeshName] = @{
                File = $MeshFile.FullName
                ReferencedMaterials = @()
                ReferencedBy = @()
            }

            # 提取材质引用
            if ($MeshConfig.ImportMaterials -and $MeshConfig.Materials) {
                foreach ($Material in $MeshConfig.Materials) {
                    $References.Meshes[$MeshName].ReferencedMaterials += $Material
                }
            }
        }
        catch {
            Write-Log "解析网格文件失败: $($MeshFile.Name) - $_" "ERROR"
        }
    }

    Write-Log "找到引用:"
    Write-Log "  材质: $($References.Materials.Count)"
    Write-Log "  网格: $($References.Meshes.Count)"

    return $References
}

function Find-UnusedAssets {
    param($References)

    Write-Log "查找未使用的资产..."

    $UnusedAssets = @{
        Textures = @()
        Materials = @()
        Meshes = @()
    }

    # 获取所有纹理文件
    $AllTextures = Get-ChildItem -Path $AssetPath -Include "*.png","*.jpg","*.jpeg","*.tga" -Recurse -File

    foreach ($Texture in $AllTextures) {
        $TextureName = [System.IO.Path]::GetFileNameWithoutExtension($Texture.Name)
        $IsReferenced = $false

        # 检查是否被材质引用
        foreach ($Material in $References.Materials.Values) {
            if ($Material.ReferencedTextures -contains $TextureName -or
                $Material.ReferencedTextures -match $TextureName) {
                $IsReferenced = $true
                break
            }
        }

        if (-not $IsReferenced) {
            $UnusedAssets.Textures += @{
                Name = $TextureName
                File = $Texture.FullName
                Size = $Texture.Length
            }
        }
    }

    # 查找未使用的材质
    foreach ($MaterialName in $References.Materials.Keys) {
        $Material = $References.Materials[$MaterialName]
        $IsReferenced = $false

        # 检查是否被网格引用
        foreach ($Mesh in $References.Meshes.Values) {
            if ($Mesh.ReferencedMaterials -contains $MaterialName) {
                $IsReferenced = $true
                break
            }
        }

        if (-not $IsReferenced) {
            $UnusedAssets.Materials += @{
                Name = $MaterialName
                File = $Material.File
            }
        }
    }

    Write-Log "未使用的资产:"
    Write-Log "  纹理: $($UnusedAssets.Textures.Count)"
    Write-Log "  材质: $($UnusedAssets.Materials.Count)"

    return $UnusedAssets
}

function Find-MissingReferences {
    param($References)

    Write-Log "查找缺失的引用..."

    $MissingReferences = @{
        Textures = @()
        Materials = @()
    }

    # 获取所有实际存在的纹理
    $ExistingTextures = Get-ChildItem -Path $AssetPath -Include "*.png","*.jpg","*.jpeg","*.tga" -Recurse -File |
        ForEach-Object { [System.IO.Path]::GetFileNameWithoutExtension($_.Name) }

    # 检查材质引用的纹理是否存在
    foreach ($MaterialName in $References.Materials.Keys) {
        $Material = $References.Materials[$MaterialName]

        foreach ($TextureRef in $Material.ReferencedTextures) {
            $TextureName = Split-Path $TextureRef -Leaf
            $TextureName = [System.IO.Path]::GetFileNameWithoutExtension($TextureName)

            if ($ExistingTextures -notcontains $TextureName) {
                $MissingReferences.Textures += @{
                    ReferencedBy = $MaterialName
                    MissingTexture = $TextureRef
                }
            }
        }
    }

    # 获取所有实际存在的材质
    $ExistingMaterials = $References.Materials.Keys

    # 检查网格引用的材质是否存在
    foreach ($MeshName in $References.Meshes.Keys) {
        $Mesh = $References.Meshes[$MeshName]

        foreach ($MaterialRef in $Mesh.ReferencedMaterials) {
            if ($ExistingMaterials -notcontains $MaterialRef) {
                $MissingReferences.Materials += @{
                    ReferencedBy = $MeshName
                    MissingMaterial = $MaterialRef
                }
            }
        }
    }

    Write-Log "缺失的引用:"
    Write-Log "  纹理: $($MissingReferences.Textures.Count)"
    Write-Log "  材质: $($MissingReferences.Materials.Count)"

    return $MissingReferences
}

function Generate-Report {
    param($References, $UnusedAssets, $MissingReferences)

    Write-Log "生成引用检查报告..."

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\AssetReferences_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
资产引用检查报告
========================================
检查时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
资产路径: $AssetPath

引用统计:
- 材质数: $($References.Materials.Count)
- 网格数: $($References.Meshes.Count)

"@

    if ($FindUnused -and $null -ne $UnusedAssets) {
        $UnusedTextureSize = ($UnusedAssets.Textures | Measure-Object -Property Size -Sum).Sum

        $Report += @"

未使用的资产:
纹理 ($($UnusedAssets.Textures.Count) 个, $([math]::Round($UnusedTextureSize / 1MB, 2)) MB):
$($UnusedAssets.Textures | ForEach-Object {
    "  - $($_.Name) ($([math]::Round($_.Size / 1KB, 2)) KB)"
} | Out-String)

材质 ($($UnusedAssets.Materials.Count) 个):
$($UnusedAssets.Materials | ForEach-Object {
    "  - $($_.Name)"
} | Out-String)

"@
    }

    if ($FindMissing -and $null -ne $MissingReferences) {
        $Report += @"

缺失的引用:
纹理引用 ($($MissingReferences.Textures.Count) 个):
$($MissingReferences.Textures | ForEach-Object {
    "  ✗ $($_.MissingTexture)"
    "    被引用于: $($_.ReferencedBy)"
    ""
} | Out-String)

材质引用 ($($MissingReferences.Materials.Count) 个):
$($MissingReferences.Materials | ForEach-Object {
    "  ✗ $($_.MissingMaterial)"
    "    被引用于: $($_.ReferencedBy)"
    ""
} | Out-String)

"@
    }

    $Report += @"
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
    Write-Log "开始检查资产引用" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $AssetPath)) {
        throw "资产路径不存在: $AssetPath"
    }

    $References = Get-AssetReferences

    $UnusedAssets = $null
    if ($FindUnused) {
        $UnusedAssets = Find-UnusedAssets -References $References
    }

    $MissingReferences = $null
    if ($FindMissing) {
        $MissingReferences = Find-MissingReferences -References $References
    }

    $ReportPath = Generate-Report -References $References -UnusedAssets $UnusedAssets -MissingReferences $MissingReferences

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "资产引用检查完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "报告路径: $ReportPath" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "资产引用检查失败: $_" "ERROR"
    exit 1
}
