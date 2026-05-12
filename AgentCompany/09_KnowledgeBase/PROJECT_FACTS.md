# Project Facts

## 基线事实

- Engine：Unreal Engine 5.6。
- Project：`HorrorProject/HorrorProject/HorrorProject.uproject`。
- Runtime module：`HorrorProject`。
- Editor module：`HorrorProjectEditor`。
- Default map：`/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1`。
- Enabled key plugins：EnhancedInput、StateTree、GameplayStateTree。
- 主要语言：C++、Unreal config、UMG/Blueprint asset。

## 当前已知健康事实

- Git 当前 dirty，且有 staged/unstaged 分裂。
- `main` ahead origin/main 4 commits。
- UE Editor 当前可能导致 Live Coding 阻塞命令行构建。
- 大量测试和 UI 类处于删除状态。
- active test coverage 有实质流失。
- 部分硬编码 UI/font 资产路径缺失。

## 操作事实

- 不得修改或回滚非当前任务所有的 Unreal 源码改动。
- 新建 AgentCompany 文件不应触碰项目源码。
- 构建验证必须明确说明是否被 Live Coding 阻塞。

