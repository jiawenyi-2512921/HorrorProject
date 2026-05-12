# Risk Register

| ID | Severity | Risk | Evidence | Owner | Mitigation | Status |
| --- | --- | --- | --- | --- | --- | --- |
| P0-GIT-001 | P0 | staged/unstaged 分裂可能导致错误提交 | `git status` 显示 `MM` 和大量 staged/deleted files | Production | 建立提交边界，逐文件确认 index | Open |
| P0-TEST-001 | P0 | active Day1/Save/UI 测试被删除 | 删除 4 个 active suites，丢失 27 个测试 | QA Intelligence | 恢复或重建测试 | Open |
| P0-SAVE-001 | P0 | New Game 不删除磁盘 autosave | New Game 只调用 `ClearCachedSaveOnly` | Engineering | 删除 slot 或重设 save policy | Open |
| P0-SAVE-002 | P0 | Continue 在主菜单世界恢复 checkpoint | 主菜单无 pawn，LoadCheckpoint 条件不满足 | Engineering | 设计 map-aware continue | Open |
| P1-BUILD-001 | P1 | Live Coding 阻塞 fresh build | UBT 输出 unable to build while Live Coding active | Build & Release | 关闭 editor/live coding 后重跑 | Open |
| P1-BUILD-002 | P1 | AssetRegistry 依赖缺失 | Source include + module load，但 Build.cs 未声明 | Engineering | 添加 private dependency | Open |
| P1-PKG-001 | P1 | Localization JSON 可能未打包 | FFileHelper 读 raw JSON，NonUFS 未列目录 | Build & Release | 添加 staging 或改为 UE localization asset | Open |
| P2-ASSET-001 | P2 | Achievement notification UMG 缺失 | hardcoded path missing | Content Studio | 创建资产或改 native fallback | Open |
| P2-ASSET-002 | P2 | Ending credits UMG 缺失 | hardcoded path missing，native fallback 存在 | Content Studio | 创建资产或移除 stale load | Open |
| P2-ASSET-003 | P2 | UI font path 缺失 | `/Game/Fonts/*` 不存在 | Content Studio | 添加字体资产或改配置 | Open |

