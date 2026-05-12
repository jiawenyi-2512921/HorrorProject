# War Room

War Room 是当前项目最高优先级战情板。P0/P1 风险必须在这里可见。

## 当前战情摘要

- 项目：Unreal Engine 5.6 第一人称恐怖项目。
- 仓库根目录：`D:\gptzuo`。
- 项目目录：`HorrorProject/HorrorProject/HorrorProject.uproject`。
- 当前状态：项目可见结构完整，但存在 dirty git、测试覆盖流失、构建验证阻塞和运行时流程风险。

## P0 风险

| ID | 风险 | 影响 | Owner | 状态 |
| --- | --- | --- | --- | --- |
| P0-GIT-001 | staged/unstaged 分裂，关键文件可能被错误提交 | 错误地图入口、丢失修复、无法可信交付 | Production + Governance | Open |
| P0-TEST-001 | active 测试被删除，丢失 Day1/存档/UI 回归保护 | 关键功能回归无证据 | QA Intelligence | Open |
| P0-SAVE-001 | New Game 只清缓存不删磁盘 autosave | 旧存档可能复活 | Engineering + QA | Open |
| P0-SAVE-002 | Continue 在主菜单世界直接 LoadCheckpoint | 继续游戏可能失败或落入错误路径 | Engineering + Gameplay | Open |

## P1 风险

| ID | 风险 | 影响 | Owner | 状态 |
| --- | --- | --- | --- | --- |
| P1-BUILD-001 | UE Live Coding 阻塞真实构建 | 无法证明编译通过 | Build & Release | Open |
| P1-BUILD-002 | `AssetRegistry` 依赖未写入 Build.cs | clean/non-unity build 可能失败 | Engineering | Open |
| P1-PKG-001 | Localization JSON 未明确 staged | 打包后本地化可能回退 | Build & Release | Open |

## P2 风险

| ID | 风险 | 影响 | Owner | 状态 |
| --- | --- | --- | --- | --- |
| P2-ASSET-001 | 缺失 Achievement notification UMG | 成就通知静默不显示 | Content Studio | Open |
| P2-ASSET-002 | 缺失 EndingCredits UMG，依赖 native fallback | 结局 UI 降级 | Content Studio | Open |
| P2-ASSET-003 | 缺失 UI font 资产路径 | 本地化字体加载失败风险 | Content Studio | Open |
| P2-INTERACT-001 | Switch 动画可能锁交互 | 关卡机关可能卡死 | Gameplay | Open |
| P2-INTERACT-002 | Pickup 失败前已标记 interacted | 拾取失败可能被存档固化 | Gameplay + Save | Open |

## 当前禁令

- 禁止在未解决 `P0-GIT-001` 前创建发布提交。
- 禁止在构建被 Live Coding 阻塞时宣称构建通过。
- 禁止删除或继续忽略 active 测试流失风险。

