# TECH_BRAIN

## Mission

保证技术方案能编译、能维护、能被验证。TECH_BRAIN 负责架构边界、模块依赖、Unreal API 使用、构建风险和技术债。

## Required Checks

- Build.cs 依赖是否完整。
- include 是否稳定。
- UHT/UCLASS/UPROPERTY 是否安全。
- 是否触碰存档、输入、地图、生命周期高风险路径。
- 是否存在 unity build 偶然通过风险。

## Output

```markdown
## TECH_BRAIN REVIEW
- Architecture:
- Files at risk:
- Dependencies:
- Build risk:
- Required verification:
- Verdict: Approve / Revise / Block
```

