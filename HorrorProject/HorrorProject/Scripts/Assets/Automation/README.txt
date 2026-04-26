========================================
资产自动化处理系统
========================================

本目录包含完整的UE5资产自动化处理脚本集合，用于批量导入、处理、优化和验证游戏资产。

目录结构
========================================

音频资产自动化/
├── AudioBatchImport.ps1          - 批量导入音频文件
├── AudioNormalization.ps1        - 音频标准化处理（LUFS标准化）
├── AudioFormatConversion.ps1     - 音频格式转换
└── GenerateAudioMetadata.ps1     - 生成音频元数据

纹理资产自动化/
├── TextureBatchImport.ps1        - 批量导入纹理文件
├── TextureCompression.ps1        - 纹理压缩优化
├── GenerateMipmaps.ps1           - 生成Mipmap链
└── TextureAtlasGeneration.ps1    - 纹理图集生成

材质资产自动化/
├── MaterialInstanceGeneration.ps1 - 材质实例生成
├── MaterialParameterBatch.ps1     - 批量设置材质参数
└── MaterialOptimization.ps1       - 材质优化

网格资产自动化/
├── MeshBatchImport.ps1           - 批量导入网格文件
├── LODGeneration.ps1             - LOD自动生成
├── CollisionGeneration.ps1       - 碰撞体生成
└── MeshOptimization.ps1          - 网格优化

资产验证自动化/
├── ValidateAllAssets.ps1         - 验证所有资产
├── CheckAssetReferences.ps1      - 检查资产引用
├── FindBrokenAssets.ps1          - 查找损坏资产
└── GenerateAssetReport.ps1       - 生成资产报告


使用说明
========================================

1. 音频资产处理
----------------------------------------

批量导入音频：
.\AudioBatchImport.ps1 -SourcePath "D:\Audio\Source" -DestinationPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio"

音频标准化（-16 LUFS）：
.\AudioNormalization.ps1 -AudioPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio" -TargetLUFS -16.0 -ProcessSubfolders

格式转换：
.\AudioFormatConversion.ps1 -SourcePath "D:\Audio" -TargetFormat "ogg" -SampleRate 48000 -BitRate 320

生成元数据：
.\GenerateAudioMetadata.ps1 -AudioPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio" -IncludeWaveform


2. 纹理资产处理
----------------------------------------

批量导入纹理：
.\TextureBatchImport.ps1 -SourcePath "D:\Textures\Source" -DestinationPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Textures" -TextureType "Default"

纹理压缩：
.\TextureCompression.ps1 -TexturePath "D:\gptzuo\HorrorProject\HorrorProject\Content\Textures" -Quality "Medium" -CompressionFormat "BC7" -MaxTextureSize 2048

生成Mipmap：
.\GenerateMipmaps.ps1 -TexturePath "D:\gptzuo\HorrorProject\HorrorProject\Content\Textures" -FilterType "Kaiser" -ProcessSubfolders

生成纹理图集：
.\TextureAtlasGeneration.ps1 -TexturePath "D:\Textures\UI" -OutputPath "D:\Output" -AtlasSize 2048 -GenerateMetadata


3. 材质资产处理
----------------------------------------

生成材质实例：
.\MaterialInstanceGeneration.ps1 -ParentMaterialPath "/Game/Materials/M_Master" -OutputPath "D:\Output" -InstanceCount 10

批量设置参数：
.\MaterialParameterBatch.ps1 -MaterialPath "D:\Materials" -ParameterFile "D:\Config\params.json" -CreateBackup

材质优化：
.\MaterialOptimization.ps1 -MaterialPath "D:\Materials" -OptimizationLevel "Medium" -RemoveUnusedNodes -SimplifyExpressions


4. 网格资产处理
----------------------------------------

批量导入网格：
.\MeshBatchImport.ps1 -SourcePath "D:\Meshes\Source" -DestinationPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Meshes" -GenerateLODs -GenerateCollision

生成LOD：
.\LODGeneration.ps1 -MeshPath "D:\Meshes" -LODCount 3 -ReductionPercentage 0.5 -ProcessSubfolders

生成碰撞体：
.\CollisionGeneration.ps1 -MeshPath "D:\Meshes" -CollisionType "Convex" -MaxHullCount 4 -MaxHullVerts 16

网格优化：
.\MeshOptimization.ps1 -MeshPath "D:\Meshes" -OptimizationLevel "Medium" -RemoveDegenerates -WeldVertices -OptimizeForGPU


5. 资产验证
----------------------------------------

验证所有资产：
.\ValidateAllAssets.ps1 -AssetPath "D:\gptzuo\HorrorProject\HorrorProject\Content" -DetailedReport

检查资产引用：
.\CheckAssetReferences.ps1 -AssetPath "D:\gptzuo\HorrorProject\HorrorProject\Content" -FindUnused -FindMissing

查找损坏资产：
.\FindBrokenAssets.ps1 -AssetPath "D:\gptzuo\HorrorProject\HorrorProject\Content" -Quarantine

生成资产报告：
.\GenerateAssetReport.ps1 -AssetPath "D:\gptzuo\HorrorProject\HorrorProject\Content" -GenerateHTML


依赖工具
========================================

必需工具：
- FFmpeg：音频/视频处理
- ImageMagick：图像处理
- UE5 命令行工具：资产导入

安装FFmpeg：
winget install FFmpeg

安装ImageMagick：
winget install ImageMagick


配置说明
========================================

所有脚本支持以下通用参数：
-LogPath：日志文件路径
-CreateBackup：创建备份（如适用）
-ProcessSubfolders：递归处理子文件夹

日志位置：
D:\gptzuo\HorrorProject\HorrorProject\Logs\

报告位置：
D:\gptzuo\HorrorProject\HorrorProject\Reports\


批处理示例
========================================

完整资产导入流程：

# 1. 导入音频
.\AudioBatchImport.ps1 -SourcePath "D:\Assets\Audio" -DestinationPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio"

# 2. 标准化音频
.\AudioNormalization.ps1 -AudioPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio" -TargetLUFS -16.0 -ProcessSubfolders

# 3. 导入纹理
.\TextureBatchImport.ps1 -SourcePath "D:\Assets\Textures" -DestinationPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Textures"

# 4. 压缩纹理
.\TextureCompression.ps1 -TexturePath "D:\gptzuo\HorrorProject\HorrorProject\Content\Textures" -Quality "Medium" -ProcessSubfolders

# 5. 导入网格
.\MeshBatchImport.ps1 -SourcePath "D:\Assets\Meshes" -DestinationPath "D:\gptzuo\HorrorProject\HorrorProject\Content\Meshes" -GenerateLODs -GenerateCollision

# 6. 验证资产
.\ValidateAllAssets.ps1 -AssetPath "D:\gptzuo\HorrorProject\HorrorProject\Content" -DetailedReport

# 7. 生成报告
.\GenerateAssetReport.ps1 -AssetPath "D:\gptzuo\HorrorProject\HorrorProject\Content" -GenerateHTML


性能优化建议
========================================

1. 纹理优化：
   - 使用BC7压缩格式（高质量）
   - 限制最大尺寸为2048x2048
   - 生成完整Mipmap链

2. 音频优化：
   - 使用OGG格式（压缩率高）
   - 标准化到-16 LUFS
   - 采样率48kHz

3. 网格优化：
   - 生成3-4级LOD
   - 使用凸包碰撞（4-8个凸包）
   - 启用GPU缓存优化

4. 材质优化：
   - 限制指令数<256
   - 限制纹理采样<12
   - 移除未使用节点


故障排除
========================================

常见问题：

1. FFmpeg未找到
   解决：确保FFmpeg在PATH中，或使用完整路径

2. ImageMagick未找到
   解决：安装ImageMagick并添加到PATH

3. 权限错误
   解决：以管理员身份运行PowerShell

4. 文件锁定
   解决：关闭UE编辑器后再运行脚本

5. 内存不足
   解决：分批处理大量资产


技术支持
========================================

日志位置：D:\gptzuo\HorrorProject\HorrorProject\Logs\
报告位置：D:\gptzuo\HorrorProject\HorrorProject\Reports\

所有脚本都会生成详细的日志和报告，包含：
- 处理进度
- 成功/失败统计
- 错误详情
- 性能指标


版本信息
========================================

版本：1.0.0
创建日期：2026-04-26
适用于：Unreal Engine 5.6
项目：HorrorProject

========================================
