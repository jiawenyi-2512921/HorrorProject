# 优化指南

## 光照优化

### 光照构建设置

#### 开发阶段（快速迭代）
在`World Settings → Lightmass Settings`中：
- Static Lighting Level Scale: `2.0`
- Num Indirect Lighting Bounces: `1`
- Indirect Lighting Quality: `0.5`
- Indirect Lighting Smoothness: `0.5`

构建时间：约5-10分钟

#### 最终发布
- Static Lighting Level Scale: `0.5`
- Num Indirect Lighting Bounces: `3`
- Indirect Lighting Quality: `2.0`
- Indirect Lighting Smoothness: `1.0`

构建时间：约30-60分钟

### 光源优化

#### 静态光源（Static Lights）
**优点**：
- 性能最优
- 支持光照烘焙
- 适合环境光和固定光源

**使用场景**：
- 室外阳光
- 固定的室内灯光
- 不会移动或改变的光源

**设置**：
```
Mobility: Static
Cast Shadows: Yes
Indirect Lighting Intensity: 1.0
```

#### 静止光源（Stationary Lights）
**优点**：
- 可以改变颜色和强度
- 支持动态阴影
- 性能适中

**使用场景**：
- 需要闪烁的灯光
- 可以开关的灯
- 恐怖氛围灯光

**设置**：
```
Mobility: Stationary
Cast Shadows: Yes
Dynamic Shadow Distance: 2000
Max Draw Distance: 5000
```

**限制**：
- 每个区域最多4个Stationary光源重叠
- 超过限制会自动降级为动态光源

#### 动态光源（Movable Lights）
**优点**：
- 完全动态
- 可以移动和旋转
- 支持所有光照效果

**缺点**：
- 性能开销最大
- 不支持间接光照

**使用场景**：
- 手电筒
- 移动的光源
- 特殊效果

**优化设置**：
```
Mobility: Movable
Cast Shadows: Yes (仅在必要时)
Shadow Bias: 0.5
Max Draw Distance: 3000
Attenuation Radius: 尽可能小
```

### 光照优化技巧

#### 1. Lightmass重要体积
```
添加 Lightmass Importance Volume 包围关卡
- 只在重要区域计算高质量光照
- 减少构建时间
- 提高光照质量
```

#### 2. 光照贴图分辨率
```
静态网格体：
- 大型墙壁/地板: 64-128
- 中型道具: 32-64
- 小型道具: 16-32

BSP几何体：
- Lightmap Resolution: 4-8（值越小越精细）
```

#### 3. 阴影优化
```
减少阴影投射物体：
- 小型装饰物：Cast Shadow = False
- 远处物体：使用 Max Draw Distance
- 使用 Shadow Cascades 优化远景阴影
```

#### 4. 反射捕获
```
放置 Sphere Reflection Capture：
- 每个房间至少一个
- 半径覆盖整个房间
- 避免过度重叠

Box Reflection Capture：
- 用于走廊和规则空间
- 更精确的控制
```

### 光照性能检查

#### 查看光照复杂度
```
视口 → Show → Visualize → Shader Complexity
- 绿色：良好
- 黄色：可接受
- 红色：需要优化
```

#### 查看光照重叠
```
视口 → Show → Visualize → Stationary Light Overlap
- 确保没有超过4个Stationary光源重叠
```

## 网格优化

### LOD（细节层次）设置

#### 自动生成LOD
1. 选择静态网格体
2. Details → LOD Settings
3. Number of LODs: `4`
4. Auto Compute LOD Distances: `True`
5. 点击 Apply Changes

#### LOD距离建议
```
LOD0 (原始): 0 - 1000 单位
LOD1: 1000 - 2500 单位
LOD2: 2500 - 5000 单位
LOD3: 5000+ 单位
```

#### 手动LOD设置
```
大型道具（家具、门）：
- LOD0: 100% 三角形
- LOD1: 50% 三角形
- LOD2: 25% 三角形

小型道具（杯子、书）：
- LOD0: 100% 三角形
- LOD1: 40% 三角形
- 远距离使用 Impostor
```

### 碰撞优化

#### 简化碰撞
```
静态网格编辑器 → Collision →
- 简单碰撞：使用盒体、球体、胶囊体
- 复杂碰撞：仅在必要时使用 Complex Collision

推荐：
- 墙壁/地板：Box Collision
- 圆柱物体：Capsule Collision
- 不规则物体：Auto Convex Collision (Max Hulls: 4-8)
```

#### 碰撞设置
```
不需要碰撞的物体：
- Collision Presets: NoCollision
- 装饰性物体、远景物体

需要碰撞的物体：
- Collision Presets: BlockAll
- 墙壁、地板、可交互物体
```

### 网格实例化

#### 使用实例化静态网格（ISM）
```cpp
// 对于重复出现的物体（椅子、灯、窗户）
使用 Instanced Static Mesh Component
- 大幅减少Draw Call
- 提高渲染性能

示例：
- 100个独立椅子 = 100 Draw Calls
- 100个实例化椅子 = 1 Draw Call
```

#### 分层实例化静态网格（HISM）
```
用于大量重复物体：
- 树木、草地
- 碎石、瓦砾
- 自动剔除和LOD
```

### 材质优化

#### 材质复杂度
```
视口 → Show → Visualize → Shader Complexity
目标：大部分区域为绿色

优化方法：
- 减少材质节点数量
- 避免复杂的数学运算
- 使用材质函数复用逻辑
```

#### 纹理优化
```
纹理大小建议：
- 主要表面（墙壁、地板）：2048x2048
- 道具：1024x1024
- 小型物体：512x512
- UI元素：根据实际显示大小

压缩设置：
- 漫反射/法线：BC1/BC5
- 带Alpha的纹理：BC3
- HDR纹理：BC6H
```

#### 材质实例
```
创建材质实例而非复制材质：
1. 右键材质 → Create Material Instance
2. 调整参数而非修改节点
3. 共享基础材质减少Shader编译
```

## 音频优化

### 音频文件格式

#### 推荐格式
```
环境音效：
- 格式：OGG Vorbis
- 采样率：44.1kHz
- 比特率：128-192 kbps

音效：
- 格式：WAV（短音效）
- 格式：OGG（长音效）
- 采样率：44.1kHz
- 比特率：16-bit

音乐：
- 格式：OGG Vorbis
- 采样率：44.1kHz
- 比特率：192-256 kbps
```

### 音频导入设置

#### Sound Cue设置
```
短音效（<5秒）：
- Compression Quality: 40
- Loading Behavior: Load on Demand

环境音（循环）：
- Compression Quality: 60
- Loading Behavior: Stream
- Looping: True

音乐：
- Compression Quality: 80
- Loading Behavior: Stream
- Looping: True (如果需要)
```

### 音频空间化

#### Attenuation设置
```
近距离音效（脚步声、门）：
- Inner Radius: 100
- Falloff Distance: 500
- Attenuation Shape: Sphere

中距离音效（环境音）：
- Inner Radius: 500
- Falloff Distance: 2000
- Attenuation Shape: Sphere

远距离音效（雷声、风声）：
- Inner Radius: 2000
- Falloff Distance: 10000
- Attenuation Shape: Sphere
```

### 音频优化技巧

#### 1. 音频池化
```
限制同时播放的音效数量：
Sound Class → Max Concurrent Play Count: 4-8

优先级设置：
- 重要音效（对话、关键音效）：Priority: 1.0
- 环境音：Priority: 0.5
- 装饰音效：Priority: 0.3
```

#### 2. 音频剔除
```
设置 Max Distance：
- 超过距离自动停止播放
- 减少CPU开销

Virtualization：
- 启用虚拟化
- 不可听见时停止处理
```

#### 3. 音频混响
```
使用 Reverb Volume：
- 每个房间一个
- 设置合适的混响预设
- 避免过度重叠

推荐预设：
- 小房间：Small Room
- 大厅：Large Hall
- 走廊：Corridor
- 地下室：Cave
```

## 蓝图优化

### 事件优化

#### Tick事件优化
```
避免在Tick中执行：
- 复杂计算
- 字符串操作
- 数组遍历

替代方案：
- 使用Timer（每0.1秒执行一次）
- 使用事件驱动
- 使用Custom Event
```

#### Timer使用
```
// 替代Tick
Set Timer by Event
- Time: 0.1-0.5秒
- Looping: True

示例：
- 检查玩家距离：每0.2秒
- 更新UI：每0.5秒
- AI逻辑：每0.1秒
```

### 引用优化

#### 软引用 vs 硬引用
```
硬引用（直接引用）：
- 立即加载资产
- 增加内存占用
- 用于频繁使用的资产

软引用（Soft Object Reference）：
- 按需加载
- 减少内存占用
- 用于可选或远距离资产

示例：
- 当前关卡资产：硬引用
- 其他关卡资产：软引用
- 过场动画：软引用
```

#### 异步加载
```
使用 Async Load Asset：
- 不阻塞游戏线程
- 显示加载进度
- 用于大型资产
```

### 蓝图结构优化

#### 函数化
```
将重复代码提取为函数：
- 提高可读性
- 便于维护
- 可以设置为Pure函数（无副作用）
```

#### 宏 vs 函数
```
宏（Macro）：
- 内联展开
- 性能略好
- 用于简单逻辑

函数（Function）：
- 可以有返回值
- 更好的调试
- 用于复杂逻辑
```

#### 事件调度器
```
使用Event Dispatcher解耦：
- 避免直接引用
- 提高模块化
- 便于扩展

示例：
- 门打开事件
- 玩家受伤事件
- 关卡完成事件
```

### 性能分析

#### 蓝图性能分析
```
1. 打开蓝图
2. Window → Developer Tools → Blueprint Profiler
3. 开始录制
4. 执行游戏逻辑
5. 停止录制
6. 查看热点函数

优化目标：
- 单个函数 < 0.1ms
- Tick事件 < 0.05ms
```

#### CPU性能分析
```
控制台命令：
stat game - 游戏线程统计
stat slow - 显示慢函数
stat startfile / stat stopfile - 记录性能数据

目标：
- Game Thread: < 16ms (60 FPS)
- Draw Thread: < 16ms
- GPU: < 16ms
```

## 通用优化检查清单

### 每日检查
- [ ] FPS保持在30+
- [ ] 内存使用 < 4GB
- [ ] 无明显卡顿
- [ ] 光照构建无错误

### 每周检查
- [ ] 运行完整性能分析
- [ ] 检查纹理内存占用
- [ ] 优化Draw Call数量
- [ ] 清理未使用资产

### 发布前检查
- [ ] 完整光照构建
- [ ] 所有LOD正确设置
- [ ] 音频正确压缩
- [ ] 蓝图优化完成
- [ ] 性能达到目标（60 FPS）
