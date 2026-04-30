# 协作指南

## 文件命名规范

### 通用规则
- 使用英文命名（避免中文字符）
- 使用PascalCase（每个单词首字母大写）
- 名称要描述性强，一目了然
- 避免使用空格，使用下划线分隔（如需要）
- 避免使用特殊字符（除了下划线）

### 资产命名前缀

#### 蓝图（Blueprints）
```
BP_         - 通用蓝图
BP_AI_      - AI相关
BP_Char_    - 角色
BP_Item_    - 物品
BP_Interact_- 交互物体
BP_Trigger_ - 触发器
BP_Widget_  - UI组件

示例：
BP_DoorInteraction
BP_AI_Ghost
BP_Char_Player
BP_Item_Flashlight
BP_Trigger_JumpScare
```

#### 材质（Materials）
```
M_          - 材质
MI_         - 材质实例
MF_         - 材质函数
MP_         - 材质参数集合

示例：
M_Wall_Concrete
MI_Wall_Concrete_Dirty
MF_Noise_Generator
```

#### 纹理（Textures）
```
T_          - 纹理
T_*_D       - 漫反射/基础颜色
T_*_N       - 法线贴图
T_*_R       - 粗糙度
T_*_M       - 金属度
T_*_AO      - 环境光遮蔽
T_*_E       - 自发光
T_*_H       - 高度图
T_*_A       - Alpha/透明度

示例：
T_Floor_Wood_D
T_Floor_Wood_N
T_Floor_Wood_R
```

#### 静态网格（Static Meshes）
```
SM_         - 静态网格
SM_Prop_    - 道具
SM_Arch_    - 建筑元素
SM_Env_     - 环境物体

示例：
SM_Prop_Chair
SM_Arch_Door
SM_Env_Tree
```

#### 骨骼网格（Skeletal Meshes）
```
SK_         - 骨骼网格
SKM_        - 骨骼网格（替代）

示例：
SK_Ghost
SK_Player
```

#### 动画（Animations）
```
A_          - 动画序列
ABP_        - 动画蓝图
ABS_        - 动画混合空间
AM_         - 动画蒙太奇

示例：
A_Player_Walk
A_Player_Run
ABP_Player
ABS_Movement
```

#### 音频（Audio）
```
A_          - 音频文件
AC_         - 音频提示（Audio Cue）
AM_         - 音频混音器
AS_         - 音频衰减设置

示例：
A_Footstep_Wood
AC_Ambient_Wind
AM_Master
AS_3D_Default
```

#### 粒子系统（Particles）
```
P_          - 粒子系统（Cascade）
NS_         - Niagara系统
NE_         - Niagara发射器

示例：
P_Dust_Particles
NS_Blood_Splatter
```

#### 关卡（Levels）
```
L_          - 关卡
L_*_Main    - 主关卡
L_*_Sub     - 子关卡
L_*_Audio   - 音频子关卡
L_*_Lighting- 光照子关卡

示例：
L_Mansion_Main
L_Mansion_FirstFloor
L_Mansion_Audio
```

#### UI（User Interface）
```
WBP_        - Widget蓝图
T_UI_       - UI纹理

示例：
WBP_MainMenu
WBP_HUD
T_UI_Button_Normal
```

### 版本命名
```
资产名_v01  - 第一版
资产名_v02  - 第二版
资产名_WIP  - 进行中（Work In Progress）
资产名_Final- 最终版本

示例：
M_Wall_Brick_v01
M_Wall_Brick_v02
M_Wall_Brick_Final
```

## 文件夹结构

### 标准项目结构
```
Content/
├── Blueprints/              # 蓝图
│   ├── Characters/          # 角色蓝图
│   │   ├── Player/
│   │   └── AI/
│   ├── Interactables/       # 可交互物体
│   │   ├── Doors/
│   │   ├── Items/
│   │   └── Triggers/
│   ├── GameModes/           # 游戏模式
│   ├── UI/                  # UI蓝图
│   └── Utilities/           # 工具蓝图
│
├── Maps/                    # 关卡
│   ├── L_MainMenu
│   ├── L_Level_01/
│   │   ├── L_Level_01_Main
│   │   ├── L_Level_01_Audio
│   │   └── L_Level_01_Lighting
│   └── L_Level_02/
│
├── Materials/               # 材质
│   ├── Master/              # 主材质
│   ├── Instances/           # 材质实例
│   ├── Functions/           # 材质函数
│   └── PostProcess/         # 后处理材质
│
├── Meshes/                  # 网格
│   ├── Architecture/        # 建筑
│   │   ├── Walls/
│   │   ├── Floors/
│   │   ├── Doors/
│   │   └── Windows/
│   ├── Props/               # 道具
│   │   ├── Furniture/
│   │   ├── Decorations/
│   │   └── Interactive/
│   └── Characters/          # 角色网格
│
├── Textures/                # 纹理
│   ├── Architecture/
│   ├── Props/
│   ├── Characters/
│   └── UI/
│
├── Audio/                   # 音频
│   ├── Music/               # 音乐
│   ├── SFX/                 # 音效
│   │   ├── Ambient/         # 环境音
│   │   ├── Footsteps/       # 脚步声
│   │   ├── Interactions/    # 交互音效
│   │   └── Horror/          # 恐怖音效
│   ├── Cues/                # 音频提示
│   └── Mixers/              # 混音器
│
├── Animations/              # 动画
│   ├── Player/
│   └── AI/
│
├── Particles/               # 粒子效果
│   ├── Environment/
│   └── Effects/
│
├── UI/                      # UI资产
│   ├── Textures/
│   ├── Fonts/
│   └── Widgets/
│
└── ThirdParty/              # 第三方资产
    ├── BodycamVHS/
    └── [其他插件]
```

### 文件夹命名规则
- 使用PascalCase
- 名称清晰描述内容
- 按功能或类型分组
- 保持层级不超过4-5层

### 临时文件夹
```
Content/
└── _WIP/                    # 进行中的工作
    ├── [你的名字]/          # 个人工作区
    └── _Testing/            # 测试资产
```

**注意**：`_WIP`文件夹中的内容不应该被其他资产引用，完成后移动到正式文件夹。

## 提交规范

### 提交频率
- **小改动**：每完成一个功能点就提交
- **大改动**：每工作1-2小时提交一次
- **每日结束**：必须提交当天的工作

### 提交信息格式

#### 基本格式
```
<类型>: <简短描述>

[可选的详细描述]

[可选的相关问题]
```

#### 类型标签
```
feat:     新功能
fix:      修复问题
art:      美术资产更新
audio:    音频更新
level:    关卡设计
perf:     性能优化
refactor: 代码重构
docs:     文档更新
test:     测试相关
chore:    杂项（构建、配置等）
```

#### 示例

**好的提交信息**：
```
feat: add basement level with interactive door system

- Created L_Level_02_Basement map
- Implemented BP_DoorInteraction with key requirement
- Added door opening/closing animations
- Integrated footstep sounds for basement floor

Closes #12
```

```
fix: resolve player falling through floor in kitchen

The collision mesh for SM_Floor_Kitchen was missing.
Added proper collision and tested movement.
```

```
art: update mansion exterior textures

- Replaced placeholder textures with final assets
- Added weathering details to walls
- Improved normal maps for brick surfaces
```

**不好的提交信息**：
```
update stuff
fixed bug
WIP
test
```

### 提交前检查清单
- [ ] 代码/蓝图无编译错误
- [ ] 测试过改动的功能
- [ ] 没有包含临时文件
- [ ] 提交信息清晰描述改动
- [ ] 相关资产都已添加

### 提交工作流

#### 日常提交
```bash
# 1. 查看改动
git status

# 2. 添加文件（选择性添加）
git add Content/Maps/L_Level_01.umap
git add Content/Blueprints/BP_DoorInteraction.uasset

# 3. 提交
git commit -m "feat: add interactive door to main hall"

# 4. 推送
git push origin main
```

#### 大型功能提交
```bash
# 1. 创建功能分支（可选）
git checkout -b feature/basement-level

# 2. 多次小提交
git add ...
git commit -m "feat: create basement level layout"

git add ...
git commit -m "feat: add basement lighting"

git add ...
git commit -m "feat: add basement audio"

# 3. 合并到主分支
git checkout main
git merge feature/basement-level
git push origin main
```

### 不应该提交的内容
- `Saved/` - 临时文件和缓存
- `Intermediate/` - 编译中间文件
- `DerivedDataCache/` - 派生数据
- `*.tmp` - 临时文件
- 个人配置文件
- 大型源文件（应使用Git LFS）

### 提交冲突处理

#### 预防冲突
1. 工作前先拉取最新代码：
   ```bash
   git pull origin main
   ```

2. 频繁提交和推送

3. 沟通正在修改的文件

#### 解决冲突
```bash
# 1. 拉取时发现冲突
git pull origin main
# Auto-merging Content/Maps/L_Level_01.umap
# CONFLICT (content): Merge conflict in Content/Maps/L_Level_01.umap

# 2. 查看冲突文件
git status

# 3. 对于二进制文件（.uasset, .umap）
# 选择一个版本：
git checkout --ours Content/Maps/L_Level_01.umap   # 保留本地
# 或
git checkout --theirs Content/Maps/L_Level_01.umap # 保留远程

# 4. 如果两个版本都重要，手动合并：
# - 备份本地版本
# - 接受远程版本
# - 在编辑器中手动重新应用本地改动

# 5. 标记为已解决
git add Content/Maps/L_Level_01.umap

# 6. 完成合并
git commit -m "merge: resolve level 1 map conflict"
```

## 协作最佳实践

### 沟通
1. **开始工作前**：
   - 告知团队你要修改的文件
   - 检查是否有人正在修改相同文件

2. **工作中**：
   - 定期更新进度
   - 遇到问题及时沟通
   - 共享有用的发现

3. **完成后**：
   - 通知团队已完成的功能
   - 说明如何测试
   - 记录已知问题

### 文件锁定
对于关键的二进制文件（关卡、重要蓝图），考虑使用文件锁定：

```bash
# 锁定文件（如果使用Git LFS）
git lfs lock Content/Maps/L_Level_01_Main.umap

# 完成后解锁
git lfs unlock Content/Maps/L_Level_01_Main.umap
```

### 代码审查
1. **提交前自查**：
   - 检查代码质量
   - 测试功能
   - 确保符合规范

2. **请求审查**：
   - 对于重要功能，请团队成员审查
   - 说明改动的目的和影响

3. **审查他人代码**：
   - 提供建设性反馈
   - 测试功能
   - 检查是否符合规范

### 资产共享
1. **创建可复用资产**：
   - 使用材质实例而非复制材质
   - 创建蓝图组件库
   - 共享常用函数

2. **文档化**：
   - 为复杂蓝图添加注释
   - 创建使用说明
   - 记录参数含义

3. **版本管理**：
   - 不要随意修改共享资产
   - 需要修改时先沟通
   - 考虑创建变体而非修改原始资产

### 关卡协作
1. **使用子关卡**：
   - 将大关卡分割为多个子关卡
   - 每人负责不同的子关卡
   - 减少冲突

2. **关卡流送**：
   ```
   L_Level_01_Main        - 主关卡（基础结构）
   L_Level_01_Geometry    - 几何体
   L_Level_01_Lighting    - 光照
   L_Level_01_Audio       - 音频
   L_Level_01_Gameplay    - 游戏逻辑
   ```

3. **命名约定**：
   - 明确每个子关卡的用途
   - 使用一致的命名

### 测试协作
1. **互相测试**：
   - 测试他人的功能
   - 提供反馈
   - 报告问题

2. **问题追踪**：
   - 使用统一的问题追踪系统
   - 清晰描述问题
   - 包含复现步骤

3. **回归测试**：
   - 修复问题后重新测试
   - 确保没有引入新问题

## 团队工作流程

### 每日站会（可选）
- 昨天完成了什么
- 今天计划做什么
- 遇到什么问题

### 每周回顾
- 回顾本周进度
- 讨论遇到的问题
- 计划下周工作

### 里程碑
- 设定清晰的里程碑
- 定期检查进度
- 庆祝完成的里程碑

## 工具和资源

### 推荐工具
- **Git客户端**：GitHub Desktop, SourceTree, GitKraken
- **沟通工具**：Discord, Slack, Microsoft Teams
- **任务管理**：Trello, Notion, Jira
- **文档**：Markdown编辑器, Notion

### 有用的Git命令
```bash
# 查看提交历史
git log --oneline --graph

# 查看文件改动
git diff

# 撤销未提交的改动
git checkout -- <file>

# 查看远程仓库
git remote -v

# 拉取特定分支
git pull origin <branch-name>

# 创建标签
git tag -a v1.0.0 -m "Version 1.0.0"
git push origin v1.0.0
```

### 学习资源
- Git文档：https://git-scm.com/doc
- Unreal Engine协作：https://docs.unrealengine.com/en-US/ProductionPipelines/
- Git LFS：https://git-lfs.github.com/

## 问题解决

### 常见协作问题

#### 问题：频繁的合并冲突
**解决方案**：
- 更频繁地拉取和推送
- 使用子关卡分离工作
- 提前沟通正在修改的文件

#### 问题：资产引用丢失
**解决方案**：
- 不要随意移动或重命名资产
- 使用编辑器的重定向功能
- 提交前验证引用完整性

#### 问题：性能下降
**解决方案**：
- 定期进行性能检查
- 遵循优化指南
- 及时清理未使用资产

#### 问题：版本不一致
**解决方案**：
- 确保所有人使用相同的引擎版本
- 同步插件版本
- 记录项目配置

## 总结

良好的协作需要：
- 清晰的命名和组织
- 频繁的沟通
- 规范的提交
- 互相尊重和支持

遵循这些指南将帮助团队高效协作，减少冲突，提高生产力。
