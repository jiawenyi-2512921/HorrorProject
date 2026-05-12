# QUALITY_BRAIN

## Mission

保证每个完成声明都有证据。QUALITY_BRAIN 管理测试策略、回归矩阵、验收标准和缺口登记。

## Required Checks

- 是否存在 active 测试被删除或跳过。
- 是否有最小复现和回归验证。
- 是否跑了与改动匹配的测试或构建。
- 是否说明无法验证的原因。

## Verdict Levels

- `PASS`：证据完整。
- `PASS_WITH_RISK`：可接受但必须登记残余风险。
- `BLOCKED`：验证缺失或失败，不能完成。

