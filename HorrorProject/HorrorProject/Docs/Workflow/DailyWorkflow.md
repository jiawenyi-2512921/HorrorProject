# 日常工作流程

## 每日关卡设计流程

### 1. 开始工作前
- 启动Unreal Engine 5.6
- 检查项目完整性（验证资产）
- 拉取最新代码：`git pull origin main`
- 检查是否有冲突或更新

### 2. 关卡设计迭代
1. **规划阶段**
   - 在纸上或白板上草绘关卡布局
   - 确定关卡目标和玩家路径
   - 标记关键恐怖时刻和触发点

2. **灰盒阶段**
   - 使用基础几何体搭建关卡结构
   - 测试玩家移动和视线
   - 调整空间比例和节奏
   - 频繁进行游玩测试

3. **美术填充**
   - 替换灰盒为最终资产
   - 添加装饰物和细节
   - 设置光照和氛围
   - 添加音效和粒子效果

4. **优化和打磨**
   - 检查性能（FPS、内存）
   - 优化光照构建
   - 调整碰撞体积
   - 修复视觉问题

### 3. 测试流程
- **快速测试**：每30分钟进行一次快速游玩测试
- **完整测试**：每天结束前进行完整关卡测试
- **记录问题**：使用编辑器内标记或文档记录问题

### 4. 每日结束前
- 保存所有更改
- 运行光照构建（如果有重大光照更改）
- 提交更改到版本控制
- 更新任务进度

## 版本控制最佳实践

### Git工作流程

#### 每日开始
```bash
# 1. 确保在main分支
git checkout main

# 2. 拉取最新更改
git pull origin main

# 3. 创建功能分支（可选，用于大型功能）
git checkout -b feature/level-2-basement
```

#### 提交更改
```bash
# 1. 查看更改
git status

# 2. 添加文件（避免添加临时文件）
git add Content/Maps/Level_02.umap
git add Content/Blueprints/BP_DoorInteraction.uasset

# 3. 提交更改（使用清晰的提交信息）
git commit -m "feat: add basement level with door interaction system"

# 4. 推送到远程
git push origin main
```

#### 提交信息规范
- `feat:` - 新功能（新关卡、新机制）
- `fix:` - 修复问题
- `art:` - 美术资产更新
- `audio:` - 音频更新
- `perf:` - 性能优化
- `docs:` - 文档更新
- `refactor:` - 代码重构

示例：
```
feat: add second floor corridor with flickering lights
fix: resolve door collision issue in main hall
art: update basement textures and materials
audio: add ambient sound for kitchen area
perf: optimize light baking in level 1
```

### 避免提交的文件
已在`.gitignore`中配置，但需注意：
- `Saved/` - 临时文件和缓存
- `Intermediate/` - 编译中间文件
- `DerivedDataCache/` - 派生数据缓存
- `*.tmp` - 临时文件
- 大型源文件（使用Git LFS）

### 处理冲突
```bash
# 1. 拉取时发现冲突
git pull origin main

# 2. 查看冲突文件
git status

# 3. 对于二进制文件（.uasset, .umap）
# 选择保留本地或远程版本
git checkout --ours Content/Maps/Level_01.umap  # 保留本地
git checkout --theirs Content/Maps/Level_01.umap  # 保留远程

# 4. 标记为已解决
git add Content/Maps/Level_01.umap

# 5. 完成合并
git commit -m "merge: resolve level 1 map conflict"
```

## 备份策略

### 自动备份
- **Git版本控制**：每次提交都是一个备份点
- **Unreal自动保存**：每5分钟自动保存（已配置）
- **本地备份**：每日结束时创建本地备份

### 手动备份流程

#### 每日备份
```bash
# 创建备份目录
mkdir -p D:/Backups/HorrorProject/$(date +%Y%m%d)

# 复制关键文件
cp -r Content/Maps D:/Backups/HorrorProject/$(date +%Y%m%d)/
cp -r Content/Blueprints D:/Backups/HorrorProject/$(date +%Y%m%d)/
```

#### 里程碑备份
在完成重要功能或关卡时：
```bash
# 创建带标签的Git提交
git tag -a v0.1.0 -m "Milestone: Level 1 complete"
git push origin v0.1.0

# 创建完整项目备份
tar -czf HorrorProject_v0.1.0.tar.gz HorrorProject/
```

### 备份检查清单
- [ ] 每日提交到Git
- [ ] 每周验证备份完整性
- [ ] 每月创建完整项目归档
- [ ] 重要里程碑创建Git标签

### 恢复流程

#### 从Git恢复
```bash
# 查看提交历史
git log --oneline

# 恢复到特定提交
git checkout <commit-hash>

# 恢复特定文件
git checkout <commit-hash> -- Content/Maps/Level_01.umap
```

#### 从自动保存恢复
1. 打开Unreal Editor
2. File → Open Recent
3. 选择自动保存的版本（标记为AutoSave）

### 云备份（推荐）
- 使用GitHub作为主要远程仓库
- 考虑使用额外的云存储（OneDrive、Google Drive）
- 定期将重要里程碑上传到云端

## 工作时间建议

### 高效工作时段
- **上午**：复杂的关卡设计和蓝图编程
- **下午**：美术填充和细节打磨
- **晚上**：测试和优化

### 休息建议
- 每工作50分钟休息10分钟
- 避免连续工作超过2小时
- 定期远离屏幕保护眼睛

## 问题追踪

### 使用编辑器内标记
1. 在视口中右键 → Place Actor → Note
2. 添加问题描述
3. 设置优先级颜色（红色=高，黄色=中，绿色=低）

### 问题日志
在`Docs/Issues.md`中记录：
```markdown
## 2026-04-28
- [ ] 地下室门碰撞问题
- [ ] 走廊光照闪烁过于频繁
- [x] 音效音量过大
```

## 性能监控

### 每日性能检查
1. 打开Stat命令：
   - `stat fps` - 帧率
   - `stat unit` - 各系统耗时
   - `stat memory` - 内存使用

2. 目标性能：
   - FPS: 60+ (开发), 30+ (最低)
   - Frame Time: <16.67ms
   - Memory: <4GB

3. 如果性能下降，参考`OptimizationGuide.md`
