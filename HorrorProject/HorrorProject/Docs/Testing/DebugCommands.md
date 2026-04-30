# HorrorProject 调试命令文档

## 控制台访问
- **打开控制台**: `~` 或 `` ` `` 键（根据键盘布局）
- **完整控制台**: `~` 两次
- **输出日志**: `Tab` 键查看命令历史

---

## 基础调试命令

### 显示信息
```
stat FPS                    // 显示帧率
stat Unit                   // 显示帧时间详情（Game/Draw/GPU）
stat UnitGraph              // 显示帧时间图表
stat Game                   // 显示游戏线程统计
stat SceneRendering         // 显示渲染统计
stat RHI                    // 显示渲染硬件接口统计
stat Memory                 // 显示内存使用
stat Streaming              // 显示资产流式加载状态
stat Audio                  // 显示音频统计
```

### 性能分析
```
ProfileGPU                  // GPU性能分析
DumpConsoleCommands         // 列出所有可用命令
r.ScreenPercentage [0-200]  // 调整渲染分辨率（性能测试）
t.MaxFPS [数值]             // 限制最大帧率
Pause                       // 暂停游戏逻辑
Slomo [0.1-10.0]           // 调整游戏速度（1.0为正常）
```

### 渲染调试
```
viewmode lit                // 正常光照模式
viewmode unlit              // 无光照模式
viewmode wireframe          // 线框模式
viewmode shadercomplexity   // 着色器复杂度
viewmode lightcomplexity    // 光照复杂度
viewmode detaillighting     // 详细光照
viewmode lightmapDensity    // 光照贴图密度
viewmode reflections        // 仅反射
viewmode lod                // LOD着色
```

### 光照调试
```
r.SetNearClipPlane [0.1-10] // 调整近裁剪面
r.LightFunctionQuality [0-1] // 光照函数质量
r.ShadowQuality [0-5]       // 阴影质量
r.Shadow.MaxResolution [512-4096] // 阴影最大分辨率
r.DynamicGlobalIllumination [0/1] // 动态全局光照
show Lighting               // 切换光照显示
show Shadows                // 切换阴影显示
show Fog                    // 切换雾效显示
```

### 碰撞调试
```
show Collision              // 显示碰撞体
show CollisionPawn          // 显示角色碰撞
show CollisionVisibility    // 显示可见性碰撞
pxvis collision             // PhysX碰撞可视化
```

### AI调试
```
showdebug ai                // 显示AI调试信息
showdebug navigation        // 显示导航网格
show Navigation             // 切换导航网格显示
p.VisualizeMovement 1       // 可视化移动
```

---

## 玩家控制命令

### 移动与相机
```
Ghost                       // 穿墙模式
Walk                        // 退出穿墙模式
Fly                         // 飞行模式
ToggleDebugCamera           // 切换调试相机
Teleport                    // 传送到准星位置
God                         // 无敌模式
```

### 角色状态
```
ChangeSize [0.1-10.0]       // 改变角色大小
SetGravity [数值]           // 设置重力（默认-980）
SetJumpZ [数值]             // 设置跳跃高度
SetSpeed [数值]             // 设置移动速度
```

---

## 关卡与世界

### 关卡管理
```
Open [MapName]              // 打开指定关卡
RestartLevel                // 重启当前关卡
Exit                        // 退出游戏
ServerTravel [MapName]      // 服务器切换关卡
```

### 时间控制
```
Pause                       // 暂停/继续
Slomo 0.5                   // 慢动作（50%速度）
Slomo 2.0                   // 快进（200%速度）
Slomo 1.0                   // 恢复正常速度
```

### 环境控制
```
ke * SetTimeOfDay [0-24]    // 设置时间（如果有时间系统）
r.Fog 0                     // 禁用雾效
r.Fog 1                     // 启用雾效
r.Tonemapper.Sharpen [0-2]  // 锐化强度
```

---

## 音频调试

### 音量控制
```
au.Debug.SoundCueVolume [0-1]     // 音效音量
au.Debug.MusicVolume [0-1]        // 音乐音量
au.Debug.MasterVolume [0-1]       // 主音量
```

### 音频可视化
```
au.Debug.Sounds                   // 显示正在播放的声音
au.3dVisualize.Enabled 1          // 3D音频可视化
stat SoundWaves                   // 声波统计
stat SoundMixes                   // 混音统计
```

---

## 内存与性能

### 内存分析
```
obj list                          // 列出所有对象
obj classes                       // 列出所有类
memreport                         // 生成内存报告
memreport -full                   // 完整内存报告
stat Memory                       // 内存统计
stat Streaming                    // 流式加载统计
```

### 垃圾回收
```
obj gc                            // 强制垃圾回收
obj purge                         // 清理未使用对象
```

### 性能捕获
```
StartFPSChart                     // 开始FPS记录
StopFPSChart                      // 停止FPS记录
stat StartFile                    // 开始统计记录
stat StopFile                     // 停止统计记录
```

---

## 蓝图调试

### 蓝图执行
```
showdebug blueprint               // 显示蓝图调试信息
EnableLiveDebugging               // 启用实时调试
DisableLiveDebugging              // 禁用实时调试
```

---

## HorrorProject 自定义调试命令

### 恐怖系统调试
```
// 以下命令需要在项目中实现

Horror.SetFearLevel [0-100]       // 设置恐惧值
Horror.ToggleEnemyAI              // 切换敌人AI
Horror.SpawnEnemy [EnemyType]     // 生成指定敌人
Horror.ShowAIDebug                // 显示AI调试信息
Horror.SetFlashlightBattery [0-100] // 设置手电筒电量
Horror.ToggleInvincible           // 切换无敌模式
Horror.UnlockAllDoors             // 解锁所有门
Horror.RevealMap                  // 显示完整地图
Horror.SkipToCheckpoint [ID]      // 跳转到检查点
```

### 氛围控制
```
Horror.SetAmbientIntensity [0-1]  // 环境强度
Horror.ToggleJumpscares           // 切换Jumpscare
Horror.SetMusicIntensity [0-1]    // 音乐强度
Horror.ToggleFog                  // 切换雾效
Horror.SetLightingPreset [Preset] // 光照预设
```

### 测试工具
```
Horror.TeleportToEnemy            // 传送到敌人位置
Horror.TeleportToObjective        // 传送到目标点
Horror.ShowCollectibles           // 显示所有收集品
Horror.CompleteObjective [ID]     // 完成指定目标
Horror.ResetProgress              // 重置进度
```

---

## 性能优化命令

### 降低质量（提升性能）
```
sg.ResolutionQuality 50           // 分辨率质量
sg.ViewDistanceQuality 0          // 视距质量
sg.AntiAliasingQuality 0          // 抗锯齿质量
sg.ShadowQuality 0                // 阴影质量
sg.PostProcessQuality 0           // 后处理质量
sg.TextureQuality 0               // 纹理质量
sg.EffectsQuality 0               // 特效质量
sg.FoliageQuality 0               // 植被质量
```

### 禁用效果（极限性能）
```
r.Bloom 0                         // 禁用泛光
r.MotionBlur 0                    // 禁用动态模糊
r.DepthOfField 0                  // 禁用景深
r.LensFlare 0                     // 禁用镜头光晕
r.SSR 0                           // 禁用屏幕空间反射
r.SSAO 0                          // 禁用环境光遮蔽
r.ContactShadows 0                // 禁用接触阴影
```

---

## 日志与输出

### 日志级别
```
Log LogTemp Log [消息]            // 普通日志
Log LogTemp Warning [消息]        // 警告日志
Log LogTemp Error [消息]          // 错误日志
```

### 日志过滤
```
Log list                          // 列出所有日志类别
Log [Category] off                // 关闭指定类别
Log [Category] verbose            // 详细日志
```

---

## 截图与录制

### 截图
```
Shot                              // 普通截图
HighResShot 2                     // 2倍分辨率截图
HighResShot 4                     // 4倍分辨率截图
HighResShot 2x2                   // 2x2平铺截图
```

### 录制
```
StartMovieCapture                 // 开始录制
StopMovieCapture                  // 停止录制
```

---

## 快速测试配置

### 快速性能测试
```
// 复制以下命令到控制台快速测试性能
stat FPS | stat Unit | r.ScreenPercentage 100
```

### 快速质量测试
```
// 测试最高质量
sg.ResolutionQuality 100 | sg.ViewDistanceQuality 3 | sg.ShadowQuality 3 | sg.PostProcessQuality 3
```

### 快速调试模式
```
// 启用所有调试显示
stat FPS | show Collision | show Navigation | showdebug ai
```

---

## 注意事项

1. **命令大小写**: 大多数命令不区分大小写
2. **参数格式**: 参数之间用空格分隔
3. **布尔值**: 0=关闭, 1=开启
4. **持久化**: 控制台命令在重启后失效，需要在配置文件中设置永久选项
5. **性能影响**: 某些调试显示会影响性能
6. **保存配置**: 使用 `WriteConfig` 保存当前设置

---

## 配置文件位置

调试设置可保存到以下配置文件：
- `Saved/Config/Windows/Engine.ini`
- `Saved/Config/Windows/Game.ini`
- `Saved/Config/Windows/Input.ini`

示例配置：
```ini
[/Script/Engine.Engine]
bSmoothFrameRate=True
SmoothedFrameRateRange=(LowerBound=(Type=Inclusive,Value=30),UpperBound=(Type=Exclusive,Value=60))

[SystemSettings]
r.VSync=0
r.ScreenPercentage=100
```
