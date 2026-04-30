# 故障排除指南

## 常见问题和解决方案

### 编辑器问题

#### 编辑器启动失败
**症状**：双击项目文件后编辑器无法启动

**解决方案**：
1. 删除以下文件夹：
   ```bash
   rm -rf Saved/
   rm -rf Intermediate/
   rm -rf DerivedDataCache/
   ```

2. 右键`.uproject`文件 → Generate Visual Studio project files

3. 重新启动编辑器

#### 编辑器崩溃
**症状**：编辑器突然关闭或冻结

**解决方案**：
1. 查看崩溃日志：
   ```
   Saved/Logs/HorrorProject.log
   Saved/Crashes/
   ```

2. 常见原因：
   - 内存不足：关闭其他程序
   - 资产损坏：恢复到上一个版本
   - 插件冲突：禁用最近安装的插件

3. 使用安全模式启动：
   - 右键`.uproject` → Switch Unreal Engine version
   - 选择当前版本重新关联

#### 资产无法打开
**症状**：双击资产时显示错误或无响应

**解决方案**：
1. 验证资产完整性：
   - 右键资产 → Asset Actions → Validate

2. 重新导入资产：
   - 找到源文件
   - 右键 → Reimport

3. 如果是蓝图：
   - 右键蓝图 → Asset Actions → Refresh
   - 或者 → Compile → Full Recompile

### 光照问题

#### 光照构建失败
**症状**：Build Lighting时出现错误

**解决方案**：
1. 检查Lightmass Importance Volume：
   - 确保关卡被完全包围
   - 不要有多个重叠的Volume

2. 检查光照贴图分辨率：
   - 过高的分辨率会导致失败
   - 建议：墙壁64-128，小物体16-32

3. 清理光照缓存：
   ```bash
   rm -rf Saved/Logs/Swarm/
   ```

4. 重启Swarm Agent：
   - 关闭编辑器
   - 任务管理器结束SwarmAgent.exe
   - 重新启动编辑器

#### 光照有黑斑或闪烁
**症状**：构建后出现不自然的黑色区域

**解决方案**：
1. 增加光照贴图分辨率：
   - 选择受影响的网格体
   - Static Mesh Settings → Light Map Resolution: 提高到128或256

2. 调整光照构建质量：
   - World Settings → Lightmass Settings
   - Static Lighting Level Scale: 降低到1.0或0.5

3. 检查UV通道：
   - 静态网格编辑器 → UV Channel 1
   - 确保没有重叠

4. 增加光照反弹次数：
   - Num Indirect Lighting Bounces: 3

#### 光照构建时间过长
**症状**：构建光照需要数小时

**解决方案**：
1. 开发阶段使用快速设置：
   - Static Lighting Level Scale: 2.0
   - Num Indirect Lighting Bounces: 1
   - Indirect Lighting Quality: 0.5

2. 减少Stationary光源数量：
   - 每个区域最多4个
   - 其他改为Static

3. 优化光照贴图分辨率：
   - 不重要的物体降低分辨率
   - 使用批量编辑

4. 使用GPU光照构建：
   - Edit → Project Settings → Rendering
   - 启用 GPU Lightmass（如果支持）

### 性能问题

#### 帧率过低（<30 FPS）
**症状**：游戏运行卡顿

**诊断**：
```
控制台命令：
stat fps - 查看帧率
stat unit - 查看各系统耗时
stat game - 游戏线程
stat gpu - GPU耗时
```

**解决方案**：

1. **如果Game Thread过高（>16ms）**：
   - 减少Tick事件使用
   - 优化蓝图逻辑
   - 减少碰撞检测

2. **如果Draw Thread过高**：
   - 减少Draw Call
   - 使用实例化网格
   - 合并材质

3. **如果GPU过高**：
   - 降低光照复杂度
   - 优化材质
   - 减少后处理效果

4. **通用优化**：
   - 启用LOD
   - 设置剔除距离
   - 优化阴影设置

#### 内存占用过高
**症状**：内存使用超过4GB，可能导致崩溃

**诊断**：
```
stat memory - 查看内存使用
memreport - 生成详细报告
```

**解决方案**：
1. 优化纹理：
   - 降低纹理分辨率
   - 使用正确的压缩格式
   - 启用纹理流送

2. 使用软引用：
   - 将硬引用改为软引用
   - 异步加载资产

3. 清理未使用资产：
   - Edit → Editor Preferences → Loading & Saving
   - 启用 Auto-delete unused assets

4. 优化音频：
   - 使用流式加载
   - 压缩音频文件

#### 加载时间过长
**症状**：关卡加载需要很长时间

**解决方案**：
1. 使用关卡流送：
   - 将大关卡分割为子关卡
   - 使用Level Streaming Volumes

2. 异步加载资产：
   - 使用Soft Object References
   - 在后台加载

3. 优化启动资产：
   - 减少默认加载的资产
   - 延迟加载非必要内容

### 蓝图问题

#### 蓝图编译错误
**症状**：蓝图显示红色错误图标

**解决方案**：
1. 打开蓝图查看错误信息
2. 常见错误：
   - 缺失引用：重新连接节点
   - 类型不匹配：检查变量类型
   - 循环依赖：重构蓝图结构

3. 强制重新编译：
   - File → Refresh All Nodes
   - Compile → Full Recompile

#### 蓝图逻辑不工作
**症状**：蓝图没有按预期执行

**调试方法**：
1. 使用断点：
   - 右键节点 → Add Breakpoint
   - 运行游戏，执行会暂停

2. 使用Print String：
   - 在关键位置添加Print String节点
   - 查看执行流程

3. 检查事件触发：
   - 确保事件被正确调用
   - 检查碰撞设置

4. 查看变量值：
   - 在调试时查看变量面板
   - 使用Watch功能

#### 蓝图性能问题
**症状**：蓝图导致性能下降

**诊断**：
```
Window → Developer Tools → Blueprint Profiler
```

**解决方案**：
1. 优化Tick事件：
   - 改用Timer
   - 减少执行频率

2. 减少复杂计算：
   - 缓存计算结果
   - 使用查找表

3. 优化数组操作：
   - 避免在Tick中遍历
   - 使用Set代替Array（查找操作）

### 碰撞问题

#### 碰撞不工作
**症状**：物体穿过其他物体

**解决方案**：
1. 检查碰撞设置：
   - 选择物体 → Details → Collision
   - Collision Presets: BlockAll

2. 检查碰撞通道：
   - 确保两个物体的碰撞通道匹配
   - Project Settings → Collision

3. 检查碰撞体积：
   - 静态网格编辑器 → Collision
   - 确保有碰撞网格

4. 启用碰撞可视化：
   - 视口 → Show → Collision

#### 碰撞过于敏感
**症状**：角色被小物体阻挡

**解决方案**：
1. 简化碰撞：
   - 使用简单碰撞体（Box、Sphere）
   - 避免复杂碰撞网格

2. 调整碰撞通道：
   - 装饰物：Overlap而非Block
   - 使用自定义碰撞通道

3. 调整角色碰撞：
   - 增大胶囊体半径
   - 启用Step Up功能

### 音频问题

#### 音频不播放
**症状**：触发音效但听不到声音

**解决方案**：
1. 检查音量设置：
   - Sound Cue → Volume Multiplier
   - Audio Component → Volume

2. 检查衰减设置：
   - 确保在衰减范围内
   - 增大Falloff Distance

3. 检查音频格式：
   - 确保正确导入
   - 重新导入音频文件

4. 检查音频设备：
   - 系统音量设置
   - 编辑器音频设置

#### 音频卡顿或延迟
**症状**：音频播放不流畅

**解决方案**：
1. 使用流式加载：
   - Sound Wave → Loading Behavior: Stream

2. 降低音频质量：
   - Compression Quality: 40-60

3. 限制同时播放数量：
   - Sound Class → Max Concurrent Play Count

4. 优化音频格式：
   - 使用OGG而非WAV

### Git问题

#### 合并冲突
**症状**：git pull时出现冲突

**解决方案**：
1. 查看冲突文件：
   ```bash
   git status
   ```

2. 对于二进制文件（.uasset, .umap）：
   ```bash
   # 保留本地版本
   git checkout --ours path/to/file

   # 或保留远程版本
   git checkout --theirs path/to/file
   ```

3. 标记为已解决：
   ```bash
   git add path/to/file
   git commit -m "merge: resolve conflict"
   ```

#### 推送失败
**症状**：git push被拒绝

**解决方案**：
1. 先拉取远程更改：
   ```bash
   git pull origin main
   ```

2. 解决冲突后再推送：
   ```bash
   git push origin main
   ```

3. 如果需要强制推送（谨慎使用）：
   ```bash
   git push --force-with-lease origin main
   ```

#### 文件过大无法提交
**症状**：Git提示文件过大

**解决方案**：
1. 使用Git LFS：
   ```bash
   git lfs install
   git lfs track "*.uasset"
   git lfs track "*.umap"
   git add .gitattributes
   ```

2. 检查`.gitignore`：
   - 确保大型临时文件被忽略

## 编译错误处理

### C++编译错误

#### 找不到头文件
**错误**：`fatal error: 'XXX.h' file not found`

**解决方案**：
1. 检查include路径
2. 重新生成项目文件：
   ```bash
   右键.uproject → Generate Visual Studio project files
   ```

3. 清理并重新编译：
   ```bash
   Build → Clean Solution
   Build → Rebuild Solution
   ```

#### 链接错误
**错误**：`unresolved external symbol`

**解决方案**：
1. 检查模块依赖：
   - 打开`HorrorProject.Build.cs`
   - 添加缺失的模块到`PublicDependencyModuleNames`

2. 重新编译

#### 语法错误
**错误**：各种C++语法错误

**解决方案**：
1. 仔细阅读错误信息
2. 检查：
   - 缺少分号
   - 括号不匹配
   - 类型不匹配
   - 命名空间问题

3. 使用IDE的错误提示

### 蓝图编译错误

#### 节点错误
**错误**：红色节点或连接

**解决方案**：
1. 悬停查看错误信息
2. 常见问题：
   - 类型不匹配：添加转换节点
   - 缺失引用：重新设置引用
   - 已删除的变量：移除或重新创建

#### 循环依赖
**错误**：`Circular dependency detected`

**解决方案**：
1. 使用接口（Interface）
2. 使用事件调度器
3. 重构蓝图结构

## 性能问题诊断

### 诊断流程

1. **确定瓶颈**：
   ```
   stat unit - 查看总体情况
   stat game - 游戏逻辑
   stat gpu - GPU渲染
   stat memory - 内存使用
   ```

2. **详细分析**：
   ```
   stat slow - 显示慢函数
   stat startfile / stat stopfile - 记录性能数据
   ```

3. **可视化分析**：
   ```
   视口 → Show → Visualize →
   - Shader Complexity
   - Light Complexity
   - Lightmap Density
   ```

4. **优化**：
   - 参考`OptimizationGuide.md`
   - 针对性优化瓶颈

### 性能目标

#### 开发阶段
- FPS: 30+
- Frame Time: <33ms
- Memory: <4GB

#### 发布阶段
- FPS: 60+
- Frame Time: <16.67ms
- Memory: <3GB
- Load Time: <10s

## 紧急恢复

### 项目损坏
如果项目严重损坏无法打开：

1. **从Git恢复**：
   ```bash
   git log --oneline
   git checkout <last-working-commit>
   ```

2. **从备份恢复**：
   ```bash
   cp -r D:/Backups/HorrorProject/latest/* ./
   ```

3. **重建项目**：
   ```bash
   rm -rf Saved/ Intermediate/ DerivedDataCache/
   右键.uproject → Generate Visual Studio project files
   ```

### 数据丢失
如果意外删除或覆盖文件：

1. **Git恢复**：
   ```bash
   git reflog
   git checkout <commit-hash> -- path/to/file
   ```

2. **自动保存恢复**：
   - File → Open Recent
   - 查找AutoSave版本

3. **备份恢复**：
   - 从每日备份中恢复

## 获取帮助

### 日志文件位置
```
Saved/Logs/HorrorProject.log - 主日志
Saved/Crashes/ - 崩溃报告
Saved/Logs/Swarm/ - 光照构建日志
```

### 有用的控制台命令
```
stat fps - 帧率
stat unit - 性能统计
stat memory - 内存使用
r.ScreenPercentage 50 - 降低渲染分辨率（测试）
t.MaxFPS 60 - 限制帧率
```

### 在线资源
- Unreal Engine文档：https://docs.unrealengine.com
- Unreal Engine论坛：https://forums.unrealengine.com
- Unreal Slackers Discord：https://unrealslackers.org
