# HorrorProject 音效目录

> SoundsOfHorror 完整音效索引与使用指南

---

## 📊 音效统计总览

| 分类 | 数量 | 用途 | 推荐场景 |
|------|------|------|----------|
| Atmosphere | 30 | 环境氛围 | 持续背景音 |
| BuildUps | 20 | 紧张递增 | 事件前奏 |
| Clues | 24 | 线索提示 | 互动反馈 |
| Impacts | 20 | 冲击音效 | 突发事件 |
| Jumpscares | 24 | 惊吓音效 | 跳杀时刻 |
| Puzzles | 24 | 谜题音效 | 解谜反馈 |
| Rumbles | 20 | 低频隆隆 | 威胁感 |
| Tension | 22 | 紧张氛围 | 危险区域 |
| XMelodies | 20 | 旋律片段 | 情绪渲染 |

**总计**: 204个专业恐怖音效

---

## 🌫️ Atmosphere (环境氛围) - 30个

### 音效列表
```
CUE_SOH_ATM_01 至 CUE_SOH_ATM_15 (Sound Cue)
WAV_SOH_ATM_01 至 WAV_SOH_ATM_15 (Wave文件)
```

### 用途建议
- **持续背景音**: 循环播放营造基础恐怖氛围
- **区域标识**: 不同区域使用不同氛围音
- **情绪铺垫**: 为后续事件建立情绪基础

### 推荐使用场景
- 废弃走廊的风声
- 地下室的水滴声
- 远处的机械噪音
- 不明来源的低语

### 技术参数
- **音量**: 30-50% (背景层)
- **循环**: 启用无缝循环
- **衰减**: 大范围衰减 (1000-3000单位)
- **优先级**: 低 (可被其他音效覆盖)

### 分层组合示例
```
Layer 1: ATM_01 (基础环境) - 40%音量
Layer 2: ATM_08 (细节层) - 25%音量
Layer 3: Tension_03 (紧张层) - 15%音量
```

---

## 📈 BuildUps (紧张递增) - 20个

### 音效列表
```
CUE_SOH_BU_01 至 CUE_SOH_BU_10 (Sound Cue)
WAV_SOH_BU_01 至 WAV_SOH_BU_10 (Wave文件)
```

### 用途建议
- **事件预告**: 在重大事件前3-5秒触发
- **紧张升级**: 逐步提升玩家警觉度
- **节奏控制**: 配合游戏节奏设计

### 推荐使用场景
- Boss出现前的音效铺垫
- 门即将打开的紧张感
- 追逐序列开始前
- 陷阱触发前的警示

### 技术参数
- **音量**: 60-80% (前景层)
- **循环**: 禁用 (单次播放)
- **衰减**: 中等衰减 (500-1500单位)
- **优先级**: 高 (不可被打断)

### 时间轴设计
```
0.0s - 1.0s: 低频启动
1.0s - 2.5s: 逐步增强
2.5s - 4.0s: 达到高潮
4.0s+: 触发主事件
```

---

## 🔍 Clues (线索提示) - 24个

### 音效列表
```
CUE_SOH_Clue_01 至 CUE_SOH_Clue_12 (Sound Cue)
WAV_SOH_Clue_01 至 WAV_SOH_Clue_12 (Wave文件)
```

### 用途建议
- **互动反馈**: 玩家发现线索时播放
- **引导提示**: 暗示重要物品位置
- **进度确认**: 确认玩家完成某个步骤

### 推荐使用场景
- 拾取日记/文档
- 发现隐藏通道
- 解锁新区域
- 触发剧情点

### 技术参数
- **音量**: 70-85% (清晰可辨)
- **循环**: 禁用
- **衰减**: 小范围 (300-800单位)
- **优先级**: 中高 (重要反馈)

### UI集成建议
```cpp
// 线索发现时
PlaySound2D(CUE_SOH_Clue_03);
ShowClueWidget();
AddToJournal(ClueData);
```

---

## 💥 Impacts (冲击音效) - 20个

### 音效列表
```
CUE_SOH_IP_01 至 CUE_SOH_IP_10 (Sound Cue)
WAV_SOH_IP_01 至 WAV_SOH_IP_10 (Wave文件)
```

### 用途建议
- **突发事件**: 物体坠落、门突然关闭
- **战斗反馈**: 攻击命中、伤害反馈
- **环境互动**: 破坏物体、触发机关

### 推荐使用场景
- 重物砸地
- 金属碰撞
- 玻璃破碎
- 爆炸余波

### 技术参数
- **音量**: 80-95% (强烈冲击感)
- **循环**: 禁用
- **衰减**: 中等衰减 (600-1200单位)
- **优先级**: 高 (瞬时反馈)

### 物理集成
```cpp
// 物体碰撞时
if (ImpactForce > Threshold) {
    int32 SoundIndex = FMath::RandRange(1, 10);
    PlaySoundAtLocation(CUE_SOH_IP_[Index], HitLocation);
}
```

---

## 😱 Jumpscares (惊吓音效) - 24个

### 音效列表
```
CUE_SOH_JS_01 至 CUE_SOH_JS_12 (Sound Cue)
WAV_SOH_JS_01 至 WAV_SOH_JS_12 (Wave文件)
```

### 用途建议
- **跳杀时刻**: 敌人突然出现
- **视觉配合**: 与跳杀动画同步
- **心理冲击**: 打破玩家预期

### 推荐使用场景
- 怪物突然扑出
- 镜子中出现鬼影
- 转身发现敌人
- 假跳杀误导

### 技术参数
- **音量**: 90-100% (最大冲击)
- **循环**: 禁用
- **衰减**: 无衰减或极小衰减
- **优先级**: 最高 (覆盖所有音效)

### 跳杀设计模式
```
准备阶段 (2-3秒):
  - 播放 BuildUp 音效
  - 降低环境音量到20%

触发时刻 (0.1秒):
  - 播放 Jumpscare 音效 (100%音量)
  - 触发相机震动
  - 显示跳杀画面

恢复阶段 (1-2秒):
  - 淡出 Jumpscare
  - 恢复环境音量
```

---

## 🧩 Puzzles (谜题音效) - 24个

### 音效列表
```
CUE_SOH_PZ_01 至 CUE_SOH_PZ_12 (Sound Cue)
WAV_SOH_PZ_01 至 WAV_SOH_PZ_12 (Wave文件)
```

### 用途建议
- **解谜反馈**: 正确/错误操作提示
- **机关音效**: 机械装置运作声
- **进度提示**: 谜题部分完成

### 推荐使用场景
- 密码锁输入
- 机关齿轮转动
- 拼图放置
- 谜题解开

### 技术参数
- **音量**: 65-80% (清晰但不刺耳)
- **循环**: 根据谜题类型决定
- **衰减**: 小范围 (400-1000单位)
- **优先级**: 中 (可与环境音共存)

### 谜题音效状态机
```
状态1: 待机 - PZ_01 (循环低音)
状态2: 交互中 - PZ_03 (机械音)
状态3: 部分完成 - PZ_05 (提示音)
状态4: 完全解开 - PZ_08 (成功音效)
状态5: 错误操作 - PZ_10 (警告音)
```

---

## 🌊 Rumbles (低频隆隆) - 20个

### 音效列表
```
CUE_SOH_RU_01 至 CUE_SOH_RU_10 (Sound Cue)
WAV_SOH_RU_01 至 WAV_SOH_RU_10 (Wave文件)
```

### 用途建议
- **威胁暗示**: 不可见的危险接近
- **环境压迫**: 营造压抑感
- **距离感**: 远处的巨大存在

### 推荐使用场景
- Boss在附近移动
- 建筑结构不稳
- 地下深处的震动
- 巨型机械运转

### 技术参数
- **音量**: 50-70% (低频为主)
- **循环**: 启用长循环
- **衰减**: 大范围 (1500-4000单位)
- **优先级**: 中低 (背景威胁层)

### 低频混音建议
```
频率范围: 20Hz - 150Hz (重点在60-80Hz)
EQ设置: 低频增强 +6dB
混响: 大空间混响 (3-5秒衰减)
压缩: 轻度压缩保持动态
```

---

## ⚡ Tension (紧张氛围) - 22个

### 音效列表
```
CUE_SOH_TS_01 至 CUE_SOH_TS_11 (Sound Cue)
WAV_SOH_TS_01 至 WAV_SOH_TS_11 (Wave文件)
```

### 用途建议
- **危险区域**: 标识高风险区域
- **追逐序列**: 逃跑时的紧张感
- **潜行段落**: 需要小心行动的区域

### 推荐使用场景
- 敌人巡逻区域
- 定时逃脱序列
- 资源匮乏区域
- 多敌人区域

### 技术参数
- **音量**: 55-75% (中等强度)
- **循环**: 启用循环
- **衰减**: 中等衰减 (800-2000单位)
- **优先级**: 中 (可与其他音效混合)

### 动态强度控制
```cpp
// 根据危险程度调整音量
float DangerLevel = CalculateDangerLevel();
float Volume = FMath::Lerp(0.4f, 0.8f, DangerLevel);
TensionSound->SetVolumeMultiplier(Volume);
```

---

## 🎵 XMelodies (旋律片段) - 20个

### 音效列表
```
CUE_SOH_XM_01 至 CUE_SOH_XM_10 (推测命名)
WAV_SOH_XM_01 至 WAV_SOH_XM_10 (推测命名)
```

### 用途建议
- **情绪渲染**: 特定情绪时刻的音乐片段
- **剧情强化**: 关键剧情点的音乐配合
- **记忆锚点**: 可识别的旋律主题

### 推荐使用场景
- 重要角色出场
- 剧情转折点
- 安全区域
- 结局序列

### 技术参数
- **音量**: 60-80% (音乐层)
- **循环**: 根据场景决定
- **衰减**: 小到中等 (500-1500单位)
- **优先级**: 中高 (情绪主导)

---

## 🎚️ 分层音频系统设计

### 三层混音架构

#### Layer 1: 环境基础层 (30-40%音量)
```
- Atmosphere 音效 (循环)
- Rumbles 音效 (循环)
- 环境混响效果
```

#### Layer 2: 情绪中间层 (40-60%音量)
```
- Tension 音效 (根据情况)
- XMelodies 音效 (剧情时刻)
- 动态音乐系统
```

#### Layer 3: 事件前景层 (70-100%音量)
```
- Jumpscares (跳杀)
- Impacts (冲击)
- Clues (反馈)
- BuildUps (递增)
- Puzzles (互动)
```

### 音效优先级系统
```cpp
enum class ESoundPriority {
    Background = 0,    // Atmosphere, Rumbles
    Low = 1,           // Tension
    Medium = 2,        // Puzzles, Clues
    High = 3,          // BuildUps, Impacts
    Critical = 4       // Jumpscares
};
```

---

## 🎯 场景音效组合推荐

### 场景1: 废弃医院走廊
```
基础层:
  - ATM_03 (医院环境音) - 35%
  - RU_02 (远处隆隆声) - 25%

情绪层:
  - TS_05 (紧张氛围) - 45%

事件:
  - 开门: IP_04
  - 发现文档: Clue_07
  - 怪物出现: BU_03 → JS_08
```

### 场景2: 地下室解谜
```
基础层:
  - ATM_08 (地下环境) - 40%
  - RU_06 (结构震动) - 30%

互动层:
  - PZ_01 (谜题待机) - 循环 50%
  - PZ_05 (部分完成) - 触发 70%
  - PZ_08 (完全解开) - 触发 80%

奖励:
  - Clue_03 (解锁提示)
```

### 场景3: Boss战前奏
```
准备阶段 (10秒):
  - ATM_12 (压抑环境) - 30%
  - RU_08 (Boss接近) - 渐强 40%→70%
  - BU_06 (紧张递增) - 5秒时触发

触发时刻:
  - JS_11 (Boss登场) - 100%
  - 相机震动 + 慢动作

战斗阶段:
  - TS_09 (战斗紧张) - 循环 60%
  - IP_07 (攻击反馈) - 触发 85%
```

### 场景4: 安全房间
```
基础层:
  - ATM_01 (平静环境) - 45%
  - XM_03 (舒缓旋律) - 50%

互动:
  - Clue_02 (存档提示)
  - PZ_12 (物品整理)

对比设计:
  - 离开时: BU_02 (重返危险)
```

---

## 🔧 技术实现指南

### Sound Cue vs Wave 文件

**CUE文件 (推荐使用)**
- 包含随机化、音量调制、音高变化
- 支持多个Wave文件随机播放
- 可设置衰减曲线和混响
- 更适合游戏动态需求

**WAV文件 (特殊情况)**
- 原始音频文件
- 用于需要精确控制的场景
- 可在蓝图中动态处理

### 蓝图实现示例

#### 动态氛围系统
```cpp
// 根据玩家位置切换氛围音效
void UpdateAtmosphereSound(FVector PlayerLocation) {
    if (IsInDangerZone(PlayerLocation)) {
        CrossfadeTo(CUE_SOH_ATM_10, 2.0f);
        AddTensionLayer(CUE_SOH_TS_07);
    } else {
        CrossfadeTo(CUE_SOH_ATM_02, 3.0f);
        RemoveTensionLayer();
    }
}
```

#### 跳杀触发器
```cpp
// 完整跳杀序列
void TriggerJumpscare(int32 JumpscareID) {
    // 1. 准备阶段
    PlaySound2D(CUE_SOH_BU_05);
    FadeOutAmbience(0.2f, 2.0f);

    // 2. 延迟触发
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this, JumpscareID]() {
            // 3. 跳杀时刻
            PlaySound2D(GetJumpscareSound(JumpscareID), 1.0f);
            TriggerCameraShake();
            ShowJumpscareVisual();

            // 4. 恢复
            FadeInAmbience(0.6f, 1.5f);
        },
        2.5f,
        false
    );
}
```

#### 谜题音效状态机
```cpp
UENUM()
enum class EPuzzleState : uint8 {
    Idle,
    Interacting,
    PartialComplete,
    FullComplete,
    Error
};

void UpdatePuzzleSound(EPuzzleState NewState) {
    switch (NewState) {
        case EPuzzleState::Idle:
            LoopSound(CUE_SOH_PZ_01, 0.5f);
            break;
        case EPuzzleState::Interacting:
            PlaySound(CUE_SOH_PZ_03, 0.7f);
            break;
        case EPuzzleState::PartialComplete:
            PlaySound(CUE_SOH_PZ_05, 0.75f);
            break;
        case EPuzzleState::FullComplete:
            StopLoopedSounds();
            PlaySound(CUE_SOH_PZ_08, 0.85f);
            PlaySound(CUE_SOH_Clue_06, 0.8f);
            break;
        case EPuzzleState::Error:
            PlaySound(CUE_SOH_PZ_10, 0.7f);
            break;
    }
}
```

---

## 🎨 音效设计原则

### 1. 对比原则
- 安静后的巨响更有冲击力
- 平静区域与危险区域形成对比
- 使用音效缺失制造不安感

### 2. 分层原则
- 不要同时播放过多高优先级音效
- 保持3-4层音频同时存在
- 低频、中频、高频合理分布

### 3. 节奏原则
- 控制跳杀频率避免疲劳
- 紧张-放松-紧张的节奏循环
- 用静默制造期待感

### 4. 空间原则
- 使用3D音效增强沉浸感
- 远近音效的音量和混响差异
- 方向性音效引导玩家

---

## 📊 性能优化建议

### 音效池管理
```cpp
// 预加载常用音效
TArray<USoundBase*> PreloadedSounds = {
    CUE_SOH_ATM_01,
    CUE_SOH_TS_05,
    CUE_SOH_JS_08,
    // ...
};

// 限制同时播放数量
const int32 MaxConcurrentSounds = 16;
```

### 音效优先级剔除
- 低优先级音效在资源紧张时自动停止
- 距离过远的音效不播放
- 相同类型音效避免重叠

### 内存管理
- 使用Sound Cue引用而非直接加载Wave
- 异步加载非关键音效
- 及时释放不再使用的音效资源

---

## 🎯 快速参考表

| 需求 | 推荐音效 | 音量 | 循环 |
|------|----------|------|------|
| 基础环境音 | ATM_01-15 | 35% | 是 |
| 事件预告 | BU_01-10 | 70% | 否 |
| 发现线索 | Clue_01-12 | 75% | 否 |
| 物体碰撞 | IP_01-10 | 85% | 否 |
| 跳杀时刻 | JS_01-12 | 100% | 否 |
| 解谜反馈 | PZ_01-12 | 70% | 视情况 |
| 威胁感 | RU_01-10 | 60% | 是 |
| 危险区域 | TS_01-11 | 65% | 是 |
| 剧情音乐 | XM_01-10 | 70% | 视情况 |

---

**文档版本**: 1.0
**最后更新**: 2026-04-28
**音效总数**: 204
**维护者**: HorrorProject Team
